# Copyright 2023 Android Open Source Project
# SPDX-License-Identifier: Apache-2.0

project('aemu', 'cpp', 'c',
         version : '0.1.2',
         license : 'Apache-2.0',
         default_options : ['cpp_std=gnu++17', 'b_staticpic=true'])

targetos = host_machine.system()

if targetos == 'darwin'
   add_languages('objc', required: true, native: false)
endif

lz4 = dependency('lz4', required: true, fallback:['lz4', 'lz4_dep'])
threads = dependency('threads')

subdir('snapshot')
subdir('host-common')
subdir('base')
