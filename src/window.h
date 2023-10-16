#pragma once

#include <X11/Xlib.h>
#include <optional>

namespace gp::x_window {

struct XWindowData {
  Display *display;
  Window window;
  int screen;
  XEvent event;
};

std::optional<XWindowData> open();
void event_loop(XWindowData &window_data);
void close(XWindowData &window_data);

} // namespace gp::x_window
