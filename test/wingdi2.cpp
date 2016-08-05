
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <windows.h>

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <algorithm>

#include "dosboxxr/lib/util/sseutil.h"
#include "dosboxxr/lib/hostcpudetect/caps.h"
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_patterns.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"
#include "dosboxxr/lib/graphics/stretchblt_general.h"

static rgb_bitmap_info          src_bitmap;

HWND				hwndMain = NULL;
const char*			hwndMainClass = "WINGDICLASS";
const char*			hwndMainTitle = "WinGDI test 1";
HINSTANCE			myInstance;

HDC				dibDC = NULL;
BITMAPINFO*			dibBmpInfo;
unsigned char			dibBmpInfoRaw[sizeof(BITMAPV4HEADER) + (256*sizeof(RGBQUAD))];
HBITMAP				dibOldBitmap = NULL,dibBitmap = NULL;	// you can't free a bitmap if it's selected into a DC
unsigned int			dibBitsPerPixel = 0;
unsigned char*			dibBits = NULL;

bool				gdi_no_dpiaware = false;
unsigned int			gdi_force_depth = 0;

rgb_bitmap_info                 gdi_bitmap;
bool				announce_fmt = true;
bool				use_bitfields = false;
bool				rgba_order = false;
bool				no24bpp_pad = false;

static size_t                   method = 0;
static bool                     resize_src_mode = false;

static void free_src_bitmap(void);

static int init_src_bitmap(unsigned int width=640,unsigned int height=480) {
    if (src_bitmap.base != NULL)
        return 0;
    if (!gdi_bitmap.is_valid())
        return 0;

    src_bitmap.clear();
    src_bitmap.base_align = 32;
    src_bitmap.stride_align = 32;
    src_bitmap.rgbinfo = gdi_bitmap.rgbinfo;
    src_bitmap.bytes_per_pixel = gdi_bitmap.bytes_per_pixel;
    src_bitmap.width = width;
    src_bitmap.height = height;
    src_bitmap.update_stride_from_width();
    if (src_bitmap.bytes_per_pixel == 3) src_bitmap.stride += src_bitmap.stride_align; // 24bpp safety
    src_bitmap.update_length_from_stride_and_height();

#if HAVE_POSIX_MEMALIGN
    if (posix_memalign((void**)(&src_bitmap.base),src_bitmap.base_align,src_bitmap.length)) {
        fprintf(stderr,"Unable to alloc src bitmap\n");
        assert(src_bitmap.base == NULL);
        free_src_bitmap();
        return 0;
    }
    assert(src_bitmap.base != NULL);
    src_bitmap.canvas = src_bitmap.base;
#else
    src_bitmap.base = (unsigned char*)malloc(src_bitmap.length + (src_bitmap.base_align*2));
    if (src_bitmap.base == NULL) {
        fprintf(stderr,"Unable to alloc src bitmap\n");
        assert(src_bitmap.base == NULL);
        free_src_bitmap();
        return 0;
    }
    src_bitmap.canvas = (unsigned char*)(((uintptr_t)src_bitmap.base + src_bitmap.base_align - 1U) & ~((uintptr_t)(src_bitmap.base_align - 1U)));
#endif

    assert(src_bitmap.is_valid());
    return 1;
}

static void free_src_bitmap(void) {
    if (src_bitmap.base != NULL) {
        free(src_bitmap.base);
        src_bitmap.base = NULL;
    }

    src_bitmap.clear();
}

void win32_dpi_aware(void) { // Windows 7? DPI scaling, disable it
	HRESULT WINAPI (*__SetProcessDpiAwareness)(unsigned int aware);
	HMODULE dll;

	dll = LoadLibrary("shcore.dll");
	if (dll == NULL) return;

	__SetProcessDpiAwareness = (HRESULT WINAPI (*)(unsigned int))
		GetProcAddress(dll,"SetProcessDpiAwareness");
	if (__SetProcessDpiAwareness == NULL) {
		FreeLibrary(dll);
		return;
	}

	__SetProcessDpiAwareness(1/*PROCESS_SYSTEM_DPI_AWARE*/);

	FreeLibrary(dll);
	return;
}

