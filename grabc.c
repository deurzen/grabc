#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <stddef.h>
#include <getopt.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>


static Window selectWindow(Display*, int*, int*);
static Window findSubWindow(Display*, Window, Window, int*, int*);
static int getWindowColor(Display*, XColor*);
static int MXError(Display*, XErrorEvent*);
static void die(char*);

int
main(int argc, char* *argv)
{
    Display* dpy;
    int status;
    XColor color;
    Colormap cmap;
    int r, g, b;
    int html = 1;
    int pound = 0;

    int opt;
    while ((opt = getopt(argc, argv, "vhd")) != -1) {
        switch (opt) {
            case 'h': html = 1;  break;
            case 'd': html = 0;  break;
            case '#': pound = 1; break;
            case 'n': pound = 0; break;
            case 'v':
            {
                printf("grabc v1.0: [-vhd]\n");
                exit(0);
            }
            default: fprintf(stderr, "invalid flag: %c\n", opt); break;
        }
    }

    dpy = XOpenDisplay(NULL);
    cmap = DefaultColormap(dpy, DefaultScreen(dpy));

    XSetErrorHandler(MXError);

    if (!dpy)
        die("failed to open dpy\n");

    status = getWindowColor(dpy, &color);
    if (status == 1) {
        XQueryColor(dpy, cmap, &color);
        r = color.red   >> 8;
        g = color.green >> 8;
        b = color.blue  >> 8;

        if (html) {
            if (pound)
                printf("#%02x%02x%02x", r, g, b);
            else
                printf("%02x%02x%02x", r, g, b);
        } else
            printf("%d,%d,%d", r, g, b);

        fflush(stdout);
    } else
        die("failed grabbing color\n");

    return 0;
}

static Window
selectWindow(Display* dpy, int* x, int* y)
{
    Cursor target_cursor;
    static Cursor cross_cursor = None;
    int status;
    Window target_win, root_win;
    XEvent event;
    target_win = None;

    if (!cross_cursor) {
        cross_cursor = XCreateFontCursor(dpy, XC_tcross);

        if (!cross_cursor)
            return None;
    }

    target_cursor = cross_cursor;
    root_win = XRootWindow(dpy, XDefaultScreen(dpy));
    status = XGrabPointer(dpy, root_win, 0,
        (unsigned)ButtonPressMask, GrabModeSync,
        GrabModeAsync, root_win, target_cursor, CurrentTime);

    if (status == GrabSuccess) {
        XAllowEvents(dpy, SyncPointer, CurrentTime);
        XWindowEvent(dpy, root_win, ButtonPressMask,&event);

        if (event.type == ButtonPress) {
            target_win = findSubWindow(dpy, root_win,
                event.xbutton.subwindow, &event.xbutton.x, &event.xbutton.y);

            if (!target_win) {
                fprintf(stderr, "failed getting target win, targeting root\n");
                target_win = root_win;
            }
            XUngrabPointer(dpy, CurrentTime);
        }
    } else
        die("failed grabbing mouse\n");

    XFreeCursor(dpy, cross_cursor);

    *x = event.xbutton.x;
    *y = event.xbutton.y;

    return target_win;
}

static Window
findSubWindow(Display* dpy, Window top_win, Window win_to_check, int* x, int* y)
{
    int newx, newy;
    Window win;

    if (!top_win)
        return None;

    if (!win_to_check)
        return None;

    win = win_to_check;

    while ((XTranslateCoordinates(dpy, top_win, win_to_check, *x, *y, &newx, &newy, &win) != 0) && !win)
        if (win) {
            top_win = win_to_check;
            win_to_check = win;
            *x = newx;
            *y = newy;
        }

    if (!win)
        win = win_to_check;

    *x = newx;
    *y = newy;

    return win;
}

static int
getWindowColor(Display* dpy, XColor* color)
{
    Window target_win;
    XImage *ximage;
    int x, y;

    target_win = selectWindow(dpy,&x,&y);

    if (!target_win)
        return 0;

    ximage = XGetImage(dpy, target_win, x, y, 1, 1, AllPlanes, ZPixmap);
    if (!ximage)
        return 0;

    color->pixel = XGetPixel(ximage, 0, 0);
    XDestroyImage(ximage);

    return 1;
}

static int
MXError(Display* dpy, XErrorEvent* error)
{
    int xerrcode;
    xerrcode = error->error_code;

    if (xerrcode == BadAlloc || (xerrcode == BadAccess && error->request_code == 88))
        return 0;

    switch (error->request_code) {
    case X_GetGeometry:
        {
            if (error->error_code == BadDrawable)
                return 0;

            break;
        }
    case X_GetWindowAttributes:
    case X_QueryTree:
        {
            if (error->error_code == BadWindow)
                return 0;

            break;
        }
    case X_QueryColors:
        {
            if (error->error_code == BadValue)
                return 0;

            break;
        }
    }

    return 0;
}

static void
die(char* msg)
{
    fprintf(stderr, "%s", msg);
    exit(1);
}
