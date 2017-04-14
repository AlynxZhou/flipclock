# flipclock

A flip clock screensaver supported by SDL2.

![flipclock.png](flipclock.png)

Usage:

- Linux/macOS:

	1. Install sdl2, sdl2_ttf.

	2. Compile it with `make`.

	3. Run `./flipclock -h` to see usage. Press `q` or `Esc` to quit.

- Windows:

	1. Use Visual Studio or Visual C++ to open project file in directory `Windows`. I create them with Visual Studio 2017.

	2. Compile it with `Release`.

	3. Find `flipclock-win32.exe` in `Windows\Release`. Take it together with `flipclock.ttf`. Then run it. Press `q` or `Esc` to quit.

**PS** I added SDL2, SDL2_ttf, libfreetype-6 static lib for Windows in this project, which is needed if you want to compile the exe without dll. But I am not sure whether I can release them with my code and I am not the author of them.

**IF THERE ARE LICENSE SAY THAT I CANNOT DO THAT, PLEASE TELL ME, AND I AM WILLING TO REMOVE THEM.**

For Windows users please visit their website first before you use these lib.

**The original works of SDL2, SDL2_ttf, libfreetype was done by [SDL](https://www.libsdl.org/), [Freetype](https://www.freetype.org/), special thanks.**