void free_bitmap(void) {
	if (dibDC != NULL) {
		if (dibOldBitmap != NULL) {
			SelectObject(dibDC,dibOldBitmap);
			dibOldBitmap = NULL;
		}

		DeleteDC(dibDC);
		dibDC = NULL;
	}

	if (dibOldBitmap != NULL) {
		DeleteObject(dibOldBitmap);
		dibOldBitmap = NULL;
	}

	if (dibBitmap != NULL) {
		DeleteObject(dibBitmap);
		dibBitmap = NULL;
	}

	gdi_bitmap.clear();
}

int init_bitmap(unsigned int w,unsigned int h,unsigned int align=32) {
	unsigned int pwidth;

	free_bitmap();
	if (w == 0 || h == 0) return 0;

	dibBmpInfo = (BITMAPINFO*)dibBmpInfoRaw;
	memset(dibBmpInfoRaw,0,sizeof(dibBmpInfoRaw));

	dibDC = CreateCompatibleDC(0);
	if (dibDC == NULL) {
		fprintf(stderr,"Unable to create compatible DC\n");
		free_bitmap();
		return 0;
	}

	if (gdi_force_depth == 0) {
		HDC screenDC = GetDC(0);
		if (screenDC == NULL) {
			fprintf(stderr,"Failed to create screen DC\n");
			free_bitmap();
			return 0;
		}

		HBITMAP sbmp = CreateCompatibleBitmap(screenDC,32,32);
		if (sbmp == NULL) {
			fprintf(stderr,"Failed to create compat bmp\n");
			ReleaseDC(hwndMain,screenDC);
			free_bitmap();
			return 0;
		}

		dibBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

		// first time, fill in the basic struct
		if (GetDIBits(dibDC,sbmp,0,32,NULL,dibBmpInfo,DIB_RGB_COLORS) == 0) {
			fprintf(stderr,"Failed to determine screen format\n");
			ReleaseDC(hwndMain,screenDC);
			DeleteObject(sbmp);
			free_bitmap();
			return 0;
		}

		// second time, fill in the "palette". In this case, we want the BI_BITFIELDS red/green/blue/alpha bitmasks */
		if (GetDIBits(dibDC,sbmp,0,32,NULL,dibBmpInfo,DIB_RGB_COLORS) == 0) {
			fprintf(stderr,"Failed to determine screen format\n");
			ReleaseDC(hwndMain,screenDC);
			DeleteObject(sbmp);
			free_bitmap();
			return 0;
		}

		// copy down bits/pixel. leave the BITMAPINFOHEADER intact */
		// On Windows Vista/7/8 
		dibBitsPerPixel = dibBmpInfo->bmiHeader.biBitCount;

		if (announce_fmt) {
			fprintf(stderr,"Using autodetected GDI format:\n");
			if (dibBmpInfo->bmiHeader.biCompression == BI_BITFIELDS) {
				BITMAPV4HEADER *v4 = (BITMAPV4HEADER*)(&dibBmpInfo->bmiHeader);

				fprintf(stderr,"  BI_BITFIELDS: %ubpp red=0x%08lx green=0x%08lx blue=0x%08lx alpha=0x%08lx\n",
						dibBmpInfo->bmiHeader.biBitCount,
						v4->bV4RedMask,
						v4->bV4GreenMask,
						v4->bV4BlueMask,
						v4->bV4AlphaMask);
			}
			else {
				fprintf(stderr,"  BI_RGB: %ubpp\n",
						dibBmpInfo->bmiHeader.biBitCount);
			}
		}

		if (dibBmpInfo->bmiHeader.biCompression == BI_BITFIELDS) {
			BITMAPV4HEADER *v4 = (BITMAPV4HEADER*)(&dibBmpInfo->bmiHeader);

			dibBmpInfo->bmiHeader.biClrImportant = 0;
			dibBmpInfo->bmiHeader.biClrUsed = 0;
			v4->bV4CSType = 0;

			// HACK: Even with BI_BITFIELDS, GetDIBits still sets biSize == sizeof(BITMAPINFOHEADER).
			//       Our code below needs biSize == sizeof(BITMAPV4HEADER) for bitfields to work!
			dibBmpInfo->bmiHeader.biSize = sizeof(BITMAPV4HEADER);
		}

		DeleteObject(sbmp);
		ReleaseDC(hwndMain,screenDC);
		// NTS: Experience says you release screen DC with your window. Windows won't release with hwnd == NULL.
		//      I used to leak a lot of resources in the Windows 9x/ME days before I learned this...
	}
	else { /* gdi_force_depth != 0 */
		dibBitsPerPixel = gdi_force_depth;
		dibBmpInfo->bmiHeader.biPlanes = 1;

		if (dibBitsPerPixel == 15)
			dibBmpInfo->bmiHeader.biBitCount = 16;
		else
			dibBmpInfo->bmiHeader.biBitCount = dibBitsPerPixel;

		// next, forge a BITMAPINFOHEADER
		// NOTES: As BI_RGB, GDI will take 1/4/8/16/24/32bpp
		//        As BI_BITFIELDS, GDI will take 16/32bpp
		if (use_bitfields && dibBitsPerPixel != 24) {
			BITMAPV4HEADER *v4 = (BITMAPV4HEADER*)(&dibBmpInfo->bmiHeader);

			dibBmpInfo->bmiHeader.biSize = sizeof(BITMAPV4HEADER);
			dibBmpInfo->bmiHeader.biCompression = BI_BITFIELDS;

			if (dibBitsPerPixel == 15) {
				v4->bV4AlphaMask = (0x1U << 15U);
				v4->bV4RedMask = (0x1FU << (rgba_order ? 0U : 10U));
				v4->bV4GreenMask = (0x1FU << 5U);
				v4->bV4BlueMask = (0x1FU << (rgba_order ? 10U : 0U));
			}
			else if (dibBitsPerPixel == 16) {
				v4->bV4AlphaMask = 0;
				v4->bV4RedMask = (0x1FU << (rgba_order ? 0U : 11U));
				v4->bV4GreenMask = (0x3FU << 5U);
				v4->bV4BlueMask = (0x1FU << (rgba_order ? 11U : 0U));
			}
			else if (dibBitsPerPixel == 32) {
				v4->bV4AlphaMask = (0xFFU << 24U);
				v4->bV4RedMask = (0xFFU << (rgba_order ? 0U : 16U));
				v4->bV4GreenMask = (0xFFU << 8U);
				v4->bV4BlueMask = (0xFFU << (rgba_order ? 16U : 0U));
			}
			else {
				fprintf(stderr,"Unsupported bit depth\n");
				free_bitmap();
				return 0;
			}
		}
		else {
			dibBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			dibBmpInfo->bmiHeader.biCompression = BI_RGB;

			if (!(dibBitsPerPixel == 15 || dibBitsPerPixel == 16 || dibBitsPerPixel == 24 || dibBitsPerPixel == 32)) {
				fprintf(stderr,"Unsupported bit depth\n");
				free_bitmap();
				return 0;
			}
		}

		if (announce_fmt)
			fprintf(stderr,"Using forced GDI depth\n");
	}

	if (dibBitsPerPixel <= 8) {
		fprintf(stderr,"Unsupported BITSPIXEL %u\n",dibBitsPerPixel);
		free_bitmap();
		return 0;
	}

	// enforce DIB alignment
	if (dibBitsPerPixel >= 8) {
		unsigned int gdi_align = ((dibBitsPerPixel+7)/8) * 4; // GDI likes 4-pixel alignment

		if (align == 0)
			align = gdi_align;
		else {
			align += gdi_align - 1;
			align -= align % gdi_align;
		}
	}

	// stride now, with alignment.
	// then we back-convert to a width.
	// Windows DIBs have their own alignment calculation.
	pwidth = w * ((dibBitsPerPixel+7)/8);
	pwidth += align - 1;
	pwidth -= pwidth % align;

	// 24bpp safety padding.
	// there is a convention in this code to work on 24bpp pixels by typecasting
	// the 3 bytes as uint32_t and operating on the lowest 24 bits. unfortunately
	// this means 1 byte past the pixel is read/written. that means that if we set
	// the stride and length of the bitmap exactly around 24bpp we risk a segfault
	// accessing 1 byte past end of buffer, therefore we must add padding.
	if (!no24bpp_pad && dibBitsPerPixel == 24 && pwidth == (w * 3)) {
		fprintf(stderr,"24bpp safety adjustment adding 1 pixel of padding per scanline\n");
		pwidth += align;
	}

	dibBmpInfo->bmiHeader.biWidth = pwidth / ((dibBitsPerPixel+7)/8); // use back-conversion from stride/align calculation
	dibBmpInfo->bmiHeader.biHeight = -h;
	dibBmpInfo->bmiHeader.biSizeImage = pwidth * h;

	dibBits = NULL;
	dibBitmap = CreateDIBSection(dibDC,dibBmpInfo,DIB_RGB_COLORS,(void**)(&dibBits),NULL,0);
	if (dibBitmap == NULL) {
		fprintf(stderr,"CreateDIBSection failed\n");
		free_bitmap();
		return 0;
	}

	// store the previous "bitmap" to restore it later.
	// experience says that deleting a bitmap while selected in a DC doesn't work.
	// it's not obvious, and if you aren't aware of this, you'll leak bitmaps and cause problems.
	dibOldBitmap = (HBITMAP)SelectObject(dibDC,dibBitmap);

	gdi_bitmap.clear();
	gdi_bitmap.width = w;
	gdi_bitmap.height = h;
	gdi_bitmap.bytes_per_pixel = (dibBitsPerPixel + 7) / 8;
	gdi_bitmap.stride = pwidth;
	gdi_bitmap.update_length_from_stride_and_height();
	gdi_bitmap.base = dibBits;
	gdi_bitmap.canvas = dibBits;

	if (dibBmpInfo->bmiHeader.biSize >= sizeof(BITMAPV4HEADER) && dibBmpInfo->bmiHeader.biCompression == BI_BITFIELDS) {
		BITMAPV4HEADER *v4 = (BITMAPV4HEADER*)(&dibBmpInfo->bmiHeader);

		if (announce_fmt)
			fprintf(stderr,"Using GDI RGB bitfields %ubpp red=0x%08lx green=0x%08lx blue=0x%08lx alpha=0x%08lx\n",
				dibBitsPerPixel,v4->bV4RedMask,v4->bV4GreenMask,v4->bV4BlueMask,v4->bV4AlphaMask);

		gdi_bitmap.rgbinfo.r.setByMask(v4->bV4RedMask);
		gdi_bitmap.rgbinfo.g.setByMask(v4->bV4GreenMask);
		gdi_bitmap.rgbinfo.b.setByMask(v4->bV4BlueMask);
		gdi_bitmap.rgbinfo.a.setByMask(v4->bV4AlphaMask);
	}
	else {
		if (announce_fmt)
			fprintf(stderr,"Assuming RGB bitfields from %ubpp\n",dibBitsPerPixel);

		switch (dibBitsPerPixel) {
			case 32: // WINGDI is XRGB (B is LSByte, X is MSByte)
				gdi_bitmap.rgbinfo.b.setByShiftOffset(0,8); /* (offset,length) */
				gdi_bitmap.rgbinfo.g.setByShiftOffset(8,8);
				gdi_bitmap.rgbinfo.r.setByShiftOffset(16,8);
				gdi_bitmap.rgbinfo.a.setByShiftOffset(24,8);
				break;
			case 24: // WINGDI is RGB (B is LSByte, R is MSByte)
			default:
				gdi_bitmap.rgbinfo.b.setByShiftOffset(0,8); /* (offset,length) */
				gdi_bitmap.rgbinfo.g.setByShiftOffset(8,8);
				gdi_bitmap.rgbinfo.r.setByShiftOffset(16,8);
				gdi_bitmap.rgbinfo.a.setByShiftOffset(0,0);
				break;
			case 15: // WINGDI is XRGB 5/5/5 (XRRRRRGGGGGBBBBB)
			case 16:
				gdi_bitmap.rgbinfo.b.setByShiftOffset(0,5); /* (offset,length) */
				gdi_bitmap.rgbinfo.g.setByShiftOffset(5,5);
				gdi_bitmap.rgbinfo.r.setByShiftOffset(10,5);
				gdi_bitmap.rgbinfo.a.setByShiftOffset(0,0);
				break;
		}
	}

	if (announce_fmt) {
		announce_fmt = false;
		fprintf(stderr,"R/G/B/A shift/width/mask %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X\n",
				gdi_bitmap.rgbinfo.r.shift, gdi_bitmap.rgbinfo.r.bwidth, gdi_bitmap.rgbinfo.r.bmask,
				gdi_bitmap.rgbinfo.g.shift, gdi_bitmap.rgbinfo.g.bwidth, gdi_bitmap.rgbinfo.g.bmask,
				gdi_bitmap.rgbinfo.b.shift, gdi_bitmap.rgbinfo.b.bwidth, gdi_bitmap.rgbinfo.b.bmask,
				gdi_bitmap.rgbinfo.a.shift, gdi_bitmap.rgbinfo.a.bwidth, gdi_bitmap.rgbinfo.a.bmask);
	}

	return 1;
}

