#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <unistd.h>
#endif

// X11 includes
#include <X11/Xlib.h>
#include <X11/Xatom.h>

void get_wm_name_x11() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Cannot open X display\n");
        return;
    }

    Atom supportingWmCheck = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", True);
    Atom netWmName = XInternAtom(dpy, "_NET_WM_NAME", True);
    Atom utf8String = XInternAtom(dpy, "UTF8_STRING", True);

    if (supportingWmCheck == None || netWmName == None) {
        fprintf(stderr, "EWMH atoms not found\n");
        XCloseDisplay(dpy);
        return;
    }

    Window root = DefaultRootWindow(dpy);
    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytesAfter;
    unsigned char *prop = NULL;
    Window wmWindow = None;

    if (XGetWindowProperty(dpy, root, supportingWmCheck, 0, (~0L), False,
                           XA_WINDOW, &actualType, &actualFormat,
                           &nitems, &bytesAfter, &prop) == Success) {
        if (prop && nitems > 0) {
            wmWindow = *(Window *)prop;
        }
        XFree(prop);
    }

    if (!wmWindow) {
        fprintf(stderr, "No WM found\n");
        XCloseDisplay(dpy);
        return;
    }

    unsigned char *name = NULL;
    if (XGetWindowProperty(dpy, wmWindow, netWmName, 0, (~0L), False,
                           utf8String != None ? utf8String : XA_STRING,
                           &actualType, &actualFormat, &nitems,
                           &bytesAfter, &name) == Success) {
        if (name) {
            printf("%s\n", (char *)name);
            XFree(name);
        } else {
            fprintf(stderr, "No WM name set\n");
        }
    }

    XCloseDisplay(dpy);
}

void get_wm_name_wayland() {
    const char *waylandDisplay = getenv("WAYLAND_DISPLAY");
    if (!waylandDisplay) {
        fprintf(stderr, "WAYLAND_DISPLAY not set\n");
        return;
    }

    // Try to detect known compositors
    const char *desktop = getenv("XDG_CURRENT_DESKTOP");
    const char *session = getenv("DESKTOP_SESSION");

    if (desktop) {
        printf("%s\n", desktop);
    } else if (session) {
        printf("%s\n", session);
    } else {
        printf("Wayland compositor (unknown)\n");
    }
}

int main(void) {
    const char *waylandDisplay = getenv("WAYLAND_DISPLAY");
    if (waylandDisplay) {
        get_wm_name_wayland();
    } else {
        get_wm_name_x11();
    }
    return 0;
}

