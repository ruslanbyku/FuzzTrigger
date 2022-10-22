#!/usr/bin/python
#
# Copyright 2022 Ruslan Byku
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
import subprocess
import argparse
import tempfile
import re

# If DEBUG is True, make an output of the results
DEBUG = True

OUTPUT_POSTFIX      = "_sources.txt"
DISASSEMBLE         = "llvm-dis"
BC_EXTENSION        = ".bc"
IR_EXTENSION        = ".ll"
IR_HEADER_MAX_LINES = 4
PATTERN             = re.compile("\s*source_filename\s*=\s*\"(.+)\"")


class CommandLineArguments:
    def __init__(self, manifest, project_path):
        self.manifest      = manifest
        self.project_path  = project_path


def filter_input() -> tuple:
    parser = argparse.ArgumentParser(prog="PROG")
    parser.add_argument("--manifest", "-M",
                        required=True,
                        help="MANIFEST.txt")
    parser.add_argument(dest="project_path",
                        help="A directory with a target project.")

    arguments    = parser.parse_args()
    manifest     = arguments.manifest
    project_path = arguments.project_path

    # Check whether the manifest exists
    if not os.path.exists(manifest):
        return False, "A manifest file does not exist."

    # Check whether the project path exists
    if not os.path.exists(project_path):
        return False, "A project path does not exist."

    # Check whether the manifest path is absolute
    if not os.path.isabs(manifest):
        input_file = os.path.abspath(manifest)

    # Check whether the project path is absolute
    if not os.path.isabs(project_path):
        input_file = os.path.abspath(project_path)

    # Check whether the manifest path is a regular file
    if not os.path.isfile(manifest):
        return False, "A manifest file is not a regular file."

    # Check whether the project path is a directory
    if not os.path.isdir(project_path):
        return False, "A project path is not a directory."

    # Check whether the manifest file is not empty
    if os.path.getsize(manifest) < 1:
        return False, "A manifest file is empty."

    return True, CommandLineArguments(manifest, project_path)


def get_project_name(project_path) -> str:
    if project_path.endswith('/'):
        project_path = project_path[:-1]

    _, name = os.path.split(project_path)

    return name


def find_source_absolute_path(bitcode_file, source_file, project_path) -> str:
    if os.path.isabs(source_file):
        return source_file

    directory, _ = os.path.split(bitcode_file)
    absolute_source_path = directory + '/' + source_file
    while project_path in absolute_source_path:
        if os.path.exists(absolute_source_path):
            return absolute_source_path

        directory, _ = os.path.split(directory)
        absolute_source_path = directory + '/' + source_file

    return ""


def find_sources(cmd) -> tuple:
    flag          = True
    sources       = []
    debug_output  = ""

    tmp_file = tempfile.NamedTemporaryFile(mode="w+", delete=True)
    debug_output += f"A temporary file {tmp_file.name} created.\n"

    with open(cmd.manifest, 'r') as manifest_file:
        # Assume that all files are absolute
        for line_number, line in enumerate(manifest_file, 1):
            if line.endswith('\n'):
                line = line[:-1]  # Remove trailing \n

            # Finish search when an empty line encounters
            if not line:
                break

            # If even one file from the manifest does not exist, abort.
            if not os.path.exists(line):
                debug_output += "A file from the manifest does not exist."
                flag = False
                break

            # The file is not a bitcode file format, abort.
            if not line.endswith(BC_EXTENSION):
                debug_output += "A file from the manifest is not a bitcode " \
                                "file."
                flag = False
                break

            bitcode_file = line
            debug_output += f"{line_number}. A bitcode file {bitcode_file} " \
                            f"was found.\n"

            disassemble_command = [DISASSEMBLE]
            disassemble_command += [bitcode_file]
            disassemble_command += ['-o']
            disassemble_command += [tmp_file.name]

            try:
                debug_output += f"Disassemble {bitcode_file} " \
                                f"to {tmp_file.name}.\n"
                debug_output += "|\n"
                debug_output += "--> "
                debug_output += " ".join(disassemble_command)
                debug_output += '\n'
                subprocess.run(disassemble_command, check=True)
            except subprocess.SubprocessError:
                debug_output += f"An error occurred during " \
                                f"disassembling {bitcode_file}."
                flag = False
                break

            debug_output += f"Find a source file in {tmp_file.name}.\n"
            source_file = []
            with open(tmp_file.name, 'r') as ir_file:
                for ir_line_number, ir_line in enumerate(ir_file, 1):
                    if ir_line_number > IR_HEADER_MAX_LINES or source_file:
                        break

                    source_file = PATTERN.findall(ir_line)

            if not source_file:
                # No source file was found in the IR file, abort.
                debug_output += f"No source file was found in {bitcode_file}."
                flag = False
                break

            debug_output += "|\n"
            debug_output += "--> "
            debug_output += f"{source_file[0]}\n"

            # Find the corresponding absolute path of the found source file
            debug_output += "  |\n"
            debug_output += "  --> "
            source_file_absolute_path = find_source_absolute_path(
                                                               bitcode_file,
                                                               source_file[0],
                                                               cmd.project_path)
            if not source_file_absolute_path:
                # The absolute path was not found, abort.
                debug_output += "does not exist.\n"
                flag = False
                break

            # Success
            sources.append(source_file_absolute_path)
            debug_output += source_file_absolute_path
            debug_output += '\n'

    tmp_file.close()

    if not flag:
        return False, [], debug_output

    return True, sources, debug_output


def main():
    filter_flag, filter_result = filter_input()
    if not filter_flag:
        sys.exit(filter_result)

    source_flag, source_result, debug_output = find_sources(filter_result)
    if not source_flag:
        if DEBUG:
            sys.exit(debug_output)
        else:
            sys.exit(1)

    # Write all found sources into a file
    output_file_name = \
        get_project_name(filter_result.project_path) + OUTPUT_POSTFIX
    with open(output_file_name, "w") as output_file:
        for file in source_result:
            output_file.write(file)
            output_file.write('\n')

    debug_output += f"\nWrite source paths into {output_file_name}."

    if DEBUG:
        print(debug_output)

    print("Success.")


if __name__ == "__main__":
    main()
