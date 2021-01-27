project(
  'flipclock',
  'c',
  version: '2.4.0',
  license: 'Apache-2.0',
  default_options: ['c_std=c11']
)

cc = meson.get_compiler('c')

if get_option('debug') == true
  warning_level = 3
  add_project_arguments('-D__DEBUG__', language: 'c')
endif

conf_data = configuration_data()
conf_data.set('install_prefix', get_option('prefix'))
conf_data.set('project_version', meson.project_version())

configure_file(
  input: 'srcs/config-meson.h.in',
  output: 'config.h',
  configuration: conf_data
)

sources = files([
  'srcs/main.c',
  'srcs/getarg.c',
  'srcs/flipclock.c'
])

if host_machine.system() == 'linux'
  # dep = dependency('foo', fallback : [subproject_name, variable_name])
  sdl2 = dependency('sdl2', required: true)
  sdl2_ttf = dependency('SDL2_ttf', required: true)

  dependencies = [sdl2, sdl2_ttf]
  include_directories = []
else
  vc_path = 'x86'
  if host_machine.cpu_family() == 'x86_64'
    vc_path = 'x64'
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
  dependencies = [sdl2, sdl2_main, sdl2_ttf]
  include_directories = [sdl2_inc, sdl2_ttf_inc]
endif

dependencies = [sdl2, sdl2_ttf]

if host_machine.system() == 'linux'
  executable(
    'flipclock',
    sources: sources,
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
else
  executable(
    'flipclock',
    sources: sources,
    dependencies: dependencies,
    include_directories: include_directories,
    install: true,
    install_dir: get_option('prefix') / meson.project_name(),
    win_subsystem: 'windows'
  )

  install_data(
    'flipclock.ttf',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'flipclock.png',
    install_dir: get_option('prefix') / meson.project_name()
  )
  install_data(
    'flipclock.desktop',
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
endif
