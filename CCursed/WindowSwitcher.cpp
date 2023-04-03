#include "WindowSwitcher.h"

WindowSwitcher::WindowSwitcher(Display *display, Window root) {
  this->windowRefs = std::map<Window, Window>();

  this->display = display;
  this->root = root;

  unsigned int root_width = 0;
  unsigned int root_height = 0;
  int root_x = 0;
  int root_y = 0;
  unsigned int root_border_width = 0;
  unsigned int root_depth = 0;

  XGetGeometry(display, root, &root, &root_x, &root_y, &root_width,
               &root_height, &root_border_width, &root_depth);

  this->switcherWindow = XCreateSimpleWindow(display, root, 0, 20, 100,
                                             root_height - 20, 1, 0, 0xC0C0C0);
  XMapWindow(display, switcherWindow);
  XFlush(display);

  Update();
}

void WindowSwitcher::AddWindow(Window window) {
  Window listentry =
      XCreateSimpleWindow(display, switcherWindow, 0, windowRefs.size() * 20,
                          100, 20, 1, 0, 0xFFFFFF);
  XMapWindow(display, listentry);
  XFlush(display);
  windowRefs.insert(std::make_pair(window, listentry));

  XGrabButton(display, Button1, AnyModifier, listentry, false, ButtonPressMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
  XGrabButton(display, Button3, AnyModifier, listentry, false, ButtonPressMask,
              GrabModeAsync, GrabModeAsync, 0, 0);

  Update();
}

void WindowSwitcher::Update() {
  XSetWindowBackground(display, switcherWindow, 0xFFFFFF);

  XFlush(display);

  for (size_t i = 0; i < windowRefs.size(); i++) {
    auto window = windowRefs.begin();
    std::advance(window, i);
    Window listentry = window->second;
    XSetWindowBackground(display, listentry, 0xFFFFFF);
    XFlush(display);

    char *windowName = nullptr;
    XFetchName(display, window->first, &windowName);
    if (windowName) {
      XDrawString(display, listentry, XDefaultGC(display, 0), 0, 10, windowName,
                  strlen(windowName));
      XFree(windowName);
    } else {
      RemoveWindow(window->first);
      return;
    }

    XFlush(display);

    XMoveWindow(display, listentry, 0, i * 20);

    XFlush(display);
  }
}

void WindowSwitcher::OnButtonPress(XButtonEvent e) {
  if (e.button == Button1) {
    for (const auto &window : windowRefs) {
      if (window.second == e.window) {
        XSetInputFocus(display, window.first, RevertToNone, e.time);
        XRaiseWindow(display, window.first);
        XFlush(display);
      }
    }
  } else if (e.button == Button3) {
    for (const auto &window : windowRefs) {
      if (window.second == e.window) {
        WMenu wmenu;
        wmenu.AddItem("Close", [&]() {
          XDestroyWindow(display, window.first);
          XFlush(display);
          RemoveWindow(window.first);
        });

        wmenu.Show(display, root);
      }
    }
  }
}
