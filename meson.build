project(
  'flipclock',
  'c',
  version: '2.4.0',
  license: 'Apache-2.0',
  default_options: ['c_std=c11']
)

if host_machine.system() == 'windows'
  add_global_arguments('-Ddefault_library=static', language: 'c')
endif

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

# dep = dependency('foo', fallback : [subproject_name, variable_name])
sdl2 = dependency('sdl2', fallback: ['sdl2', 'sdl2_dep'])
sdl2_ttf = dependency('SDL2_ttf', fallback: ['sdl2_ttf', 'sdl2_ttf_dep'])

dependencies = [sdl2, sdl2_ttf]

if host_machine.system() == 'windows'
  executable(
    'flipclock',
    sources: sources,
    dependencies: dependencies,
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
