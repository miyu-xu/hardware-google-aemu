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

# Test custom_target with multiple inputs and indexed @INPUT@ variables
my_inputs = ['py_gen.py', 'input0.txt', 'input1.txt']

msg_2_header = custom_target(
  'msg_2.h',
  input: my_inputs,
  output : 'msg_2.h',
  command : [
    prog_python, '@INPUT0@',
    '--msg-file', '@INPUT1@',
    '--msg-var', 'msg2',
    '-o', '@OUTPUT@',
    '-d', '@DEPFILE@'],
  depfile: 'msg_2.h.d',
  install: false)

msg_3_header = custom_target(
  'msg_3.h',
  input: my_inputs,
  output : 'msg_3.h',
  command : [
    prog_python, '@INPUT0@',
    '--msg-file', '@INPUT2@',
    '--msg-var', 'msg3',
    '-o', '@OUTPUT@',
    '-d', '@DEPFILE@'],
  depfile: 'msg_3.h.d',
  install: false)

executable(
  'custom_target_exe',
  'main.cpp',
  sources: [
    message_header,
    msg_1_header,
    my_pkgconfig_header,
    msg_2_header,
    msg_3_header
  ])
