
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

IDirectDraw*                    ddraw = NULL;
DWORD                           ddrawCooperativeLevel = 0;

IDirectDrawClipper*             ddclipperPrimary = NULL;
IDirectDrawSurface*             ddsurfacePrimary = NULL;
IDirectDrawSurface*             ddsurfaceGDI = NULL;    // Microsoft is very unclear about this: You can only get the GDI surface in fullscreen/exclusive. BUT, you can create a primary surface of the screen!
IDirectDrawSurface*             ddsurfaceBMP = NULL;

HWND                            hwndMain = NULL;
const char*                     hwndMainClass = "WINGDICLASS";
const char*                     hwndMainTitle = "WinGDI test 1";
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

int init_bitmap(unsigned int w,unsigned int h,unsigned int align=32) {
    DDSURFACEDESC ddsurf;
    HRESULT hr;

    free_bitmap();
    if (w == 0 || h == 0) return 0;

    memset(&ddsurf,0,sizeof(ddsurf));
    ddsurf.dwSize = sizeof(ddsurf);
    ddsurf.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsurf.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsurf.dwWidth = w;
    ddsurf.dwHeight = h;

    if ((hr=ddraw->CreateSurface(&ddsurf,&ddsurfaceBMP,NULL)) != DD_OK) {
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
    dx_bitmap.width = ddsurf.dwWidth;
    dx_bitmap.height = ddsurf.dwHeight;
    dx_bitmap.bytes_per_pixel = (ddsurf.ddpfPixelFormat.dwRGBBitCount + 7) / 8;
    dx_bitmap.stride = ddsurf.lPitch;
    dx_bitmap.update_length_from_stride_and_height();

    dx_bitmap.rgbinfo.r.setByMask(ddsurf.ddpfPixelFormat.dwRBitMask);
    dx_bitmap.rgbinfo.g.setByMask(ddsurf.ddpfPixelFormat.dwGBitMask);
    dx_bitmap.rgbinfo.b.setByMask(ddsurf.ddpfPixelFormat.dwBBitMask);
    dx_bitmap.rgbinfo.a.setByMask(ddsurf.ddpfPixelFormat.dwRGBAlphaBitMask);

    // the catch with DirectX is that we cannot keep the pointer in the dx bitmap at all times,
    // because of the need to lock the surface. be aware of that. Until you lock the surface,
    // our dx_bitmap is technically invalid. I'm not going to just leave it locked because we
    // want DirectX to be able to Blit from our surface to the screen.
    dx_bitmap.base = dx_bitmap.canvas = NULL;

    return 1;
}

void update_screen() {
    RECT dstRect;
    HRESULT hr;
    POINT pt;

    // NTS: dx_bitmap.is_valid() is not a valid test because base==canvas==NULL unless surface is locked
    if (ddsurfaceBMP == NULL) return;

    pt.x = pt.y = 0;
    ClientToScreen(hwndMain,&pt);
    GetClientRect(hwndMain,&dstRect);
    OffsetRect(&dstRect,pt.x,pt.y);

    if (ddsurfaceGDI != NULL)
        hr = ddsurfaceGDI->Blt(&dstRect,ddsurfaceBMP,NULL,0,NULL);
    else if (ddsurfacePrimary != NULL)
        hr = ddsurfacePrimary->Blt(&dstRect,ddsurfaceBMP,NULL,0,NULL);
    else {
        hr = DD_OK;
        fprintf(stderr,"Neither primary nor surface ready\n");
    }

    if (hr != DD_OK)
        fprintf(stderr,"DirectX blit failed, HR=0x%08lx\n",hr);
}

static LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd,&ps);
            update_screen();
            EndPaint(hwnd,&ps);
            } break;
        case WM_DISPLAYCHANGE:
            announce_fmt = true;
            if (dx_bitmap.width == 0 || dx_bitmap.height == 0) {
                RECT rct;
                GetClientRect(hwnd,&rct);
                dx_bitmap.width = rct.right;
                dx_bitmap.height = rct.bottom;
            }
            if (!init_bitmap(dx_bitmap.width,dx_bitmap.height))
                fprintf(stderr,"WARNING WM_RESIZE init_bitmap(%u,%u) failed\n",
                    dx_bitmap.width,dx_bitmap.height);

            if (lock_bitmap()) {
                render_test_pattern_rgb_gradients(dx_bitmap);
                unlock_bitmap();
            }

            InvalidateRect(hwndMain,NULL,FALSE); // DWM compositor-based versions set WM_PAINT such that only the affected area will repaint
            return DefWindowProc(hwnd,uMsg,wParam,lParam);
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
        case WM_COMMAND:
            return DefWindowProc(hwnd,uMsg,wParam,lParam);
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
            return 1;
        }
    }

    /* Okay, now try to get GDI surface.
       I don't expect this to work. Feh. */
    if ((hr=ddraw->GetGDISurface(&ddsurfaceGDI)) != DD_OK)
        fprintf(stderr,"Oh, gee, couldn't get the GDI surface (HR=0x%08lx), oh golly what a surprise (not)\n",hr);

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
    if (lock_bitmap()) {
        render_test_pattern_rgb_gradients(dx_bitmap);
        unlock_bitmap();
    }

    if ((hr=ddraw->CreateClipper(0,&ddclipperPrimary,NULL)) != DD_OK) {
        fprintf(stderr,"Failed to create clipper, HR=0x%08lx\n",hr);
        return 1;
    }
    if ((hr=ddclipperPrimary->SetHWnd(0,hwndMain)) != DD_OK) {
        fprintf(stderr,"Failed to set clipper to main window, HR=0x%08lx\n",hr);
        return 1;
    }
    if ((hr=ddsurfacePrimary->SetClipper(ddclipperPrimary)) != DD_OK) {
        fprintf(stderr,"Failed to set clipper to primary surface, HR=0x%08lx\n",hr);
        return 1;
    }

    ShowWindow(hwndMain,SW_SHOW);
    UpdateWindow(hwndMain);

    while (GetMessage(&msg,NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (ddsurfacePrimary != NULL) {
        ddsurfacePrimary->Release();
        ddsurfacePrimary = NULL;
    }
    if (ddsurfaceGDI != NULL) {
        ddsurfaceGDI->Release();
        ddsurfaceGDI = NULL;
    }
    if (ddsurfaceBMP != NULL) {
        ddsurfaceBMP->Release();
        ddsurfaceBMP = NULL;
    }
    if (ddclipperPrimary != NULL) {
        ddclipperPrimary->Release();
        ddclipperPrimary = NULL;
    }
    if (ddraw != NULL) {
        ddraw->Release();
        ddraw = NULL;
    }

    return 0;
}
