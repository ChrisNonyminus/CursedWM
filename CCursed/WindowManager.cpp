#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef enum {
  LOG_Debug,
  LOG_Info,
  LOG_Warn,
  LOG_Error,
} LogLevel;

typedef struct SimpleLogger {
  LogLevel level;

  void (*Debug)(struct SimpleLogger *logger,  char *message);
  void (*Info)(struct SimpleLogger *logger,  char *message);
  void (*Warn)(struct SimpleLogger *logger,  char *message);
  void (*Error)(struct SimpleLogger *logger,  char *message);
} SimpleLogger;

char *FormatString(char *format, ...) {
  va_list args;
  va_start(args, format);
  int len = vsnprintf(NULL, 0, format, args);
  va_end(args);

  char *str = (char*)malloc(len + 1);
  va_start(args, format);
  vsnprintf(str, len + 1, format, args);
  va_end(args);

  return str;
}

/* Message strings must NOT be const! */
void SimpleLogger_Write(SimpleLogger *logger, char *message,
                        LogLevel message_level) {
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80];
  if (logger->level <= message_level) {

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%I:%M:%S %p", timeinfo);
    printf("[%s][%d]: %s\n", buffer, message_level, message);
  }
  free(message);
}

void SimpleLogger_Debug(SimpleLogger *logger, char *message) {
  SimpleLogger_Write(logger, message, LOG_Debug);
}

void SimpleLogger_Info(SimpleLogger *logger, char *message) {
  SimpleLogger_Write(logger, message, LOG_Info);
}

void SimpleLogger_Warn(SimpleLogger *logger, char *message) {
  SimpleLogger_Write(logger, message, LOG_Warn);
}

void SimpleLogger_Error(SimpleLogger *logger, char *message) {
  SimpleLogger_Write(logger, message, LOG_Error);
}
void SimpleLogger_init(SimpleLogger *logger, LogLevel level) {
  logger->level = level;
  logger->Debug = &SimpleLogger_Debug;
  logger->Info = &SimpleLogger_Info;
  logger->Warn = &SimpleLogger_Warn;
  logger->Error = &SimpleLogger_Error;
}

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct WindowGroup {
  Window child;
  Window frame;
  Window title;
  Window menubutton;
} WindowGroup;

typedef struct WMCursors {
  Cursor DefaultCursor;
  Cursor FrameCursor;
  Cursor TitleCursor;
} WMCursors;

typedef struct WMColours {
  unsigned long ActiveFrameColor;
  unsigned long ActiveTitleColor;
  unsigned long ActiveTitleBorder;
  unsigned long InactiveFrameColor;
  unsigned long InactiveTitleColor;
  unsigned long InactiveTitleBorder;
  unsigned long DesktopBackground;
  unsigned long WindowBackground;
} WMColours;
typedef struct {
  int frame_width;
  int title_height;
  int inner_border;
  WMColours Colours;
  WMCursors Cursors;
} WMConfig;

WMConfig gConfig = {.frame_width = 3, .title_height = 20, .inner_border = 1};
typedef enum MouseMoveType {
  MouseMoveType_TitleDrag,
  MouseMoveType_TopLeftFrameDrag,
  MouseMoveType_TopRightFrameDrag,
  MouseMoveType_BottomLeftFrameDrag,
  MouseMoveType_BottomRightFrameDrag,
  MouseMoveType_RightFrameDrag,
  MouseMoveType_TopFrameDrag,
  MouseMoveType_LeftFrameDrag,
  MouseMoveType_BottomFrameDrag,
} MouseMoveType;

typedef struct MouseMovement {
  MouseMoveType Type;
  int MotionStartX;
  int MotionStartY;
  int WindowOriginPointX;
  int WindowOriginPointY;
} MouseMovement;

void MouseMovement_init(MouseMovement *self, MouseMoveType type, int Motion_X,
                        int Motion_Y, int Window_X, int Window_Y) {
  self->Type = type;
  self->MotionStartX = Motion_X;
  self->MotionStartY = Motion_Y;
  self->WindowOriginPointX = Window_X;
  self->WindowOriginPointY = Window_Y;
}

typedef int (*XErrorHandlerDelegate)(Display *display, XErrorEvent *ev);

typedef struct dict_entry_s {
  struct dict_entry_s *next;
  union {
    Window wind;
    void *vp;
  } key;
  void *val;
} DictEntry;

typedef struct {
  size_t len;
  DictEntry *head;
} Dict;

