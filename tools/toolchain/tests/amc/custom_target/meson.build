project('custom_target_project', 'cpp')

message = get_option('message')
py_gen = find_program('py_gen', native: true)

message_header = custom_target('message.h',
  output : 'message.h',
  command : [py_gen, '-m', message, '-o', '@OUTPUT@'],
  install : false)

executable('custom_target_exe',
           'main.cpp',
           sources: [message_header])
