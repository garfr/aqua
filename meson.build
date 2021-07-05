project('aqua', 'c')

src = [
  'src/aqua.c',
  'src/object.c',
  'src/state.c',
  'src/dump.c',
  'src/compiler.c',
  'src/vm.c',
  'src/gc.c',
]

inc = include_directories('.')

aqualib = library(
  'aqua',
  src,
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  include_directories : [inc]
)

aquarepl = executable(
  'aquarepl',
  'aquarepl.c',
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  link_with : aqualib,
  include_directories : [inc]
)

aquaexec = executable(
  'aquaexec',
  'aquaexec.c',
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  link_with : aqualib,
  include_directories : [inc]
)