Dict *Dict_init() {
  Dict *dict = (Dict *)malloc(sizeof(Dict));
  dict->len = 0;
  dict->head = (DictEntry *)malloc(sizeof(DictEntry));

  return dict;
}

int Dict_has_vp(Dict *dict, void *key) {
  DictEntry *ref;
  ref = dict->head;

  do {
    if (ref->key.vp == key) {
      return 1;
    }
  } while (ref->next != NULL && (ref = ref->next));

  return 0;
}

int Dict_add(Dict *dict, void *key, void *value) {
  DictEntry *ref = dict->head;

  if (dict->len == 0) {
    dict->len++;
    ref->key.vp = key;
    ref->val = value;

    return 1;
  }

  do {
    if (ref->key.vp == key) {
      return 0;
    }
  } while (ref->next && (ref = ref->next));

  dict->len++;
  ref->next = (DictEntry *)malloc(sizeof(DictEntry));
  ref->next->key.vp = key;
  ref->next->val = value;

  return 1;
}

DictEntry *Dict_entry(Dict *dict, void *key) {
  DictEntry *ref = dict->head;

  do {
    if (ref->key.vp == key) {
      return ref;
    }
  } while (ref->next && (ref = ref->next));

  return 0;
}

void *Dict_get(Dict *dict, void *key) {
  DictEntry *ref = Dict_entry(dict, key);
  return ref->val;
}

void Dict_remove(Dict *dict, void *key) {
  DictEntry *ref = dict->head;
  DictEntry *prev = NULL;
  void* val = NULL;

  do {
    if (ref->key.vp == key) {
      if (prev == NULL) {
        dict->head = ref->next;
      } else {
        prev->next = ref->next;
      }
      free(ref);
      dict->len--;
      return;
    }
    prev = ref;
  } while (ref->next && (ref = ref->next));
}


#include "WMenu.h"
#include "WindowSwitcher.h"



typedef struct {
  SimpleLogger Log;
  XErrorHandlerDelegate OnError;
  Display *display;
  Window root;
  std::map<Window, WindowGroup> WindowGroupsByClient;
  std::map<Window, WindowGroup> WindowGroupsByTitle;
  std::map<Window, WindowGroup> WindowGroupsByFrame;
  std::map<Window, WindowGroup> WindowGroupsByMenuButt;
  WindowSwitcher Switcher;
  MouseMovement mouseMovement;
} WindowManager;

WindowManager *gWindowManager = NULL;

int WMErrorHandler(Display *display, XErrorEvent *ev) {
  WindowManager *wm = gWindowManager;
  char description[1024];

  if (ev->error_code == BadAccess) {
    wm->Log.Error(
        &wm->Log,
        FormatString("X11 denied access to window manager resources - "
                     "another window manager is already running"));
    exit(1);
  }

  XGetErrorText(display, ev->error_code, description, sizeof(description));
  wm->Log.Warn(&wm->Log, description);
  return 0;
}

unsigned long GetPixelByName(const char *color_name) {
  XColor color;
  Colormap colormap;
  Display *display = XOpenDisplay(NULL);
  colormap = DefaultColormap(display, 0);
  XParseColor(display, colormap, color_name, &color);
  XAllocColor(display, colormap, &color);
  XCloseDisplay(display);
  return color.pixel;
}

WindowManager *WindowManager_init() {
  WindowManager *wm = (WindowManager *)malloc(sizeof(WindowManager));
  wm->display = XOpenDisplay(NULL);

  SimpleLogger_init(&wm->Log, LOG_Debug);

  if (wm->display == NULL) {
    wm->Log.Error(&wm->Log,
                  FormatString("Unable to open the default X display\n"));
    exit(1);
  }

  wm->root = DefaultRootWindow(wm->display);
  XSetErrorHandler(WMErrorHandler);

  XSelectInput(wm->display, wm->root,
               SubstructureRedirectMask | SubstructureNotifyMask |
                   ButtonPressMask | KeyPressMask);

  XSync(wm->display, False);

  // Setup cursors
  gConfig.Cursors.DefaultCursor = XCreateFontCursor(wm->display, XC_left_ptr);
  gConfig.Cursors.TitleCursor = XCreateFontCursor(wm->display, XC_fleur);
  gConfig.Cursors.FrameCursor = XCreateFontCursor(wm->display, XC_sizing);
  XDefineCursor(wm->display, wm->root, gConfig.Cursors.DefaultCursor);

  // Setup colours
  gConfig.Colours.DesktopBackground = GetPixelByName("grey");
  gConfig.Colours.WindowBackground = GetPixelByName("white");
  gConfig.Colours.InactiveTitleBorder = GetPixelByName("light slate grey");
  gConfig.Colours.InactiveTitleColor = GetPixelByName("slate grey");
  gConfig.Colours.InactiveFrameColor = GetPixelByName("dark slate grey");
  gConfig.Colours.ActiveFrameColor = GetPixelByName("dark goldenrod");
  gConfig.Colours.ActiveTitleColor = GetPixelByName("gold");
  gConfig.Colours.ActiveTitleBorder = GetPixelByName("saddle brown");

  XSetWindowBackground(wm->display, wm->root,
                       gConfig.Colours.DesktopBackground);
  XClearWindow(wm->display, wm->root);

  return wm;
}



