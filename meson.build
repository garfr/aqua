project('aqua', 'c')

src = [
  'src/aqua.c'
]

inc = include_directories('.')

aqualib = library(
  'aqua',
  src,
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  include_directories : [inc]
)

aqua = executable(
  'aqua',
  'repl.c',
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  link_with : aqualib,
  include_directories : [inc]
)