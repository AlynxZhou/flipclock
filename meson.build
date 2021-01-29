project(
  'flipclock',
  'c',
  version: '2.6.0',
  license: 'Apache-2.0',
  default_options: ['c_std=c11']
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
if host_machine.system() == 'linux'
  # Just simply use pkg-config.
  sdl2 = dependency('sdl2', required: true)
  sdl2_ttf = dependency('SDL2_ttf', required: true)
  dependencies += [sdl2, sdl2_ttf]
  # Deps from pkg-config don't need to set include_directories explicitly.
elif host_machine.system() == 'windows'
  # I failed to build static SDL2 on Windows,
  # so I just find downloaded prebuild libraries.
  vc_path = 'x86'
  if host_machine.cpu_family() == 'x86_64'
    vc_path = 'x64'
  endif
  sdl2 = cc.find_library(
    'SDL2',
    dirs: [meson.project_source_root() / 'deps' / 'SDL2' / 'lib' / vc_path],
    required: true
  )
  sdl2main = cc.find_library(
    'SDL2main',
    dirs: [meson.project_source_root() / 'deps' / 'SDL2' / 'lib' / vc_path],
    required: true
  )
  sdl2_ttf = cc.find_library(
    'SDL2_ttf',
    dirs: [meson.project_source_root() / 'deps' / 'SDL2_ttf' / 'lib' / vc_path],
    required: true
  )
  sdl2_inc = include_directories(
    meson.project_source_root() / 'deps' / 'SDL2' / 'include'
  )
  sdl2_ttf_inc = include_directories(
    meson.project_source_root() / 'deps' / 'SDL2_ttf' / 'include'
  )
  dependencies += [sdl2, sdl2main, sdl2_ttf]
  include_directories += [sdl2_inc, sdl2_ttf_inc]
endif

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
    'flipclock.ttf',
    install_dir: get_option('datadir') / 'fonts'
  )
  install_data(
    'flipclock.png',
    install_dir: get_option('datadir') / 'pixmaps'
  )
  install_data(
    'flipclock.desktop',
    install_dir: get_option('datadir') / 'applications'
  )
  install_data(
    'LICENSE',
    install_dir: get_option('datadir') / 'license' / meson.project_name()
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
      win_subsystem: 'windows'
    )
  endif

  install_data(
    'flipclock.ttf',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'LICENSE',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'COPYING',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    '请先读我.txt',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'deps' / 'SDL2' / 'lib' / vc_path / 'SDL2.dll',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'deps' / 'SDL2_ttf' / 'lib' / vc_path / 'SDL2_ttf.dll',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'deps' / 'SDL2_ttf' / 'lib' / vc_path / 'libfreetype-6.dll',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'deps' / 'SDL2_ttf' / 'lib' / vc_path / 'zlib1.dll',
    install_dir: get_option('prefix') / meson.project_name()
  )
endif
