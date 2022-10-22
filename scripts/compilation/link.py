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

# If DEBUG is True, make an output of commands that were used during
# project linking
DEBUG = True

EXTRACT                    = "extract-bc"
DISASSEMBLE                = "llvm-dis"
BC_EXTENSION               = ".bc"
IR_EXTENSION               = ".ll"
MANIFEST                   = "MANIFEST.txt"
DEFAULT_MANIFEST_EXTENSION = ".llvm.manifest"

EI_NIDENT   = 16
E_TYPE_SIZE = 2
ET_NONE     = 0
ET_EXEC     = 2
ET_DYN      = 3


def get_executable_type(input_file):
    e_type = ET_NONE
    with open(input_file, "rb") as file:
        file.read(EI_NIDENT)
        e_type_bytes = file.read(E_TYPE_SIZE)
        e_type, _    = [byte for byte in e_type_bytes]

    return e_type


class CommandLineArguments:
    def __init__(self, executable, executable_type):
        self.executable      = executable
        self.executable_type = executable_type


def filter_input() -> tuple:
    parser = argparse.ArgumentParser(prog="PROG")
    parser.add_argument(dest="executable",
                        help="An executable file (regular or .so).")

    arguments = parser.parse_args()
    executable = arguments.executable

    # Check whether the executable exists
    if not os.path.exists(executable):
        return False, "An executable does not exist."

    # Check whether the executable path is absolute
    if not os.path.isabs(executable):
        executable = os.path.abspath(executable)

    # Check whether the executable is a regular file
    if not os.path.isfile(executable):
        return False, "An executable is not a regular file."

    e_type = get_executable_type(executable)
    if e_type != ET_EXEC and e_type != ET_DYN:
        return False, "An unsupported executable file type."

    return True, CommandLineArguments(executable, e_type)


def get_stem_from_filename(filename) -> str:
    stem, _ = os.path.splitext(filename)

    return stem


def get_filename_from_path(path) -> str:
    _, name = os.path.split(path)

    return name


def link_project(cmd) -> str:
    extract_command         = []
    disassemble_command     = []
    output_ir               = get_stem_from_filename(
        get_filename_from_path(cmd.executable)) + IR_EXTENSION
    debug_used_commands     = ""

    # Extract .bc files and link them together
    extract_command += [EXTRACT]
    extract_command += ["--manifest"]
    if cmd.executable_type == ET_DYN:
        extract_command += ["--bitcode"]
    extract_command += [cmd.executable]

    # Convert the final .bc file to .ll
    disassemble_command += [DISASSEMBLE]
    disassemble_command += [cmd.executable + BC_EXTENSION]
    disassemble_command += ['-o']
    disassemble_command += [output_ir]

    # Issue the manifest with all used files
    old_manifest = cmd.executable + DEFAULT_MANIFEST_EXTENSION
    new_manifest = MANIFEST

    try:
        debug_used_commands += " ".join(extract_command) + '\n'
        subprocess.run(extract_command, check=True)

        debug_used_commands += " ".join(disassemble_command) + '\n'
        subprocess.run(disassemble_command, check=True)

        debug_used_commands += "mv " + old_manifest + " " + new_manifest + "\n"
        os.rename(old_manifest, new_manifest)
    except subprocess.SubprocessError:
        debug_used_commands += "*** An error occurred, abort.\n"

    # Debug purpose only
    return debug_used_commands


def main():
    flag, result = filter_input()
    if not flag:
        sys.exit(result)

    debug_used_command = link_project(result)

    if DEBUG:
        print("==LINKING COMMANDS==")
        print(debug_used_command, end="")
        print("==")

    print("Success.\n")


if __name__ == "__main__":
    main()
