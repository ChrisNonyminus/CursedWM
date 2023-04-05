using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using X11;

namespace CursedWM
{
    public class WMenuItem
    {
        public WMenuItem(string name, Action action)
        {
            this.Name = name;
            this.Action = action;
        }

        public string Name { get; set; }
        public Action Action { get; set; }
    }
    public class WMenu
    {
        public WMenu()
        {
            this.Items = new List<WMenuItem>();
        }

        public List<WMenuItem> Items { get; set; }

        public void AddItem(WMenuItem item)
        {
            this.Items.Add(item);
        }

        public void AddItem(string name, Action action)
        {
            this.Items.Add(new WMenuItem(name, action));
        }

        public void Show(IntPtr display, Window root)
        {
            uint menuWidth = 100;
            uint menuHeight = (uint)(this.Items.Count * 20);
            int cursorX = 0;
            int cursorY = 0;
            uint mask = 0;
            Window root_return = new Window();
            Window child_return = new Window();
            int win_x = 0;
            int win_y = 0;
            Xlib.XQueryPointer(display, root, ref root_return, ref child_return, ref cursorX, ref cursorY, ref win_x, ref win_y, ref mask);
            var menu = Xlib.XCreateSimpleWindow(display, root_return, cursorX, cursorY  + (int)(menuHeight / 2),
            menuWidth, menuHeight, 0, 0, 0);
            Xlib.XMapWindow(display, menu);
            Xlib.XFlush(display);
            var menuX = 0;
            var menuY = 0;
            uint menuBorderWidth = 0;
            uint menuDepth = 0;
            Xlib.XGetGeometry(display, menu, ref root, ref menuX, ref menuY, ref menuWidth, ref menuHeight, ref menuBorderWidth, ref menuDepth);
            var menuGCValues = new XGCValues();
            var menuGC = Xlib.XCreateGC(display, menu, 0, ref menuGCValues);

            var itemWindows = new List<Window>();
            var itemDict = new Dictionary<Window, WMenuItem>();

            for (int i = 0; i < this.Items.Count; i++)
            {
                var item = this.Items[i];
                var itemWidth = menuWidth;
                uint itemHeight = (uint)(menuHeight / this.Items.Count);
                var itemX = 0;
                var itemY = (int)(i * itemHeight);
                var itemWindow = Xlib.XCreateSimpleWindow(display, menu, itemX, itemY, itemWidth, itemHeight, 0, 0, 0xFFFFFF);
                Xlib.XMapWindow(display, itemWindow);
                Xlib.XFlush(display);
                Xlib.XDrawString(display, itemWindow, menuGC, 0, (int)itemHeight - this.Items.Count, item.Name, item.Name.Length);
                Xlib.XFlush(display);
                Xlib.XSelectInput(display, itemWindow, EventMask.ButtonPressMask);
                Xlib.XFlush(display);

                itemWindows.Add(itemWindow);
                itemDict.Add(itemWindow, item);
            }

            while (true)
            {
                IntPtr ev = Marshal.AllocHGlobal(24 * sizeof(long));
                Xlib.XNextEvent(display, ev);
                var xevent = Marshal.PtrToStructure<X11.XAnyEvent>(ev);
                if (xevent.type == (int)Event.ButtonPress)
                {

                    var buttonEvent = Marshal.PtrToStructure<X11.XButtonEvent>(ev);
                    if (!itemWindows.Contains(buttonEvent.window))
                        break;
                    var itemWindow = buttonEvent.window;
                    Console.WriteLine("Button pressed at Y: " + buttonEvent.y);
                    Console.WriteLine("Menu height: " + menuHeight);
                    Console.WriteLine("Item count: " + this.Items.Count);

                    var item = itemDict[itemWindow];
                    item.Action();
                    break;
                }
            }

            Xlib.XDestroyWindow(display, menu);
            Xlib.XFlush(display);

        }

    }
}