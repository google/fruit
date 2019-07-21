#!/usr/bin/env python3
#  Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS-IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from absl.testing import parameterized
from fruit_test_common import *

class TestDefnHIncludes(parameterized.TestCase):
    def test_defn_file_inclusion(self):
        if os.sep != '/':
            # This only works in platforms where paths are /-separated.
            return
        include_pattern = re.compile(' *#include *<(.*)> *')

        fruit_headers_root_absolute_path = os.path.abspath(PATH_TO_FRUIT_STATIC_HEADERS)

        includes = {}
        for root, _, files in os.walk(fruit_headers_root_absolute_path):
            for file in files:
                if file.endswith('.h'):
                    path = os.path.join(root, file)
                    with open(path, 'r') as f:
                        current_includes = set()
                        for line in f.readlines():
                            # Remove the newline
                            line = line[:-1]
                            matches = re.match(include_pattern, line)
                            if matches:
                                current_includes.add(matches.groups()[0])
                        root_relative_path = root.replace(fruit_headers_root_absolute_path, '')
                        relative_path = os.path.join(root_relative_path, file)
                        if relative_path.startswith(os.sep):
                            relative_path = relative_path[1:]
                        includes[relative_path] = current_includes

        for defn_file, defn_file_includes in includes.items():
            if defn_file.endswith('.defn.h'):
                main_header_file = defn_file.replace('.defn.h', '.h')
                # This is a special case where we don't follow the convention, so we need to specify the corresponding main
                # header file explicitly.
                if defn_file == 'fruit/impl/component_functors.defn.h':
                    main_header_file = 'fruit/component.h'

                # The .defn.h files for headers in fruit/ are in fruit/impl/
                alternative_main_header_file = main_header_file.replace('fruit/impl/', 'fruit/')

                if main_header_file not in includes and alternative_main_header_file not in includes:
                    raise Exception('Can\'t find the .h header corresponding to: %s. Considered: %s' % (defn_file, (main_header_file, alternative_main_header_file)))
                if main_header_file not in defn_file_includes and alternative_main_header_file not in defn_file_includes:
                    raise Exception('%s should have included %s, but it includes only: %s' % (defn_file, main_header_file, defn_file_includes))
                if main_header_file in includes and defn_file not in includes[main_header_file]:
                    raise Exception('%s should have included %s, but it includes only: %s' % (main_header_file, defn_file, includes[main_header_file]))
                if alternative_main_header_file in includes and defn_file not in includes[alternative_main_header_file]:
                    raise Exception('%s should have included %s, but it includes only: %s' % (main_header_file, defn_file, includes[alternative_main_header_file]))
                for other_header, other_header_includes in includes.items():
                    if other_header not in (main_header_file, alternative_main_header_file) and defn_file in other_header_includes:
                        raise Exception('Unexpected direct include: %s includes %s' % (other_header, defn_file))

if __name__ == '__main__':
    absltest.main()
