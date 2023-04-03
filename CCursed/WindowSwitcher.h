#pragma once

#include "WMenu.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>
#include <map>
#include <string.h>

class WindowSwitcher {
public:
  WindowSwitcher(Display *display, Window root);

  Display *display;
  Window root;

  Window switcherWindow;

  std::map<Window, Window> windowRefs;

  void AddWindow(Window window) ;

  void RemoveWindow(Window window) {
    XDestroyWindow(display, windowRefs[window]);
    windowRefs.erase(window);
    Update();
  }

  void Update();

  void OnButtonPress(XButtonEvent e);
};
