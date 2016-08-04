
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

#include "dosboxxr/lib/hostcpudetect/caps.h"
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_patterns.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"
#include "dosboxxr/lib/graphics/stretchblt_general.h"

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

	dibDC = CreateCompatibleDC(NULL);
	if (dibDC == NULL) {
		fprintf(stderr,"Unable to create compatible DC\n");
		free_bitmap();
		return 0;
	}

	if (gdi_force_depth != 0)
		dibBitsPerPixel = gdi_force_depth;
	else
		dibBitsPerPixel = GetDeviceCaps(dibDC,BITSPIXEL);

	if (dibBitsPerPixel <= 8) {
		fprintf(stderr,"Unsupported BITSPIXEL %u\n",dibBitsPerPixel);
		free_bitmap();
		return 0;
	}

	// enforce DIB alignment
	if (dibBitsPerPixel > 16) {
		align = (align + 3) & (~3); // must be multiple of 4
		if (align == 0) align = 4;
	}
	else {
		align = (align + 7) & (~7); // must be multiple of 8
		if (align == 0) align = 8;
	}

	// stride now, with alignment.
	// then we back-convert to a width.
	// Windows DIBs have their own alignment calculation.
	pwidth = w * ((dibBitsPerPixel+7)/8);
	pwidth += align - 1;
	pwidth -= pwidth % align;

	dibBmpInfo = (BITMAPINFO*)dibBmpInfoRaw;
	memset(dibBmpInfoRaw,0,sizeof(dibBmpInfoRaw));
	// NTS: The GDI will tell us bits per pixel but won't give us the exact placement
	// of R/G/B bitfields i.e. we can't tell if 16bpp means RGB5/5/5 or RGB5/6/5.
	// However WinGDI 16bpp is always RGB5/5/5 anyway (unless you explicitly use BI_BITFIELDS
	// to make it RGB5/6/5), but the point is, if the screen is RGB5/6/5 or RGB5/5/5 we can't
	// tell from the GDI interfaces and we have to guess.
	//
	// Also note we autodetect from the screen, but WinGDI is fully supportive of our efforts
	// if we choose to create a DIB section that is not the screen format, it will convert.
	if (use_bitfields) {
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
		else if (dibBitsPerPixel == 24) {
			v4->bV4AlphaMask = 0;
			v4->bV4RedMask = (0xFFU << (rgba_order ? 0U : 16U));
			v4->bV4GreenMask = (0xFFU << 8U);
			v4->bV4BlueMask = (0xFFU << (rgba_order ? 16U : 0U));
		}
		else if (dibBitsPerPixel == 30) { // Intel chipsets support a 10-bit ARGB format. Does this work? It's worth a try...
			v4->bV4AlphaMask = (0x3U << 30U);
			v4->bV4RedMask = (0x3FFU << (rgba_order ? 0U : 20U));
			v4->bV4GreenMask = (0x3FFU << 10U);
			v4->bV4BlueMask = (0x3FFU << (rgba_order ? 20U : 0U));
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

		if (!(dibBitsPerPixel == 15 || dibBitsPerPixel == 16 || dibBitsPerPixel == 24 || dibBitsPerPixel == 32)) {
			fprintf(stderr,"Unsupported bit depth\n");
			free_bitmap();
			return 0;
		}
	}
	dibBmpInfo->bmiHeader.biWidth = pwidth / ((dibBitsPerPixel+7)/8); // use back-conversion from stride/align calculation
	dibBmpInfo->bmiHeader.biHeight = -h;
	dibBmpInfo->bmiHeader.biPlanes = 1;
	if (dibBitsPerPixel == 15)
		dibBmpInfo->bmiHeader.biBitCount = 16;
	else
		dibBmpInfo->bmiHeader.biBitCount = dibBitsPerPixel;
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

		gdi_bitmap.rgbinfo.r.setByMask(v4->bV4RedMask);
		gdi_bitmap.rgbinfo.g.setByMask(v4->bV4GreenMask);
		gdi_bitmap.rgbinfo.b.setByMask(v4->bV4BlueMask);
		gdi_bitmap.rgbinfo.a.setByMask(v4->bV4AlphaMask);
	}
	else {
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

static LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
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
			render_test_pattern_rgb_gradients(gdi_bitmap);
			InvalidateRect(hwndMain,NULL,FALSE); // DWM compositor-based versions set WM_PAINT such that only the affected area will repaint
			break;
		case WM_KEYDOWN:
			switch (wParam) {
				case VK_ESCAPE:
					PostMessage(hwnd,WM_CLOSE,0,0);
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

	if (sizeof(uint24_t) != 3)
		fprintf(stderr,"WARNING: uint24_t is not 3 bytes long, it is %zu bytes\n",sizeof(uint24_t));

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
	render_test_pattern_rgb_gradients(gdi_bitmap);

	ShowWindow(hwndMain,SW_SHOW);
	UpdateWindow(hwndMain);

	while (GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
