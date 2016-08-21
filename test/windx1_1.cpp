
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <windows.h>

#include <ddraw.h>

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

IDirectDraw*                    ddraw = NULL;
DWORD                           ddrawCooperativeLevel = 0;
bool                            hasSetMode = false; // NTS: Windows XP acts funny if a program goes into exclusive fullscreen without having set a video mode

IDirectDrawClipper*             ddclipperPrimary = NULL;
IDirectDrawSurface*             ddsurfacePrimary = NULL;
IDirectDrawSurface*             ddsurfaceGDI = NULL;    // Microsoft is very unclear about this: You can only get the GDI surface in fullscreen/exclusive. BUT, you can create a primary surface of the screen!
IDirectDrawSurface*             ddsurfaceBMP = NULL;

DWORD                           hwndMainStyle;
HWND                            hwndMain = NULL;
const char*                     hwndMainClass = "WINDX1CLASS";
const char*                     hwndMainTitle = "WinDirectDraw test 1";
HINSTANCE                       myInstance;

bool                            gdi_no_dpiaware = false;

rgb_bitmap_info                 dx_bitmap;
bool                            announce_fmt = true;

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

bool lock_bitmap(void) {
    DDSURFACEDESC ddsurf;
    HRESULT hr;

    if (ddsurfaceBMP == NULL)
        return false;
    if (dx_bitmap.base != NULL)
        return true;

    memset(&ddsurf,0,sizeof(ddsurf));
    ddsurf.dwSize = sizeof(ddsurf);

    if ((hr=ddsurfaceBMP->Lock(NULL,&ddsurf,0,NULL)) != DD_OK) {
        fprintf(stderr,"Failed to lock BMP surface, HR=0x%08lx\n",hr);
        return false;
    }

    dx_bitmap.base = dx_bitmap.canvas = (unsigned char*)ddsurf.lpSurface;
    return true;
}

bool unlock_bitmap(void) {
    HRESULT hr;

    if (ddsurfaceBMP == NULL)
        return false;
    if (dx_bitmap.base == NULL)
        return true;

    if ((hr=ddsurfaceBMP->Unlock(NULL)) != DD_OK) {
        fprintf(stderr,"Failed to unlock BMP surface, HR=0x%08lx\n",hr);
        return false;
    }

    dx_bitmap.base = dx_bitmap.canvas = NULL;
    return true;
}

void free_bitmap(void) {
    dx_bitmap.clear();

    if (ddsurfaceBMP != NULL) {
        ddsurfaceBMP->Release();
        ddsurfaceBMP = NULL;
    }
}

void free_dx_primary_surface(void) {
    if (ddclipperPrimary != NULL) {
        ddclipperPrimary->Release();
        ddclipperPrimary = NULL;
    }
    if (ddsurfacePrimary != NULL) {
        ddsurfacePrimary->Release();
        ddsurfacePrimary = NULL;
    }
    if (ddsurfaceGDI != NULL) {
        ddsurfaceGDI->Release();
        ddsurfaceGDI = NULL;
    }
}

