
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

HMENU                           menuDisplayModes = NULL;

HWND                            hwndMain = NULL;
const char*                     hwndMainClass = "WINGDICLASS";
const char*                     hwndMainTitle = "WinGDI test 1";
HINSTANCE                       myInstance;

HDC                             dibDC = NULL;
BITMAPINFO*                     dibBmpInfo;
unsigned char                   dibBmpInfoRaw[sizeof(BITMAPV4HEADER) + (256*sizeof(RGBQUAD))];
HBITMAP                         dibOldBitmap = NULL,dibBitmap = NULL;    // you can't free a bitmap if it's selected into a DC
unsigned int                    dibBitsPerPixel = 0;
unsigned char*                  dibBits = NULL;

bool                            gdi_no_dpiaware = false;
unsigned int                    gdi_force_depth = 0;

rgb_bitmap_info                 gdi_bitmap;
bool                            announce_fmt = true;
bool                            use_bitfields = false;
bool                            rgba_order = false;
bool                            no24bpp_pad = false;
bool                            change_display_back = false;

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
                        (unsigned long)v4->bV4RedMask,
                        (unsigned long)v4->bV4GreenMask,
                        (unsigned long)v4->bV4BlueMask,
                        (unsigned long)v4->bV4AlphaMask);
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
                (unsigned int)dibBitsPerPixel,(unsigned long)v4->bV4RedMask,(unsigned long)v4->bV4GreenMask,(unsigned long)v4->bV4BlueMask,(unsigned long)v4->bV4AlphaMask);

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

#define MAX_DISPLAY_MODES 1024

#ifndef DM_DISPLAYFIXEDOUTPUT
# define DM_DISPLAYFIXEDOUTPUT   0x20000000L
#endif

#ifndef DMDFO_DEFAULT
# define DMDFO_DEFAULT           0
#endif

#ifndef DMDFO_STRETCH
# define DMDFO_STRETCH           1
#endif

#ifndef DMDFO_CENTER
# define DMDFO_CENTER            2
#endif

DEVMODE displayModes[MAX_DISPLAY_MODES];
int displayModesCount = 0;

