// -- Written in C -- //

#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "dosboxxr/lib/hostcpudetect/caps.h"
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_patterns.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"
#include "dosboxxr/lib/graphics/stretchblt_general.h"

rgb_bitmap_info         src_bitmap;
bool                    resize_src_mode = false;

Display                 *dpy;
Window                  root;
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
XWindowAttributes       gwa;
XEvent                  xev;

bool                    linear_blur = true;

GLuint                  src_bmp_texture = GL_INVALID_VALUE;
bool                    src_bmp_texture_init = false;

static void free_src_texture(void) {
    if (src_bmp_texture != GL_INVALID_VALUE) {
        glDeleteTextures(1,&src_bmp_texture);
        src_bmp_texture = GL_INVALID_VALUE;
    }

    src_bmp_texture_init = false;
}

static int init_src_texture(void) {
    if (!src_bmp_texture_init) {
        free_src_texture();
        if (src_bitmap.base == NULL)
            return 0;

        glGenTextures(1,&src_bmp_texture);
        if (src_bmp_texture == GL_INVALID_VALUE) {
            fprintf(stderr,"glGenTextures() failed\n");
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, src_bmp_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, src_bitmap.stride/src_bitmap.bytes_per_pixel, src_bitmap.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, src_bitmap.canvas);

        src_bmp_texture_init = true;
    }

    return 1;
}

static void free_src_bitmap(void);

static int init_src_bitmap(unsigned int width=640,unsigned int height=480) {
    if (src_bitmap.base != NULL)
        return 0;

    src_bitmap.clear();
    src_bitmap.base_align = 32;
    src_bitmap.stride_align = 32;
    src_bitmap.rgbinfo.r.setByMask(0x00FF0000UL);
    src_bitmap.rgbinfo.g.setByMask(0x0000FF00UL);
    src_bitmap.rgbinfo.b.setByMask(0x000000FFUL);
    src_bitmap.rgbinfo.a.setByMask(0xFF000000UL);
    src_bitmap.bytes_per_pixel = 4;
    src_bitmap.width = width;
    src_bitmap.height = height;
    src_bitmap.update_stride_from_width();
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

void DrawAQuad() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1., 1., -1., 1., 1., 20.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,src_bmp_texture);

    if (linear_blur) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    /* NTS: We can't always give right edge texture coordinate 1.0 because OpenGL doesn't understand our bitmap
     *      stride vs width difference. So compute the right edge texture coordinate now. */
    double tx = (double)src_bitmap.width / (src_bitmap.stride / src_bitmap.bytes_per_pixel);

    glBegin(GL_QUADS);
    glTexCoord2f(0., 1.);
    glVertex3f(-1., -1., 0.);
    glTexCoord2f(tx, 1.);
    glVertex3f( 1., -1., 0.);
    glTexCoord2f(tx, 0.);
    glVertex3f( 1.,  1., 0.);
    glTexCoord2f(0., 0.);
    glVertex3f(-1.,  1., 0.);
    glEnd();
}

int main() {
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        printf("\n\tcannot connect to X server\n\n");
        exit(0);
    }

    root = DefaultRootWindow(dpy);

    vi = glXChooseVisual(dpy, 0, att);

    if (vi == NULL) {
        printf("\n\tno appropriate visual found\n\n");
        exit(0);
    }
    else {
        printf("\n\tvisual %p selected\n", (void *)vi->visualid); /* %p creates hexadecimal output like in glxinfo */
    }

    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;

    win = XCreateWindow(dpy, root, 0, 0, 640, 480, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

    XMapWindow(dpy, win);
    XStoreName(dpy, win, "VERY SIMPLE APPLICATION");

    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);

    glEnable(GL_DEPTH_TEST);

    hostCPUcaps.detect();

    if (!init_src_bitmap(640,480))
        return 1;

    test_pattern_1_render(/*&*/src_bitmap);
    init_src_texture();

    while (1) {
        XNextEvent(dpy, &xev);

        if (xev.type == Expose) {
            XGetWindowAttributes(dpy, win, &gwa);
            glViewport(0, 0, gwa.width, gwa.height);

            if (resize_src_mode) {
                free_src_texture();
                free_src_bitmap();
                init_src_bitmap(gwa.width,gwa.height);
                test_pattern_1_render(/*&*/src_bitmap);
                init_src_texture();
            }

            DrawAQuad();
            glXSwapBuffers(dpy, win);
        }
        else if (xev.type == KeyPress) {
            char buffer[80];
            KeySym sym=0;

            XLookupString(&xev.xkey, buffer, sizeof(buffer), &sym, NULL);

            if (sym == XK_s) {
                resize_src_mode = !resize_src_mode;
                if (resize_src_mode) {
                    XGetWindowAttributes(dpy, win, &gwa);
                    glViewport(0, 0, gwa.width, gwa.height);
                    free_src_texture();
                    free_src_bitmap();
                    init_src_bitmap(gwa.width,gwa.height);
                    test_pattern_1_render(/*&*/src_bitmap);
                    init_src_texture();
                    DrawAQuad();
                    glXSwapBuffers(dpy, win);
                }
            }
            else if (sym == XK_space) {
                linear_blur = !linear_blur;
                DrawAQuad();
                glXSwapBuffers(dpy, win);
            }
            else if (sym == XK_Escape) {
                break;
            }
        }
    }

    free_src_texture();
    free_src_bitmap();

    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, glc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