bool init_dx_primary_surface(void) {
    HRESULT hr;

    if (ddsurfacePrimary != NULL || ddclipperPrimary != NULL)
        return true;

    if (hwndMain == NULL) {
        fprintf(stderr,"Bug: attempt to init DX primary surface without creating window first\n");
        return false;
    }

    /* create a primary surface so that we can blt to the screen. I don't intend to lock the display surface, I only intend
       to hardware blit to it. Also, trying to directly access the screen in Windows Vista and higher causes the DWM compositor
       to blank the screen and fallback to a compat mode, which is visually unappealing. */
    /* NTS: Past experience from 1997-1999 says that even creating a primary surface this way won't work if you're using
       ancient SVGA graphics cards with bank-switched RAM and no linear framebuffer (like a TSENG ET4000). But that
       wasn't really a problem back then with games who knew how to fallback to Windows GDI anyway. */
    {
        DDSURFACEDESC ddsurf;

        memset(&ddsurf,0,sizeof(ddsurf));
        ddsurf.dwSize = sizeof(ddsurf);
        ddsurf.dwFlags = DDSD_CAPS;
        ddsurf.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        if ((hr=ddraw->CreateSurface(&ddsurf,&ddsurfacePrimary,NULL)) != DD_OK) {
            fprintf(stderr,"Unable to create primary surface (screen), HR=0x%08lx\n",hr);
            return false;
        }
    }

    if ((hr=ddraw->CreateClipper(0,&ddclipperPrimary,NULL)) != DD_OK) {
        fprintf(stderr,"Failed to create clipper, HR=0x%08lx\n",hr);
        return false;
    }
    if ((hr=ddclipperPrimary->SetHWnd(0,hwndMain)) != DD_OK) {
        fprintf(stderr,"Failed to set clipper to main window, HR=0x%08lx\n",hr);
        return false;
    }
    if ((hr=ddsurfacePrimary->SetClipper(ddclipperPrimary)) != DD_OK) {
        fprintf(stderr,"Failed to set clipper to primary surface, HR=0x%08lx\n",hr);
        return false;
    }

    /* Okay, now try to get GDI surface.
       I don't expect this to work. Feh. */
    if (ddsurfaceGDI == NULL && (hr=ddraw->GetGDISurface(&ddsurfaceGDI)) != DD_OK)
        fprintf(stderr,"Oh, gee, couldn't get the GDI surface (HR=0x%08lx), oh golly what a surprise (not)\n",hr);

    return true;
}

