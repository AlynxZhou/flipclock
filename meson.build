# If we want to build static program with wrap on Windows,
# We need to set `default_library=static` here
# instead of setting it for each dependencies.
# Because though FlipClock does not link against freetype2 directly,
# sdl2_ttf needs to link against freetype2,
# and if we only pass `default_library=static` to sdl2_ttf,
# it still links against freetype2 dynamically.
project(
  'flipclock',
  'c',
  version: '2.8.5',
  license: 'Apache-2.0',
  default_options: ['c_std=c11', 'default_library=static']
)

cc = meson.get_compiler('c')

c_args = []
if get_option('debug') == true
  warning_level = 3
  c_args += ['-D__DEBUG__']
endif

conf_data = configuration_data()
conf_data.set('project_version', meson.project_version())
conf_data.set('package_bindir', get_option('prefix') / get_option('bindir'))
conf_data.set('package_datadir', get_option('prefix') / get_option('datadir'))
# `sysconfdir` is `/etc` if `prefix` is `/usr`,
# but it will be relative path if using other prefix.
# See <https://mesonbuild.com/Builtin-options.html#universal-options>.
# Meson's string path building operator will not join two absolute path,
# it will keep the right one, so it's OK to handle `sysconfdir` with it.
# See <https://mesonbuild.com/Syntax.html#string-path-building>.
conf_data.set('package_sysconfdir', get_option('prefix') / get_option('sysconfdir'))

configure_file(
  input: 'srcs/config.h.in',
  output: 'config.h',
  configuration: conf_data
)

# Don't install desktop entry for Windows.
if host_machine.system() == 'linux'
  configure_file(
    input: 'dists/one.alynx.FlipClock.desktop.in',
    output: 'one.alynx.FlipClock.desktop',
    configuration: conf_data,
    install: true,
    # If a relative path is given, meson will put it under prefix
    # so we don't need to put `get_option('prefix')` manually here.
    install_dir: get_option('datadir') / 'applications'
  )
endif

sources = files(
  'srcs/main.c',
  'srcs/getarg.c',
  'srcs/flipclock.c'
)

dependencies = []
include_directories = []
# Wrap files will be used as fallback.
sdl2 = dependency('sdl2', required: true)
sdl2_ttf = dependency('SDL2_ttf', required: true)
dependencies += [sdl2, sdl2_ttf]

if host_machine.system() == 'linux'
  executable(
    'flipclock',
    sources: sources,
    c_args: c_args,
    dependencies: dependencies,
    include_directories: include_directories,
    install: true
    # By default, meson install binary to
    # `get_option('prefix') / get_option('bindir')`,
    # so we can omit `install_dir` here.
  )

  install_data(
    'dists' / 'flipclock.conf',
    install_dir: get_option('sysconfdir')
  )
  install_data(
    'dists' / 'flipclock.ttf',
    install_dir: get_option('datadir') / 'fonts'
  )
  install_data(
    'dists' / 'one.alynx.FlipClock.png',
    install_dir: get_option('datadir') / 'icons' / 'hicolor' / '48x48' / 'apps'
  )
  install_data(
    'dists' / 'one.alynx.FlipClock.svg',
    install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps'
  )
  install_data(
    'LICENSE',
    install_dir: get_option('datadir') / 'licenses' / meson.project_name()
  )
elif host_machine.system() == 'windows'
  if get_option('debug') == true
    executable(
      'flipclock',
      sources: sources,
      c_args: c_args,
      dependencies: dependencies,
      include_directories: include_directories,
      install: true,
      install_dir: meson.project_name(),
      name_suffix: 'scr',
      win_subsystem: 'console'
    )
  else
    executable(
      'flipclock',
      sources: sources,
      c_args: c_args,
      dependencies: dependencies,
      include_directories: include_directories,
      install: true,
      install_dir: meson.project_name(),
      name_suffix: 'scr',
      win_subsystem: 'windows'
    )
  endif

  install_data(
    'dists' / 'flipclock.conf',
    install_dir: meson.project_name()
  )
  install_data(
    'dists' / 'flipclock.ttf',
    install_dir: meson.project_name()
  )
  install_data(
    'LICENSE',
    install_dir: meson.project_name()
  )
  install_data(
    'dists' / 'COPYING',
    install_dir: meson.project_name()
  )
  install_data(
    'dists' / '请先读我.txt',
    install_dir: meson.project_name()
  )
endif
