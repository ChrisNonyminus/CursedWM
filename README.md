# CursedWM (Working title)

An X11 Window Manager built in C#. Yes, really. Sorry.

Based on existing work from https://github.com/ajnewlands/cswm.

Right now the WM is barebones and has few features.

# Features it does have

- Snappable stacking windows (carryover from cswm)
- Rudimentary window switcher sidebar
- Dropdown menu that appears when clicking on the top right button on a window, or right clicking its title in the sidebar. Right now the only menu option is to close the window.
- That's all I can think of at the top of my head.

# Ideal future features

- A Classic Mac OS-style menu bar at the top of the root window.
- A classic Windows-style start/applications menu.
- Window tabbing/docking
- ???

# Dependencies

## Nuget

- X11 bindings (see cswm github repo)

## System libraries

- .NET 6.0 if running a .NET 6.0 build.
- Mono if running a mono build.
- X11

## Testing

- Xephyr or something similar.

# Compiling

There are two methods to compile this. One is to compile the WM as a .NET 6 application, by ``cd``ing into ``./CursedWM`` and running ``dotnet build --project CursedWM.csproj``. The other method, which I haven't quite outlined the steps for, is to build the CursedWM.mono.csproj with mono and .NET Core 2.1.

AOT compilation with .NET 7 is not yet tested on the latest code, but did work last time I tried.

# Running

If running via Xephyr, just do ``DISPLAY:=<xephyr display number> dotnet run --project CursedWM.csproj`` if you're running the .NET 6 build.

Native X11 sessions not yet tested.
