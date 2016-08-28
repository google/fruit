# Copyright 2016 Google Inc. All Rights Reserved.
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


def generate_makefile(sources, executable_name, compile_command, link_command, link_command_suffix, output_dir, shared_libraries_paths):
    wrapper_script_rule_template = """
{executable_name}: {executable_name}-exec
\techo '#!/bin/bash' >{executable_name}
\techo 'LD_LIBRARY_PATH="{shared_libraries_paths}:$$LD_LIBRARY_PATH" exec {output_dir}/{executable_name}-exec "$$@"' >>{executable_name}
\tchmod +x {executable_name}
"""

    link_rule_template = """
{executable_name}-exec: {object_files}
\t{link_command} {object_files} -o {executable_name}-exec {link_command_suffix}
"""
    compile_rule_template = """
{name}.o: {name}.cpp
\t{compile_command} -c {name}.cpp -o {name}.o
"""

    clean_rule_template = """
clean:
\trm -f {object_files} {executable_name}
"""

    compile_rules = []
    object_files = []
    for source in sources:
        compile_rule = compile_rule_template.format(
            name=source,
            compile_command=compile_command)
        compile_rules += [compile_rule]
        object_files += ['%s.o' % source]

    wrapper_script_rule = wrapper_script_rule_template.format(
        executable_name=executable_name,
        shared_libraries_paths=':'.join(shared_libraries_paths),
        output_dir=output_dir)

    link_rule = link_rule_template.format(
        object_files=' '.join(object_files),
        link_command=link_command,
        link_command_suffix=link_command_suffix,
        executable_name=executable_name)

    clean_rule = clean_rule_template.format(
        object_files=' '.join(object_files),
        executable_name=executable_name)

    # We put the wrapper script rule first so that it's the default Make target.
    return wrapper_script_rule + link_rule + ''.join(compile_rules) + clean_rule