void populateDisplayModes(void) {
    char tmp[128],*w,*wf;
    DEVMODE devmode;
    int count=0,index;

    if (menuDisplayModes != NULL) return;
    menuDisplayModes = CreatePopupMenu();
    if (menuDisplayModes == NULL) return;
    displayModesCount = 0;
    index = 0;

    do {
        if (index >= MAX_DISPLAY_MODES) break;
        memset(&devmode,0,sizeof(devmode));
        devmode.dmSize = sizeof(devmode);
        if (!EnumDisplaySettings(NULL,(DWORD)index,&devmode)) break;

        w = tmp;
        wf = tmp + sizeof(tmp) - 1;

        if (devmode.dmFields & DM_PELSWIDTH) {
            w += snprintf(w,(size_t)(wf-w),"%ux%u",
                    (unsigned int)devmode.dmPelsWidth,
                    (unsigned int)devmode.dmPelsHeight);
        }
        if ((devmode.dmFields & DM_DISPLAYFLAGS) && w < wf)
            *w++ = (devmode.dmDisplayFlags&DM_INTERLACED)?'i':'p';
        if (devmode.dmFields & DM_BITSPERPEL) {
            w += snprintf(w,(size_t)(wf-w)," x %ubpp",
                    (unsigned int)devmode.dmBitsPerPel);
        }
        if (devmode.dmFields & DM_DISPLAYFREQUENCY) {
            w += snprintf(w,(size_t)(wf-w)," @%uHz",
                    (unsigned int)devmode.dmDisplayFrequency);
        }
        if (devmode.dmFields & DM_DISPLAYFIXEDOUTPUT) {
            if (devmode.dmDisplayFixedOutput == DMDFO_CENTER)
                w += snprintf(w,(size_t)(wf-w)," fixed-centered");
            else if (devmode.dmDisplayFixedOutput == DMDFO_STRETCH)
                w += snprintf(w,(size_t)(wf-w)," fixed-stretched");
            else if (devmode.dmDisplayFixedOutput == DMDFO_DEFAULT) /* which apparently means "fixed-stretched" but preserve aspect ratio */
                w += snprintf(w,(size_t)(wf-w)," fixed-default");
        }
        else {
            /* which apparently means "fixed-stretchedd" but preserve aspect ratio */
        }

        AppendMenu(menuDisplayModes,MF_ENABLED|MF_STRING,4000+displayModesCount,tmp);
        displayModes[displayModesCount++] = devmode;
        index++;
        count++;
    } while(1);

    if (count == 0) AppendMenu(menuDisplayModes,MF_DISABLED|MF_GRAYED|MF_STRING,0,"(none)");
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
        case WM_DISPLAYCHANGE:
            announce_fmt = true;
            if (gdi_bitmap.width == 0 || gdi_bitmap.height == 0) {
                RECT rct;
                GetClientRect(hwnd,&rct);
                gdi_bitmap.width = rct.right;
                gdi_bitmap.height = rct.bottom;
            }
            if (!init_bitmap(gdi_bitmap.width,gdi_bitmap.height))
                fprintf(stderr,"WARNING WM_RESIZE init_bitmap(%u,%u) failed\n",
                    gdi_bitmap.width,gdi_bitmap.height);
            render_test_pattern_rgb_gradients(gdi_bitmap);
            InvalidateRect(hwndMain,NULL,FALSE); // DWM compositor-based versions set WM_PAINT such that only the affected area will repaint
            return DefWindowProc(hwnd,uMsg,wParam,lParam);
        case WM_SIZE:
            if (!init_bitmap(LOWORD(lParam),HIWORD(lParam)))
                fprintf(stderr,"WARNING WM_RESIZE init_bitmap(%u,%u) failed\n",
                    LOWORD(lParam),HIWORD(lParam));
            render_test_pattern_rgb_gradients(gdi_bitmap);
            InvalidateRect(hwndMain,NULL,FALSE); // DWM compositor-based versions set WM_PAINT such that only the affected area will repaint
            break;
        case WM_COMMAND:
            if (wParam >= 4000 && wParam < (WPARAM)(4000+displayModesCount)) {
                LONG ret = ChangeDisplaySettings(&displayModes[wParam-4000],CDS_FULLSCREEN);
                change_display_back = true;

                if (ret == DISP_CHANGE_BADDUALVIEW)
                    MessageBox(hwnd,"","Failed, bad dual view",MB_OK);
                else if (ret == DISP_CHANGE_BADFLAGS)
                    MessageBox(hwnd,"","Failed, bad flags",MB_OK);
                else if (ret == DISP_CHANGE_BADMODE)
                    MessageBox(hwnd,"","Failed, bad mode",MB_OK);
                else if (ret == DISP_CHANGE_BADPARAM)
                    MessageBox(hwnd,"","Failed, bad param",MB_OK);
                else if (ret == DISP_CHANGE_FAILED)
                    MessageBox(hwnd,"","Failed, failed",MB_OK);
                else if (ret == DISP_CHANGE_NOTUPDATED)
                    MessageBox(hwnd,"","Failed, not updated",MB_OK);
                else if (ret == DISP_CHANGE_RESTART)
                    MessageBox(hwnd,"","Failed, need restart",MB_OK);
            }
            else {
                return DefWindowProc(hwnd,uMsg,wParam,lParam);
            }
            break;
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_ESCAPE:
                    PostMessage(hwnd,WM_CLOSE,0,0);
                    break;
            }
            break;
        case WM_RBUTTONUP:
            populateDisplayModes();
            if (menuDisplayModes != NULL) {
                POINT pt;

                GetCursorPos(&pt);
                TrackPopupMenu(menuDisplayModes,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,0,hwnd,NULL);
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
        fprintf(stderr,"WARNING: uint24_t is not 3 bytes long, it is %u bytes\n",(unsigned int)sizeof(rgb24bpp_t));

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
        rect.right - rect.left,rect.bottom - rect.top,
        NULL,NULL,
        myInstance,NULL);
    if (!hwndMain) {
        fprintf(stderr,"CreateWindow() failed\n");
        return 1;
    }

    GetClientRect(hwndMain,&rect);
    if (!init_bitmap(rect.right,rect.bottom)) {
        fprintf(stderr,"nit_bitmap() failed for %ux%u\n",(unsigned int)rect.right,(unsigned int)rect.bottom);
        return 1;
    }
    render_test_pattern_rgb_gradients(gdi_bitmap);

    ShowWindow(hwndMain,SW_SHOW);
    UpdateWindow(hwndMain);

    while (GetMessage(&msg,NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (menuDisplayModes != NULL) {
        DestroyMenu(menuDisplayModes);
        menuDisplayModes = NULL;
    }

    if (change_display_back)
        ChangeDisplaySettings(NULL,0);

    return 0;
}
