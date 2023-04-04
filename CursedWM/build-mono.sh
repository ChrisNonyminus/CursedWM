#!/bin/sh

Configuration=$1; export Configuration

rm -rf bin/$Configuration/mono obj/$Configuration/mono
mkdir -p bin/$Configuration/mono
mkdir -p obj/$Configuration/mono

mcs -debug -r:Facades/System.Runtime.dll -r:Facades/System.Collections.dll -r:ext/X11.dll -out:bin/$Configuration/mono/CursedWM.exe SimpleLogger.cs WindowManager.cs WindowManager.ErrorHandling.cs WindowSwitcher.cs WMenu.cs Program.cs

cp ext/X11.dll bin/$Configuration/mono/

mono --aot=full bin/$Configuration/mono/X11.dll
mono --aot=full bin/$Configuration/mono/CursedWM.exe
