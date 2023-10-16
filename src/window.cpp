#include "window.h"

namespace gp::x_window {

std::optional<XWindowData> open() {
  Display *display;
  Window window;
  int screen;
  XEvent event;

  display = XOpenDisplay(NULL);
  if (display == NULL)
    return {};

  screen = DefaultScreen(display);
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10,
                               100, 100, 1, BlackPixel(display, screen),
                               WhitePixel(display, screen));

  XSelectInput(display, window, ExposureMask);
  XMapWindow(display, window);

  return XWindowData{display, window, screen, event};
}

void event_loop(XWindowData &window_data) {
  Atom wm_kill_msg =
      XInternAtom(window_data.display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(window_data.display, window_data.window, &wm_kill_msg, 1);

  bool running = true;
  while (running) {
    XNextEvent(window_data.display, &window_data.event);

    switch (window_data.event.type) {
    case Expose:
      XFillRectangle(window_data.display, window_data.window,
                     DefaultGC(window_data.display, window_data.screen), 20, 20,
                     10, 10);
      break;

    case ClientMessage:
      if (window_data.event.xclient.data.l[0] == wm_kill_msg)
        running = false;
      break;

    default:
      break;
    }
  }

  close(window_data);
}

void close(XWindowData &window_data) { XCloseDisplay(window_data.display); }

} // namespace gp::x_window