void WindowManager_AddFrame(WindowManager *wm, Window child) {
  XWindowAttributes attr;
  Window title;
  Window frame;
  Window menubutton;
  int adjusted_x_loc, adjusted_y_loc;
  char *Name = NULL;
  WindowGroup* wg = (WindowGroup*)malloc(sizeof(WindowGroup));

  if (wm->WindowGroupsByClient.count(child) > 0) {
    return;
  }

  XFetchName(wm->display, child, &Name);
  wm->Log.Debug(&wm->Log, FormatString("Framing &s\n", Name));

  XGetWindowAttributes(wm->display, child, &attr);
  title = XCreateSimpleWindow(
      wm->display, wm->root, attr.x, attr.y,
      attr.width - (2 * gConfig.inner_border),
      (gConfig.title_height - 2 * gConfig.inner_border), gConfig.inner_border,
      gConfig.Colours.InactiveTitleColor, gConfig.Colours.InactiveTitleBorder);
  adjusted_x_loc =
      (attr.x - gConfig.frame_width < 0) ? 0 : attr.x - gConfig.frame_width;
  adjusted_y_loc =
      (attr.y - (gConfig.title_height + gConfig.frame_width) < 0)
          ? 0
          : (attr.y - (gConfig.title_height + gConfig.frame_width));
  frame = XCreateSimpleWindow(wm->display, wm->root, adjusted_x_loc,
                                       adjusted_y_loc, attr.width, attr.height + gConfig.title_height,
                                       3, gConfig.Colours.InactiveFrameColor, gConfig.Colours.WindowBackground);
  menubutton = XCreateSimpleWindow(wm->display, wm->root,
                                            (int)(attr.width - (gConfig.title_height)), attr.y, gConfig.title_height, gConfig.title_height, gConfig.inner_border, gConfig.Colours.InactiveTitleColor, gConfig.Colours.WindowBackground);
  XSelectInput(wm->display, title, ButtonPressMask | ButtonReleaseMask
                 | Button1MotionMask | ExposureMask);
    XSelectInput(wm->display, frame, ButtonPressMask | ButtonReleaseMask
                 | Button1MotionMask | FocusChangeMask | SubstructureRedirectMask | SubstructureNotifyMask);
    XSelectInput(wm->display, menubutton, ButtonPressMask | ButtonReleaseMask);

    XDefineCursor(wm->display, title, gConfig.Cursors.TitleCursor);
    XDefineCursor(wm->display, frame, gConfig.Cursors.FrameCursor);

    XReparentWindow(wm->display, title, frame, 0, 0);
    XReparentWindow(wm->display, child, frame, 0, gConfig.title_height);
    XReparentWindow(wm->display, menubutton, frame, (int)(attr.width - (gConfig.title_height)), 0);
    XMapWindow(wm->display, title);
    XMapWindow(wm->display, frame);
    XMapWindow(wm->display, menubutton);
    XAddToSaveSet(wm->display, child);
    wg->child = child;
    wg->frame = frame;
    wg->menubutton = menubutton;
    wg->title = title;
    wm->WindowGroupsByClient[child] = *wg;
    wm->WindowGroupsByFrame[frame] = *wg;
    wm->WindowGroupsByMenuButt[menubutton] = *wg;
    wm->WindowGroupsByTitle[title] = *wg;
    free(wg);
}