int init_bitmap(unsigned int w,unsigned int h,unsigned int align=32) {
    DDSURFACEDESC ddsurf,dispsurf;
    HRESULT hr;

    free_bitmap();
    if (w == 0 || h == 0) return 0;

    (void)align; // unused

    /* what screen format? we need to add padding to width just in case 24bpp */
    memset(&dispsurf,0,sizeof(dispsurf));
    dispsurf.dwSize = sizeof(dispsurf);
    if ((hr=ddraw->GetDisplayMode(&dispsurf)) != DD_OK) {
        fprintf(stderr,"Failed to query display mode HR=0x%08lx\n",hr);
        free_bitmap();
        return 0;
    }

    fprintf(stderr,"Screen is %u x %u x %ubpp\n",
        (unsigned int)dispsurf.dwWidth,
        (unsigned int)dispsurf.dwHeight,
        (unsigned int)dispsurf.ddpfPixelFormat.dwRGBBitCount);

    // NTS: We used to let DirectX auto-choose the primary surface depth for us, but... it turns out
    //      some versions of Windows have problems here when the screen has just changed bit depth.
    //      But GetDisplayMode() returns the new bit depth, so we can provide it anyway.
    //
    // Case 1: Windows XP: Changing from a higher to lower bit depth: CreateSurface with OFFSCREENPLAIN
    //         will make a surface with the stride and allocation of the new bit depth but the RGB bits
    //         per pixel of the old bit depth. Blind adherence to the bit depth will mean that we overrun
    //         the buffer, and crash.
    memset(&ddsurf,0,sizeof(ddsurf));
    ddsurf.dwSize = sizeof(ddsurf);
    ddsurf.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_PITCH;
    ddsurf.ddpfPixelFormat = dispsurf.ddpfPixelFormat;
    ddsurf.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsurf.dwWidth = (w+3) & (~3); // NTS: Windows XP does weird things with 24bpp if width not a multiple of 4, including resetting lPitch to width * 3 ha ha!
    ddsurf.dwHeight = h;
    ddsurf.lPitch = ((ddsurf.ddpfPixelFormat.dwRGBBitCount+7)/8) * ddsurf.dwWidth;

    // DirectX demands at least DWORD alignment
    if (align < 4) align = 4;
    align = (align+3) & (~3); // round up

    /* if 24bpp, may need extra padding */
    if (ddsurf.ddpfPixelFormat.dwRGBBitCount == 24 && (w&3) == 0) {
        ddsurf.dwWidth += 4; // NTS: only because DirectX in Windows XP appears to ignore our pitch spec when creating the surface. Also width must be multiple of 4.
        ddsurf.lPitch = ((ddsurf.ddpfPixelFormat.dwRGBBitCount+7)/8) * ddsurf.dwWidth;
    }

    if (align > 0) {
        // try to encourage DirectX to set the pitch by our alignment.
        // Not that it works (from actual testing) but let's try anyway.
        ddsurf.lPitch += align - 1;
        ddsurf.lPitch -= ddsurf.lPitch % align;
    }

    hr = ddraw->CreateSurface(&ddsurf,&ddsurfaceBMP,NULL);
    if (hr == DDERR_INVALIDPARAMS && ddsurf.ddpfPixelFormat.dwRGBBitCount == 24) {
        // NTS: Apparently creating an offscreen surface wider than the screen is not supported by DirectX on Windows 95/98.
        //      We need the extra padding, so if we can't pad horizontally try padding vertically.
        fprintf(stderr,"Windows 9x/ME DirectX screen compenstation\n");

        ddsurf.dwHeight = h + 1;
        ddsurf.dwWidth = (w+3) & (~3); // NTS: Windows XP does weird things with 24bpp if width not a multiple of 4, including resetting lPitch to width * 3 ha ha!
        ddsurf.lPitch = ((ddsurf.ddpfPixelFormat.dwRGBBitCount+7)/8) * ddsurf.dwWidth;

        if (align > 0) {
            // try to encourage DirectX to set the pitch by our alignment.
            // Not that it works (from actual testing) but let's try anyway.
            ddsurf.lPitch += align - 1;
            ddsurf.lPitch -= ddsurf.lPitch % align;
        }

        hr = ddraw->CreateSurface(&ddsurf,&ddsurfaceBMP,NULL);
    }
    if (hr != DD_OK) {
        fprintf(stderr,"Failed to create BMP surface HR=0x%08lx\n",hr);
        free_bitmap();
        return 0;
    }

    memset(&ddsurf,0,sizeof(ddsurf));
    ddsurf.dwSize = sizeof(ddsurf);
    if ((hr=ddsurfaceBMP->GetSurfaceDesc(&ddsurf)) != DD_OK) {
        fprintf(stderr,"Failed to get BMP surface info HR=0x%08lx\n",hr);
        free_bitmap();
        return 0;
    }

    fprintf(stderr,"BMP surface info:");
    if ((ddsurf.dwFlags & (DDSD_WIDTH|DDSD_HEIGHT)) == (DDSD_WIDTH|DDSD_HEIGHT))
        fprintf(stderr," %lux%lu",(unsigned long)ddsurf.dwWidth,(unsigned long)ddsurf.dwHeight);
    if (ddsurf.dwFlags & DDSD_PITCH)
        fprintf(stderr," pitch=%lu",(unsigned long)ddsurf.lPitch);
    if (ddsurf.dwFlags & DDSD_PIXELFORMAT) {
        fprintf(stderr," pixelfmt=");
        if (ddsurf.ddpfPixelFormat.dwFlags & DDPF_RGB) {
            fprintf(stderr,"RGB=%ubpp ",(unsigned int)ddsurf.ddpfPixelFormat.dwRGBBitCount);
            fprintf(stderr,"R/G/B/A=0x%08lx/0x%08lx/0x%08lx/0x%08lx ",
                (unsigned long)ddsurf.ddpfPixelFormat.dwRBitMask,
                (unsigned long)ddsurf.ddpfPixelFormat.dwGBitMask,
                (unsigned long)ddsurf.ddpfPixelFormat.dwBBitMask,
                (unsigned long)ddsurf.ddpfPixelFormat.dwRGBAlphaBitMask);
        }
    }
    fprintf(stderr,"\n");

    {
        DWORD want = DDSD_WIDTH|DDSD_HEIGHT|DDSD_PITCH|DDSD_PIXELFORMAT;
        if ((ddsurf.dwFlags&want) != want) {
            fprintf(stderr,"DirectX did not provide sufficient information for me to work by\n");
            free_bitmap();
            return 0;
        }
    }

    if (!(ddsurf.ddpfPixelFormat.dwFlags & DDPF_RGB)) {
        fprintf(stderr,"DirectX did not provide pixel format or pixel format is not RGB\n");
        free_bitmap();
        return 0;
    }

    dx_bitmap.clear();
    dx_bitmap.width = w;
    dx_bitmap.height = h;
    dx_bitmap.bytes_per_pixel = (ddsurf.ddpfPixelFormat.dwRGBBitCount + 7) / 8;
    dx_bitmap.stride = ddsurf.lPitch;
    dx_bitmap.length = ddsurf.lPitch * ddsurf.dwHeight;

    dx_bitmap.rgbinfo.r.setByMask(ddsurf.ddpfPixelFormat.dwRBitMask);
    dx_bitmap.rgbinfo.g.setByMask(ddsurf.ddpfPixelFormat.dwGBitMask);
    dx_bitmap.rgbinfo.b.setByMask(ddsurf.ddpfPixelFormat.dwBBitMask);
    dx_bitmap.rgbinfo.a.setByMask(ddsurf.ddpfPixelFormat.dwRGBAlphaBitMask);

    // check!
    if (dx_bitmap.length < (dx_bitmap.height * dx_bitmap.stride)) {
        fprintf(stderr,"DirectX did not provide a surface with sufficient height\n");
        free_bitmap();
        return 0;
    }

    // 24bpp check: make sure the pitch (stride) is larger than width*3, or we were able to get at least one extra scanline
    if (ddsurf.ddpfPixelFormat.dwRGBBitCount == 24) {
        if (dx_bitmap.height == ddsurf.dwHeight) {
            if (dx_bitmap.stride <= (dx_bitmap.width*3)) {
                fprintf(stderr,"DirectX did not provide a surface with a stride padded enough for 24bpp\n");
                free_bitmap();
                return 0;
            }
        }
    }

    // check that we didn't catch DirectX mid-mode-change.
    // NTS: Apparently switching from 32bpp to 24bpp in Windows XP can cause an easily-hittable
    //      window of opportunity to create a primary surface with the stride of a 24bpp screen
    //      and the specification of a 32bpp screen. Needless to say, if we don't catch this,
    //      we will crash when we start to draw again.
    if ((dx_bitmap.width * dx_bitmap.bytes_per_pixel) > dx_bitmap.stride) {
        fprintf(stderr,"DirectX did not provide a valid surface pitch (Windows XP DirectX 24/32bpp mode change bug?)\n");
        free_bitmap();
        return 0;
    }

    // the catch with DirectX is that we cannot keep the pointer in the dx bitmap at all times,
    // because of the need to lock the surface. be aware of that. Until you lock the surface,
    // our dx_bitmap is technically invalid. I'm not going to just leave it locked because we
    // want DirectX to be able to Blit from our surface to the screen.
    dx_bitmap.base = dx_bitmap.canvas = NULL;

    return 1;
}

