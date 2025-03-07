FlipClock
=========

A flip clock screensaver supported by SDL2.
-------------------------------------------

[Project Website](https://flipclock.alynx.one)

![Screenshot](screenshot.png)

# Installation

## Distribution Package (Recommended)

### Arch Linux

#### Install From AUR

You can get PKGBUILD from [FlipClock's AUR page](https://aur.archlinux.org/packages/flipclock/) and build it manually, or use some AUR helpers.

```
$ paru flipclock
```

#### Install From `archlinuxcn` Repo

First [add `archlinuxcn` repo to your system](https://www.archlinuxcn.org/archlinux-cn-repo-and-mirror/), then use `pacman` to install it.

```
# pacman -S flipclock
```

### Flatpak

FlipClock is also available as Flatpak on Flathub, you can get it from [FlipClock's Flathub page](https://flathub.org/apps/details/one.alynx.FlipClock), or using `flatpak` CLI tool.

```
# flatpak install flathub one.alynx.FlipClock && flatpak run one.alynx.FlipClock
```

### Other Linux Distributions

Please help package FlipClock to your distribution!

### Windows

Just download file with `win` in its name from [release page](https://github.com/AlynxZhou/flipclock/releases/), extract it and right click `flipclock.scr` to install it as a screensaver. Please note I may not have time to build every version for Windows, just pick the latest available one.

## Build From Source

### Linux

1. Install a C compiler, Meson, Ninja, libc, libm, SDL2 and SDL2_ttf.
2. `mkdir build && cd build && meson setup . .. && meson compile`
3. `./flipclock -f ../dists/flipclock.ttf`
4. If you want to install this to your system, you could use `mkdir build && cd build && meson setup --prefix=/usr --buildtype=release . .. && meson compile && sudo meson install`.

### Windows

I saw a windows user says "This program has dlls in its folder so it's not simple!" and I got angry. Anyone who knows compiling, linking and loading won't complain. It might be quite hard for some Windows users to understand how hard to build static libraries. Windows is a horrible platform for developers: there is no package manager so you have to package all dependencies, Visual Studio is slow and complex on adding dependenvies. Thanks to Meson which handles all dirty things, its SDL2 wrap works good and I managed to tweak it to build a static linked program automatically if no pre-built dependency found.

1. Install Meson, Ninja, Visual Studio.
2. Create a prefix directory, for example `d:/flipclock-prefix`, program files will be installed into it.
3. Open `x64 Native Tools Command Prompt for VS 2019` from Start Menu, or other architectures you need.
4. Change dir to where you put this project. Run `mkdir build && cd build && meson setup --prefix=d:/flipclock-prefix --buildtype=release . .. && meson compile && meson install`. You can change prefix argument to other path you created in Step 2, but you need to use UNIX style slash instead of backslash because it's escape character in C.
5. Go to `flipclock` dir under your prefix directory, you can now find `flipclock.scr` and right click it to install it as a screensaver.

### Android

See [flipclock-android](https://github.com/AlynxZhou/flipclock-android/). It may be obsolete because I don't have enough time to update the Android wrapper.

# Configuration

On Linux, program will first use `$XDG_CONFIG_HOME/flipclock.conf`, if `XDG_CONFIG_HOME` is not set or file does not exist, it will use `$HOME/.config/flipclock.conf`. If per-user configuration file does not exist, it will use `/etc/flipclock.conf` or `flipclock.conf` under `sysconfdir` you choosed while building.

If you want to run this program under Wayland, you can set environment variable `SDL_VIDEODRIVER` to `wayland`:

```
SDL_VIDEODRIVER=wayland
```

On Windows, program will use `flipclock.conf` under the same directory as program.

`flipclock.conf` should be installed with the binary by Meson.

# About Fullscreen and Multi-Monitor

This program has multi-monitor support when it is started as fullscreen, but adding/removing monitors while program is running is not supported, and you should not do this.

If you run this program in windowed mode (`-w`), it will only create one window.

# Contribution

If you want some features and you can implement it, PRs are always welcome, but there are some rules or personal habits:

- If you are writing multi-line comment, please use the same style with existing comments. Comments should always occupy a new line. If your comment is longer than Column 80, break it into block comment with `/* */` (but don't break long URL, it's fine), don't use `//` for block comment.
- You can use all **C11** features freely.
- Try to use C standard functions first, until you are implementing some platform-dependent features that libc does not support. Do use preprocessor (`_WIN32`, `__ANDROID__` and `__linux__`) for platform-dependent code. (Note: Android will also define `__linux__`, so if you are targeting traditional Linux distributions, please use `defined(__linux__) && !defined(__ANDROID__)`.)
- Use `RETURN_IF_FAIL(EXPR)` or `RETURN_VAL_IF_FAIL(EXPR, VAL)` from `flipclock.h` to check arguments for newly added functions.
- Try not to pull new dependencies into project other than SDL2 and SDL2_ttf, it's too brain damage to add dependencies when building on Windows, building on this platform is a disaster and packaging on this platform is a mistery.
- **When you are coding please use [Linux kernel coding style](https://www.kernel.org/doc/html/latest/process/coding-style.html).** There is a `.clang-format` for this project, please run `clang-format` yourself before committing something. It will keep most coding style consistent.
- There are still some coding style `clang-format` cannot change, please keep the same as existing code. For example, add period for all comments and printed text, add `\n` yourself for logging.
- Prefer to `++i`, except when you really need `i` before increasement.
- I prefer to write commit message in past tense, capitalize the first character and add period. For example "Added new feature.", "Updated README.md.".
- If you added new options to configuration file and you are able to write Chinese, please also update `dists/请先读我.txt`. This file is a README for Chinese Windows users and **should use GB2312 as encoding and CRLF as return**.

# License

[Apache-2.0](./LICENSE)
