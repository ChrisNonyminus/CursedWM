using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using X11;

namespace CursedWM
{
    public class WindowSwitcher
    {
        public WindowSwitcher(IntPtr display, Window root)
        {
            this.WindowRefs = new Dictionary<Window, Window>();

            this.display = display;
            this.root = root;
            
            uint root_width = 0;
            uint root_height = 0;
            int root_x = 0;
            int root_y = 0;
            uint root_border_width = 0;
            uint root_depth = 0;

            Xlib.XGetGeometry(display, root, ref root, ref root_x, ref root_y, ref root_width, ref root_height, ref root_border_width, ref root_depth);

            this.switcherWindow = Xlib.XCreateSimpleWindow(display, root, 0, 20, 100, root_height - 20, 1, 0, 0xC0C0C0);
            Xlib.XMapWindow(display, switcherWindow);
            Xlib.XFlush(display);
            
            Update();
        }

        public IntPtr display { get; set; }
        public Window root { get; set; }

        public Window switcherWindow { get; set; }

        public Dictionary<Window, Window> WindowRefs { get; set; }
        
        public void AddWindow(Window window)
        {
            var listentry = Xlib.XCreateSimpleWindow(display, switcherWindow, 0, WindowRefs.Count * 20, 100, 20, 1, 0, 0xFFFFFF);
            Xlib.XMapWindow(display, listentry);
            Xlib.XFlush(display);
            WindowRefs.Add(window, listentry);

            Xlib.XGrabButton(display, Button.LEFT, KeyButtonMask.AnyModifier, listentry, false, EventMask.ButtonPressMask, GrabMode.Async, GrabMode.Async, 0, 0);
            Xlib.XGrabButton(display, Button.RIGHT, KeyButtonMask.AnyModifier, listentry, false, EventMask.ButtonPressMask, GrabMode.Async, GrabMode.Async, 0, 0);

            Update();
        }

        public void RemoveWindow(Window window)
        {
            Xlib.XDestroyWindow(display, WindowRefs[window]);
            WindowRefs.Remove(window);
            Update();
        }

        public void Update() {
            Xlib.XSetWindowBackground(display, switcherWindow, 0xFFFFFF);
            
            Xlib.XFlush(display);

            for (int i = 0; i < WindowRefs.Count; i++)
            {
                var window = WindowRefs.ElementAt(i).Key;
                var listentry = WindowRefs.ElementAt(i).Value;
                Xlib.XSetWindowBackground(display, listentry, 0xFFFFFF);
                Xlib.XFlush(display);

                try {
                string windowName = "";
                Xlib.XFetchName(display, window, ref windowName);
                Xlib.XDrawString(display, listentry, Xlib.XDefaultGC(display, 0), 0, 10, windowName, windowName.Length);
                }
                catch (Exception e) {
                    Console.WriteLine(e);
                    // likely a window that has been closed
                    RemoveWindow(window);
                    return;
                        
                }

                Xlib.XFlush(display);

                Xlib.XMoveWindow(display, listentry, 0, i * 20);

                Xlib.XFlush(display);

            }



        }


        public void OnButtonPress(XButtonEvent e)
        {
            if (e.button == (int)Button.LEFT)
            {
                foreach (var window in WindowRefs)
                {
                    if (window.Value == e.window)
                    {
                        Xlib.XSetInputFocus(display, window.Key, RevertFocus.RevertToNone, (long)e.time);
                        Xlib.XRaiseWindow(display, window.Key);
                        Xlib.XFlush(display);
                    }
                }
            } else if (e.button == (int)Button.RIGHT) {
                foreach (var window in WindowRefs)
                {
                    if (window.Value == e.window)
                    {
                        // open WMenu
                        var wmenu = new WMenu();
                        wmenu.AddItem("Close", () => {
                            Xlib.XDestroyWindow(display, window.Key);
                            Xlib.XFlush(display);
                            RemoveWindow(window.Key);
                        });

                        wmenu.Show(display, root);
                    }
                }
            }
        }
    }
}