# flipclock

A flip clock screensaver supported by SDL2.

![flipclock.png](flipclock.png)

Usage:

- Linux/macOS:

	1. Install sdl2, sdl2_ttf.

	2. Clone and compile it with `make`.

	3. Run `./flipclock -h` to see usage. Press `q` or `Esc` to quit. Press `t` to toggle 12h/24h type.

- Windows:

	1. Download Windows project files and static libs from [here](https://github.com/AlynxZhou/flipclock/archive/win32.zip) and get the master code. And take the extracted `Windows` folder into where you extracted master code.

	2. Use Visual Studio or Visual C++ to open project file in `Windows` folder. I create them with Visual Studio 2017. Compile it with `Release` `x86`.

	3. Find `flipclock-win32.exe` in `Windows\Release`. Take it
together with `flipclock.ttf`. Then run it. Press `q` or `Esc` to quit. Press `t` to toggle 12h/24h type.

**The original works of SDL2, SDL2_ttf, libfreetype was done by [SDL](https://www.libsdl.org/), [Freetype](https://www.freetype.org/), special thanks.**

## Known Issue

On GNOME 3.24 the fullscreen toggle may not work properly and may cause system stuck.