void update_screen(HDC targetDC) {
	if (dibDC == NULL) return;
	if (!gdi_bitmap.is_valid()) return;

	if (!BitBlt(targetDC,0,0,gdi_bitmap.width,gdi_bitmap.height,dibDC,0,0,SRCCOPY))
		fprintf(stderr,"update_screen() warning BitBlt failed\n");
}

// NTS: GCC/MinGW assumes 16-byte stack alignment. Windows uses 4-byte stack alignment.
//      If supported by the compiler, we tell the compiler to force realignment in this function.
//      Failure to do so will cause random crashes when other code called from this point attempts
//      to use SSE intrinsics on the stack! That is what "SSE_REALIGN" means.
static SSE_REALIGN LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE:
			break;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd,&ps);
			update_screen(hdc);
			EndPaint(hwnd,&ps);
			} break;
		case WM_SIZE:
			if (!init_bitmap(LOWORD(lParam),HIWORD(lParam)))
				fprintf(stderr,"WARNING WM_RESIZE init_bitmap(%u,%u) failed\n",
					LOWORD(lParam),HIWORD(lParam));

			if (resize_src_mode) {
				free_src_bitmap();
				if (!init_src_bitmap(gdi_bitmap.width,gdi_bitmap.height))
					break;
				test_pattern_1_render(/*&*/src_bitmap);
			}

			if (method <= stretchblt_mode_count())
				stretchblt_modes[method].render(/*&*/gdi_bitmap,/*&*/src_bitmap);

			InvalidateRect(hwndMain,NULL,FALSE); // DWM compositor-based versions set WM_PAINT such that only the affected area will repaint
			break;
		case WM_KEYDOWN:
			switch (wParam) {
				case VK_ESCAPE:
					PostMessage(hwnd,WM_CLOSE,0,0);
					break;
				case 'S':
					resize_src_mode = !resize_src_mode;

					if (resize_src_mode) {
						free_src_bitmap();
						if (!init_src_bitmap(gdi_bitmap.width,gdi_bitmap.height))
							break;
						test_pattern_1_render(/*&*/src_bitmap);
					}

					if (method <= stretchblt_mode_count())
						stretchblt_modes[method].render(/*&*/gdi_bitmap,/*&*/src_bitmap);

					InvalidateRect(hwnd,NULL,FALSE);
					break;
				case VK_SPACE:
					do {
						if ((++method) >= stretchblt_mode_count())
							method = 0;

						if (stretchblt_modes[method].can_do(/*&*/gdi_bitmap,/*&*/src_bitmap))
							break;
						fprintf(stderr,"Can't do %s, skipping\n",stretchblt_modes[method].name);
					} while (1);

					fprintf(stderr,"Switching to %s\n",stretchblt_modes[method].name);

					if (method <= stretchblt_mode_count())
						stretchblt_modes[method].render(/*&*/gdi_bitmap,/*&*/src_bitmap);

					InvalidateRect(hwnd,NULL,FALSE);
					break;
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}

	return 0;
}

