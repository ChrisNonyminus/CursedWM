#include "WMenu.h"

void WMenu::Show(Display *display, Window root) {
  unsigned int menuWidth = 100;
  unsigned int menuHeight = static_cast<unsigned int>(Items.size() * 20);
  int cursorX = 0;
  int cursorY = 0;
  unsigned int mask = 0;
  Window root_return;
  Window child_return;
  int win_x = 0;
  int win_y = 0;
  XQueryPointer(display, root, &root_return, &child_return, &cursorX, &cursorY,
                &win_x, &win_y, &mask);
  Window menu = XCreateSimpleWindow(display, root_return, cursorX,
                                    cursorY + static_cast<int>(menuHeight / 2),
                                    menuWidth, menuHeight, 0, 0, 0);
  XMapWindow(display, menu);
  XFlush(display);
  int menuX = 0;
  int menuY = 0;
  unsigned int menuBorderWidth = 0;
  unsigned int menuDepth = 0;
  XGetGeometry(display, menu, &root, &menuX, &menuY, &menuWidth, &menuHeight,
               &menuBorderWidth, &menuDepth);
  XGCValues menuGCValues;
  GC menuGC = XCreateGC(display, menu, 0, &menuGCValues);

  std::vector<Window> itemWindows;

  for (size_t i = 0; i < Items.size(); i++) {
    IMenuItem* item = Items[i];
    unsigned int itemWidth = menuWidth;
    unsigned int itemHeight =
        static_cast<unsigned int>(menuHeight / Items.size());
    int itemX = 0;
    int itemY = static_cast<int>(i * itemHeight);
    Window itemWindow = XCreateSimpleWindow(
        display, menu, itemX, itemY, itemWidth, itemHeight, 0, 0, 0xFFFFFF);
    XMapWindow(display, itemWindow);
    XFlush(display);
    XDrawString(display, itemWindow, menuGC, 0,
                static_cast<int>(itemHeight - Items.size()), item->GetName().c_str(),
                item->GetName().length());
    XFlush(display);
    XSelectInput(display, itemWindow, ButtonPressMask);
    XFlush(display);

    itemWindows.push_back(itemWindow);
  }

  while (true) {
    XEvent ev;
    XNextEvent(display, &ev);
    if (ev.type == ButtonPress) {
      XButtonEvent buttonEvent = ev.xbutton;
      if (std::find(itemWindows.begin(), itemWindows.end(),
                    buttonEvent.window) == itemWindows.end())
        break;
      size_t itemIndex =
          static_cast<size_t>(buttonEvent.y / (menuHeight / Items.size()));
      IMenuItem* item = Items[itemIndex];
      item->OnClick();
      break;
    }
  }

  XDestroyWindow(display, menu);
  XFlush(display);
}
