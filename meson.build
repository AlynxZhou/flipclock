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
  version: '2.8.3',
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
conf_data.set('install_prefix', get_option('prefix'))
conf_data.set('project_version', meson.project_version())

configure_file(
  input: 'srcs/config-meson.h.in',
  output: 'config.h',
  configuration: conf_data
)

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
    install: true,
    install_dir: get_option('prefix') / 'bin'
  )

  install_data(
    'LICENSE',
    install_dir: get_option('datadir') / 'licenses' / meson.project_name()
  )
  install_data(
    'dists' / 'flipclock.ttf',
    install_dir: get_option('datadir') / 'fonts'
  )
  install_data(
    'dists' / 'flipclock.png',
    install_dir: get_option('datadir') / 'pixmaps'
  )
  install_data(
    'dists' / 'flipclock.desktop',
    install_dir: get_option('datadir') / 'applications'
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
      install_dir: get_option('prefix') / meson.project_name(),
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
      install_dir: get_option('prefix') / meson.project_name(),
      name_suffix: 'scr',
      win_subsystem: 'windows'
    )
  endif

  install_data(
    'LICENSE',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'dists' / 'flipclock.ttf',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'dists' / 'COPYING',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'dists' / '请先读我.txt',
    install_dir: get_option('prefix') / meson.project_name()
  )
endif
