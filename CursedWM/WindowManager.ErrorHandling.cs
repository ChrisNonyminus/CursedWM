using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using X11;

namespace CursedWM
{
    public partial class WindowManager
    {

        public int ErrorHandler(IntPtr display, ref XErrorEvent ev)
        {
            if (ev.error_code == 10) // BadAccess, i.e. another window manager has already claimed those privileges.
            {
                Log.Error("X11 denied access to window manager resources - another window manager is already running");
                Environment.Exit(1);
            }

            // Other runtime errors and warnings.
            var description = Marshal.AllocHGlobal(1024);
            Xlib.XGetErrorText(this.display, ev.error_code, description, 1024);
            var desc = Marshal.PtrToStringAnsi(description);
            Log.Warn($"X11 Error: {desc}");
            Marshal.FreeHGlobal(description);
            return 0;
        }
    }
}
