project('custom_target_project', 'cpp')

message = get_option('message')
prog_py_gen = find_program('prog_py_gen', native: true)
prog_python = find_program('python3', 'python', version : '>= 3.8')

# Test custom_target with shell wrapper and depfile generation
message_header = custom_target('message.h',
  output : 'message.h',
  command : [prog_py_gen, '-m', message, '--msg-var', 'msg0', '-o', '@OUTPUT@', '-d', '@DEPFILE@'],
  depfile: 'message.h.d',
  install: false)

# Test custom_target with explicit python program
msg_1_header = custom_target(
  'msg_1.h',
  input: 'py_gen.py',
  output : 'msg_1.h',
  command : [
    prog_python, '@INPUT@',
    '-m', 'msg: using explicit python program',
    '--msg-var', 'msg1',
    '-o', '@OUTPUT@',
    '-d', '@DEPFILE@'],
  depfile: 'msg_1.h.d',
  install: false)

# Test custom_target with a program pulled from a pkg-config file
my_pkgconfig_dep = dependency('my-pkgconfig-prog', native: true)
my_pkgconfig_prog = find_program(my_pkgconfig_dep.get_variable(pkgconfig : 'my_pkgconfig_prog'))
my_pkgconfig_header = custom_target('my_pkgconfig_header.h',
  output : 'my_pkgconfig_header.h',
  command : [my_pkgconfig_prog, '@OUTPUT@'],
  install: false)

executable(
  'custom_target_exe',
  'main.cpp',
  sources: [
    message_header,
    msg_1_header,
    my_pkgconfig_header
  ])
