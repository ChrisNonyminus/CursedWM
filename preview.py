from pyvirtualdisplay.smartdisplay import SmartDisplay
from easyprocess import EasyProcess
import subprocess
import os
from IPython.display import display


if "XDG_SEAT" in os.environ:
    del os.environ["XDG_SEAT"]
with SmartDisplay(size=(1024,768), backend="xephyr") as disp:
    # with EasyProcess(["./awful_wm"]) as proc:
    #     EasyProcess(["xterm", "-fg", "white", "-bg",  "black"]).start()
    #     EasyProcess(["xeyes"]).start()
    #     # grab image
    #     while True:
    #         # disp.waitgrab() grabs the screen and returns an image
    #         img = disp.waitgrab()
    #         display(img, clear=True)
    with EasyProcess(["dotnet", "run", "--project", "CursedWM"]) as proc:
        EasyProcess(["xterm", "-fg", "white", "-bg",  "black"]).start()
        EasyProcess(["xeyes"]).start()
        # grab image
        while True:
            # disp.waitgrab() grabs the screen and returns an image
            img = disp.waitgrab()
            display(img, clear=True)

