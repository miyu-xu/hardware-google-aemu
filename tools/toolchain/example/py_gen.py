# -*- coding: utf-8 -*-
# Copyright 2023 - The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the',  help='License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an',  help='AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description='Generate a C++ header file with a message.')
    parser.add_argument('-m', '--message', required=True,
                        help='The message to include in the header file.')
    parser.add_argument('-o', '--output', required=True, help='The output header file path.')
    parser.add_argument('-d', '--depfile', required=True, help='The output dependency file.')
    args = parser.parse_args()

    out = Path(args.output).absolute()
    depfile = Path(args.depfile).absolute()
    py_gen_file = Path(__file__).absolute()

    print(f"Writing {out}")
    with open(out, 'w') as f:
        f.write(f'#include <string>\n\n')
        f.write(f'namespace generated {{\n')
        f.write(f'  std::string msg = "{args.message}";\n')
        f.write(f'}}\n')

    with open(depfile, 'w') as f:
        # Make py_gen.py a dependency of message.h so a build gets trigger when this file is
        # modified.
        f.write(f'{str(out)}: {str(py_gen_file)}')
if __name__ == '__main__':
    main()
