#include <X11/Xlib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main() {
  Display *display;
  Window window;
  XEvent event;
  const char *msg = "Hello, World!";
  int s;

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    exit(EXIT_FAILURE);
  }

  s = DefaultScreen(display);

  window =
      XCreateSimpleWindow(display, RootWindow(display, s), 10, 10, 100, 100, 1,
                          BlackPixel(display, s), WhitePixel(display, s));

  XSelectInput(display, window, ExposureMask | KeyPressMask);
  XMapWindow(display, window);

  Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteMessage, 1);

  bool running = true;
  while (running) {
    XNextEvent(display, &event);

    switch (event.type) {
    case Expose:
      XFillRectangle(display, window, DefaultGC(display, s), 20, 20, 10, 10);
      XDrawString(display, window, DefaultGC(display, s), 50, 50, msg,
                  strlen(msg));
      break;

    case ClientMessage:
      if (event.xclient.data.l[0] == wmDeleteMessage)
        running = false;
      break;

    default:
      break;
    }
  }

  XCloseDisplay(display);

  return EXIT_SUCCESS;
}