void update_screen() {
    IDirectDrawSurface *dst;
    RECT dstRect;
    RECT srcRect;
    HRESULT hr;
    POINT pt;

    // NTS: dx_bitmap.is_valid() is not a valid test because base==canvas==NULL unless surface is locked
    if (ddsurfaceBMP == NULL) return;

    // NTS: We must provide a source rect, else Windows XP in 24bpp mode will conveniently choose to render the
    //      right half (where extra padding+junk exist) rather than from the left side. Provide a source rect
    //      to make it 100% clear which part of the surface we want blitted to screen.
    pt.x = pt.y = 0;
    ClientToScreen(hwndMain,&pt);
    GetClientRect(hwndMain,&dstRect);
    OffsetRect(&dstRect,pt.x,pt.y);
    srcRect.top = srcRect.left = 0;
    srcRect.right = dx_bitmap.width;
    srcRect.bottom = dx_bitmap.height;

    if (ddsurfaceGDI != NULL)
        dst = ddsurfaceGDI;
    else if (ddsurfacePrimary != NULL)
        dst = ddsurfacePrimary;
    else
        dst = NULL;

    if (dst != NULL)
        hr = dst->Blt(&dstRect,ddsurfaceBMP,&srcRect,0,NULL);
    else
        hr = DD_OK;

    if (hr == DDERR_SURFACELOST) {
        fprintf(stderr,"Whoops, the primary surface was lost.\n");
        if ((hr=dst->Restore()) != DD_OK) {
            fprintf(stderr,"Unable to restore surface hr=0x%08lx.\n",hr);
            unlock_bitmap();
            free_bitmap();
            free_dx_primary_surface();
            init_dx_primary_surface();

            RECT rct;
            GetClientRect(hwndMain,&rct);

            if (!init_bitmap(rct.right,rct.bottom))
                fprintf(stderr,"WARNING WM_RESIZE init_bitmap(%u,%u) failed\n",
                        (unsigned int)rct.right,(unsigned int)rct.bottom);

            if (lock_bitmap()) {
                render_test_pattern_rgb_gradients(dx_bitmap);
                unlock_bitmap();
            }

            InvalidateRect(hwndMain,NULL,FALSE); // DWM compositor-based versions set WM_PAINT such that only the affected area will repaint
        }
    }
    else if (hr != DD_OK) {
        fprintf(stderr,"DirectX blit failed, HR=0x%08lx\n",hr);
    }
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

#ifndef SWP_FRAMECHANGED
# define SWP_FRAMECHANGED  0x0020
#endif

DDSURFACEDESC displayModes[MAX_DISPLAY_MODES];
int displayModesCount = 0;

HRESULT WINAPI populateDisplayModesEnumCallback(LPDDSURFACEDESC lpDD,LPVOID lpContent) {
    (void)lpContent; // unused

    if (displayModesCount < MAX_DISPLAY_MODES)
        displayModes[displayModesCount++] = *lpDD;

    return DDENUMRET_OK;
}

void leave_exclusive_mode(void) {
    HRESULT hr;
    DWORD want;

    want = (DDSCL_ALLOWMODEX | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if ((ddrawCooperativeLevel & want) == want) {
        DWORD dowant = (ddrawCooperativeLevel & (~want)) | DDSCL_NORMAL;
        if ((hr=ddraw->SetCooperativeLevel(hwndMain,dowant)) == DD_OK) {
            fprintf(stderr,"Switched cooperative mode to normal\n");
            ddrawCooperativeLevel = dowant;
        }
        else {
            fprintf(stderr,"Failed to switch cooperative mode to normal\n");
        }
    }

    LONG style;
    if (ddrawCooperativeLevel & DDSCL_FULLSCREEN)
        style = hwndMainStyle & ~(WS_BORDER|WS_CAPTION|WS_DLGFRAME|WS_HSCROLL|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_SIZEBOX|WS_SYSMENU|WS_THICKFRAME);
    else 
        style = hwndMainStyle;

    SetWindowLong(hwndMain,GWL_STYLE,style);
    SetWindowPos(hwndMain,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);

    SetForegroundWindow(hwndMain);
    SetFocus(hwndMain);
    SetWindowPos(hwndMain,HWND_TOP,0,0,0,0,SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
    ShowWindow(hwndMain,SW_SHOWNORMAL);
    InvalidateRect(hwndMain,NULL,FALSE);
}

void try_exclusive_mode(void) {
    HRESULT hr;
    DWORD want;

    want = (DDSCL_ALLOWMODEX | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if ((ddrawCooperativeLevel & want) != want) {
        DWORD dowant = (ddrawCooperativeLevel & (~DDSCL_NORMAL)) | want;
        if ((hr=ddraw->SetCooperativeLevel(hwndMain,dowant)) == DD_OK) {
            fprintf(stderr,"Switched cooperative mode to exclusive/fullscreen\n");
            ddrawCooperativeLevel = dowant;
        }
        else {
            fprintf(stderr,"Failed to switch cooperative mode to exclusive\n");
        }
    }

    LONG style;
    if (ddrawCooperativeLevel & DDSCL_FULLSCREEN)
        style = hwndMainStyle & ~(WS_BORDER|WS_CAPTION|WS_DLGFRAME|WS_HSCROLL|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_SIZEBOX|WS_SYSMENU|WS_THICKFRAME);
    else 
        style = hwndMainStyle;

    SetWindowLong(hwndMain,GWL_STYLE,style);
    SetWindowPos(hwndMain,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);

    SetForegroundWindow(hwndMain);
    SetFocus(hwndMain);
    SetWindowPos(hwndMain,HWND_TOP,0,0,0,0,SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
    ShowWindow(hwndMain,SW_SHOWMAXIMIZED);
    InvalidateRect(hwndMain,NULL,FALSE);
}

void populateDisplayModes(void) {
    char tmp[128],*w,*wf;
    DDSURFACEDESC devmode;
    int index;

    if (menuDisplayModes != NULL) return;
    menuDisplayModes = CreatePopupMenu();
    if (menuDisplayModes == NULL) return;
    displayModesCount = 0;
    if (ddraw == NULL) return;

    AppendMenu(menuDisplayModes,MF_ENABLED|MF_STRING,5000,"Enter exclusive mode");
    AppendMenu(menuDisplayModes,MF_ENABLED|MF_STRING,5001,"Leave exclusive mode");
    AppendMenu(menuDisplayModes,MF_SEPARATOR,0,"");

    // NTS: Windows 95 OSR2 (with Anapa VESA drivers) will not enumerate any modes this way.
    //      But you can use GDI functions to make mode changes.
    ddraw->EnumDisplayModes(DDEDM_REFRESHRATES|DDEDM_STANDARDVGAMODES,NULL,NULL,populateDisplayModesEnumCallback);

    for (index=0;index < (int)displayModesCount;index++) {
        devmode = displayModes[index];

        w = tmp;
        wf = tmp + sizeof(tmp) - 1;

        w += snprintf(w,(size_t)(wf-w),"%ux%u",
                (unsigned int)devmode.dwWidth,
                (unsigned int)devmode.dwHeight);
        w += snprintf(w,(size_t)(wf-w)," x %ubpp",
                (unsigned int)devmode.ddpfPixelFormat.dwRGBBitCount);

        if (devmode.dwFlags & DDSD_REFRESHRATE) {
            w += snprintf(w,(size_t)(wf-w)," @%uHz",
                    (unsigned int)devmode.dwRefreshRate);
        }

        AppendMenu(menuDisplayModes,MF_ENABLED|MF_STRING,4000+index,tmp);
    }

    if (displayModesCount == 0) AppendMenu(menuDisplayModes,MF_DISABLED|MF_GRAYED|MF_STRING,0,"(none)");
}


static LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd,&ps);
            EndPaint(hwnd,&ps);
            update_screen();
            } break;
        case WM_DISPLAYCHANGE: {
            announce_fmt = true;

            unlock_bitmap();
            free_bitmap();
            free_dx_primary_surface();
            init_dx_primary_surface();

            RECT rct;
            GetClientRect(hwndMain,&rct);

            if (!init_bitmap(rct.right,rct.bottom))
                fprintf(stderr,"WARNING WM_RESIZE init_bitmap(%u,%u) failed\n",
                        (unsigned int)rct.right,(unsigned int)rct.bottom);

            if (lock_bitmap()) {
                render_test_pattern_rgb_gradients(dx_bitmap);
                unlock_bitmap();
            }

            InvalidateRect(hwndMain,NULL,FALSE); // DWM compositor-based versions set WM_PAINT such that only the affected area will repaint
            } return DefWindowProc(hwnd,uMsg,wParam,lParam);
        case WM_SIZE:
            if (!init_bitmap(LOWORD(lParam),HIWORD(lParam)))
                fprintf(stderr,"WARNING WM_RESIZE init_bitmap(%u,%u) failed\n",
                    LOWORD(lParam),HIWORD(lParam));

            if (lock_bitmap()) {
                render_test_pattern_rgb_gradients(dx_bitmap);
                unlock_bitmap();
            }

            InvalidateRect(hwndMain,NULL,FALSE); // DWM compositor-based versions set WM_PAINT such that only the affected area will repaint
            break;
        case WM_RBUTTONUP:
            populateDisplayModes();
            if (menuDisplayModes != NULL) {
                POINT pt;

                GetCursorPos(&pt);
                TrackPopupMenu(menuDisplayModes,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,0,hwnd,NULL);
            }
            break;
        case WM_COMMAND:
            if (wParam >= 4000 && wParam < (WPARAM)(4000+displayModesCount)) {
                DDSURFACEDESC devmode = displayModes[wParam-4000];
                HRESULT hr;

                fprintf(stderr,"Setting display to %u x %u x %ubpp\n",
                    (unsigned int)devmode.dwWidth,
                    (unsigned int)devmode.dwHeight,
                    (unsigned int)devmode.ddpfPixelFormat.dwRGBBitCount);
                if ((hr=ddraw->SetDisplayMode(devmode.dwWidth,devmode.dwHeight,devmode.ddpfPixelFormat.dwRGBBitCount)) != DD_OK)
                    fprintf(stderr,"Failed to set display mode HR=0x%08lx\n",hr);
                if (hr == DDERR_NOEXCLUSIVEMODE) {
                    fprintf(stderr,"Sorry, Windows demands we set the cooperative mode to Exclusive\n"); // Windows 9x/ME behavior for NORMAL cooperative mode DirectX apps
                    try_exclusive_mode();
                    if ((hr=ddraw->SetDisplayMode(devmode.dwWidth,devmode.dwHeight,devmode.ddpfPixelFormat.dwRGBBitCount)) != DD_OK)
                        fprintf(stderr,"Failed to set display mode HR=0x%08lx\n",hr);
                }

                if (hr == DD_OK) {
                    hasSetMode = true;
                    InvalidateRect(hwndMain,NULL,FALSE);
                    update_screen();
                }
            }
            else if (wParam == 5000) {
                if (hasSetMode) {
                    try_exclusive_mode();
                    update_screen();
                }
            }
            else if (wParam == 5001) {
                leave_exclusive_mode();
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
    HRESULT hr;
    RECT rect;
    MSG msg;
    char *a;
    int i;

    for (i=1;i < argc;) {
        a = argv[i++];

        if (*a == '-') {
            do { a++; } while (*a == '-');

            if (!strcmp(a,"nodpiaware")) {
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

    /* Bring up DirectX */
    /* NTS: On multi-monitor setups, this will create an "emulated" object. Hardware accel won't be possible unless we enum monitors. */
    if ((hr=DirectDrawCreate(NULL,&ddraw,NULL)) != DD_OK) {
        fprintf(stderr,"DirectDrawCreate failed, HRESULT=0x%08lx\n",hr);
        return 1;
    }
    if ((hr=ddraw->SetCooperativeLevel(hwndMain,ddrawCooperativeLevel=DDSCL_NORMAL)) != DD_OK) {
        fprintf(stderr,"Unable to ask for Normal cooperative mode, HR=0x%08lx\n",hr);
        return 1;
    }

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
        hwndMainStyle=WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,
        rect.right - rect.left,rect.bottom - rect.top,
        NULL,NULL,
        myInstance,NULL);
    if (!hwndMain) {
        fprintf(stderr,"CreateWindow() failed\n");
        return 1;
    }

    if (!init_dx_primary_surface()) {
        fprintf(stderr,"Failed to init primary surface\n");
        return 1;
    }

    GetClientRect(hwndMain,&rect);
    if (!init_bitmap(rect.right,rect.bottom)) {
        fprintf(stderr,"nit_bitmap() failed for %ux%u\n",(unsigned int)rect.right,(unsigned int)rect.bottom);
        return 1;
    }
    if (lock_bitmap()) {
        render_test_pattern_rgb_gradients(dx_bitmap);
        unlock_bitmap();
    }

    ShowWindow(hwndMain,SW_SHOW);
    UpdateWindow(hwndMain);

    while (GetMessage(&msg,NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    free_dx_primary_surface();
    free_bitmap();
    if (ddraw != NULL) {
        ddraw->RestoreDisplayMode();
        ddraw->Release();
        ddraw = NULL;
    }

    return 0;
}