void WindowManager_RemoveFrame(WindowManager *wm, Window child) {
  XWindowAttributes attr;
  // WindowGroup* wg = (WindowGroup*)Dict_get(wm->WindowGroups, (void*)child);
  // if (wg == NULL) {
  //   return;
  // }
  if (wm->WindowGroupsByClient.count(child) == 0) {
    return;
  }
  WindowGroup* wg = &wm->WindowGroupsByClient[child];
  XGetWindowAttributes(wm->display, child, &attr);
  XReparentWindow(wm->display, wg->child, wm->root, attr.x, attr.y);
  XUnmapWindow(wm->display, wg->frame);
  XUnmapWindow(wm->display, wg->title);
  XUnmapWindow(wm->display, wg->menubutton);
  XRemoveFromSaveSet(wm->display, wg->child);
  wm->WindowGroupsByClient.erase(child);
  wm->WindowGroupsByFrame.erase(wg->frame);
  wm->WindowGroupsByMenuButt.erase(wg->menubutton);
  wm->WindowGroupsByTitle.erase(wg->title);
  XDestroyWindow(wm->display, wg->frame);
  XDestroyWindow(wm->display, wg->title);
  XDestroyWindow(wm->display, wg->menubutton);
  free(wg);
}

void WindowManager_SetFocusTrap(WindowManager* wm, Window child) {
  XGrabButton(wm->display, Button1, AnyModifier, child, False, ButtonPressMask, GrabModeAsync,GrabModeAsync, None, None);
}

void WindowManager_UnsetFocusTrap(WindowManager* wm, Window w) {
  XUngrabButton(wm->display, Button1, AnyModifier, w);
}

void WindowManager_OnMapRequest(WindowManager* wm, XMapRequestEvent ev) {
  WindowManager_AddFrame(wm, ev.window);
  XMapWindow(wm->display, ev.window);
}

void WindowManager_OpenWMenu(WindowManager* wm, XButtonEvent ev);

void WindowManager_LeftClickClientWindow(WindowManager* wm, XButtonEvent ev);

void WindowManager_LeftClickTitleBar(WindowManager* wm, XButtonEvent ev);

void WindowManager_LeftClickFrame(WindowManager* wm, XButtonEvent ev);

void WindowManager_FocusAndRaiseWindow(WindowManager* wm, Window focus);

void WindowManager_OnButtonPressEvent(WindowManager* wm, XButtonEvent ev) {
  Window client = ev.window;
  if (wm->WindowGroupsByClient.count(ev.window) > 0 && ev.button == Button1) {
    WindowManager_LeftClickClientWindow(wm, ev);
  } else if (wm->WindowGroupsByTitle.count(ev.window) > 0 && ev.button == Button1) {
    WindowManager_LeftClickTitleBar(wm, ev);
    client = wm->WindowGroupsByTitle[ev.window].child;
  }else if (wm->WindowGroupsByFrame.count(ev.window) > 0&& ev.button == Button1) {
    WindowManager_LeftClickFrame(wm, ev);
    client = wm->WindowGroupsByFrame[ev.window].child;
  } else if (wm->WindowGroupsByMenuButt.count(ev.window) > 0 && ev.button == Button1) {
    WindowManager_OpenWMenu(wm, ev);
  }
  else WindowManager_FocusAndRaiseWindow(wm, client);

  wm->Switcher.OnButtonPress(ev);
}


void WindowManager_OpenWMenu(WindowManager* wm, XButtonEvent ev) {
  WindowGroup* wg = &wm->WindowGroupsByMenuButt[ev.window];
  Window frame = wg->frame;
  Window child = wg->child;
  Window title = wg->title;
  XWindowAttributes attr;
  XGetWindowAttributes(wm->display, wg->child, &attr);
  WMenu menu;
  menu.AddItem("Close", [=]() {
    XUnmapWindow(wm->display, child);
    XDestroyWindow(wm->display, child);
    WindowManager_RemoveFrame(wm, child);
    wm->Switcher.RemoveWindow(child);
  });

  menu.Show(wm->display, wm->root);
}

void WindowManager_LeftClickTitleBar(WindowManager* wm, XButtonEvent ev) {
  Window frame = wm->WindowGroupsByTitle[ev.window].frame;
  Window child = wm->WindowGroupsByTitle[ev.window].child;
  WindowManager_FocusAndRaiseWindow(wm, child);
  XWindowAttributes attr;
  XGetWindowAttributes(wm->display, frame, &attr);
  MouseMovement_init(&wm->mouseMovement, MouseMoveType_TitleDrag, ev.x_root, ev.y_root, attr.x, attr.y);
}

void WindowManager_LeftClickFrame(WindowManager* wm, XButtonEvent ev) {
  XWindowAttributes attr;
  XGetWindowAttributes
}
