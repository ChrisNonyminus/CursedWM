#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

class IMenuItem {
public:
  virtual void OnClick() = 0;
  virtual std::string GetName() = 0;
};

template<typename F>
class WMenuItem : public IMenuItem {
public:
  WMenuItem(std::string name, F&& action)
      : Name(name), Action(action), Params(params) {}

  std::string Name;
  F Action;

  void OnClick() override {
    Action();
  }

  std::string GetName() override {
    return Name;
  }

};

class WMenu {
public:
  WMenu() {}

  ~WMenu() {
    for (auto item : Items) {
      delete item;
    }
  }

  std::vector<IMenuItem*> Items;

  inline void AddItem(IMenuItem* item) { Items.push_back(item); }

  template <typename F>
  inline void AddItem(std::string name,
                      F&& action) {
    Items.push_back(new WMenuItem<F>(name, action));
  }

  void Show(Display *display, Window root);
};