int main(int argc,char **argv) {
	WNDCLASS wnd;
	RECT rect;
	MSG msg;
	char *a;
	int i;

    hostCPUcaps.detect();

	for (i=1;i < argc;) {
		a = argv[i++];

		if (*a == '-') {
			do { a++; } while (*a == '-');

			if (!strcmp(a,"depth")) {
				a = argv[i++];
				if (a == NULL) return 1;
				gdi_force_depth = atoi(a);
			}
			else if (!strcmp(a,"bitfields")) {
				use_bitfields = true;
			}
			else if (!strcmp(a,"rgba")) {
				rgba_order = true;
			}
			else if (!strcmp(a,"no24pad")) {
				no24bpp_pad = true;
			}
			else if (!strcmp(a,"nodpiaware")) {
				gdi_no_dpiaware = true;
			}
			else {
				fprintf(stderr,"Unhandled switch %s\n",a);
				return 1;
			}
		}
		else {
			fprintf(stderr,"Unhandled arg %s\n",a);
			return 1;
		}
	}

	if (sizeof(rgb24bpp_t) != 3)
		fprintf(stderr,"WARNING: uint24_t is not 3 bytes long, it is %zu bytes\n",sizeof(rgb24bpp_t));

	/* Please don't scale me in the name of "DPI awareness" */
	if (!gdi_no_dpiaware)
		win32_dpi_aware();

	myInstance = GetModuleHandle(NULL);

	memset(&wnd,0,sizeof(wnd));
	wnd.lpfnWndProc = WndProc;
	wnd.hInstance = myInstance;
	wnd.lpszClassName = hwndMainClass;
	wnd.hCursor = LoadCursor(NULL,IDC_ARROW);

	if (!RegisterClass(&wnd)) {
		fprintf(stderr,"RegisterClass() failed\n");
		return 1;
	}

	rect.top = 0;
	rect.left = 0;
	rect.right = 640;
	rect.bottom = 480;
	AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);

	hwndMain = CreateWindow(hwndMainClass,hwndMainTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,
		rect.right,rect.bottom,
		NULL,NULL,
		myInstance,NULL);
	if (!hwndMain) {
		fprintf(stderr,"CreateWindow() failed\n");
		return 1;
	}

	GetClientRect(hwndMain,&rect);
	if (!init_bitmap(rect.right,rect.bottom)) {
		fprintf(stderr,"nit_bitmap() failed for %ux%u\n",rect.right,rect.bottom);
		return 1;
	}
	if (!init_src_bitmap(640,480)) {
		fprintf(stderr,"failed to init src bitmap\n");
		return 1;
	}
	test_pattern_1_render(/*&*/src_bitmap);

	do {
		if (stretchblt_modes[method].can_do(/*&*/gdi_bitmap,/*&*/src_bitmap))
			break;
		fprintf(stderr,"Can't do %s, skipping\n",stretchblt_modes[method].name);

		if ((++method) >= stretchblt_mode_count())
			method = 0;
	} while (1);

	if (method <= stretchblt_mode_count())
		stretchblt_modes[method].render(/*&*/gdi_bitmap,/*&*/src_bitmap);

	ShowWindow(hwndMain,SW_SHOW);
	UpdateWindow(hwndMain);

	while (GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	free_src_bitmap();
	free_bitmap();
	return 0;
}
