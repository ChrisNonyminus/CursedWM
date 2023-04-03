# CursedWM (Working title)

![image](https://user-images.githubusercontent.com/50080567/229363470-756951d7-16b3-4796-87e7-16bc4e5c71ac.png)

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

- X11 bindings (see cswm github repo; nuget package and nuget itself not needed if compiling mono build)

## System libraries

- .NET 7 if running a .NET 7 build.
- Mono if running a mono build.
- X11

## Testing

- Xephyr or something similar.

# Tested Platforms and Runtimes

- Linux/amd64 - .NET 7 JIT build works, .NET 7 AOT build works, mono JIT build works.

# Compiling

There are two methods to compile this. One is to compile the WM as a .NET 7 application, by ``cd``ing into ``./CursedWM`` and running ``dotnet build --project CursedWM.csproj``. The other method is to build with mono. Simply, assuming you have mono, ``cd`` into ``CursedWM``, and run ``build-mono.sh``.

AOT compilation with .NET 7 works.

# Running

If running via Xephyr, just do ``DISPLAY:=<xephyr display number> dotnet run --project CursedWM.csproj`` if you're running the .NET 7 build., or ``DISPLAY:=<xephyr display number> <path to compiled exe>`` if you're running the mono build.



Native X11 sessions not yet tested.
