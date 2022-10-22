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
import textwrap

# If DEBUG is True, make an output of commands that were used during
# project compilation
DEBUG = True

COMPILER_ENVIRON = {"LLVM_COMPILER": "clang"}
COMPILER_WRAPPER = "wllvm"
CC_COMPILER      = {"CC": COMPILER_WRAPPER}
AUTOTOOLS        = "configure"
CMAKE            = "cmake"
MAKE             = "make"
OPTIMIZATION     = "CFLAGS='-O0'"  # https://stackoverflow.com/a/9725326


PARSER_DESCRIPTION = "description:\n  PROG changes directory to input_file " \
                     "and commences compilation process."
PARSER_EPILOG = f"""
examples:
  1. cmake with default arguments:
  $ PROG --cmake /path/to/project
  ==> ... cmake .
                                       
  2. autotools with default arguments:
  $ PROG --autotools /path/to/project
  ==> ... ./configure
                                       
  3. autotools with override arguments:
  $ PROG -A --args=--with-openssl /path/to/project
  ==> ... ./configure --with-openssl
                                       
  4. cmake with override arguments:
  $ PROG -C --args=.,--list-presets,-v /path/to/project
  ==> ... cmake . --list-presets -v
  $ PROG -C --args="-D <var>:<type>=<value> -B <path-to-build>" /path/to/project
  ==> ... cmake -D <var>:<type>=<value> -B <path-to-build>
"""


class CommandLineArguments:
    def __init__(self, project_directory, build_engine, build_arguments):
        self.project_directory = project_directory
        self.build_engine      = build_engine
        self.build_arguments   = build_arguments


def filter_input() -> tuple:
    parser = argparse.ArgumentParser(
        prog="PROG",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent(PARSER_DESCRIPTION),
        epilog=textwrap.dedent(PARSER_EPILOG)
    )
    parser.add_argument(dest="project_path",
                        help="A directory with a target project to be "
                             "compiled.")
    parser.add_argument("--autotools", "-A",
                        action="store_true",
                        help="A project needs to be compiled with autotools.")
    parser.add_argument("--cmake", "-C",
                        action="store_true",
                        help="A project needs to be compiled with cmake.")
    parser.add_argument("--args=arg1,arg2,...",
                        dest="args",
                        metavar="",
                        help="Override arguments for autotools or cmake.")

    arguments = parser.parse_args()
    project_path = arguments.project_path

    # Check whether the project path exists
    if not os.path.exists(project_path):
        return False, "A project path does not exist."

    # Check whether the project path is absolute
    if not os.path.isabs(project_path):
        project_path = os.path.abspath(project_path)

    # Check whether the project path is a directory
    if not os.path.isdir(project_path):
        return False, "A project path is not a directory."

    # Get build engine
    if arguments.autotools:
        build_engine = AUTOTOOLS
    elif arguments.cmake:
        build_engine = CMAKE
    else:
        return False, "No build engine is specified."

    # Get build arguments
    build_arguments = []
    if arguments.args:
        build_arguments = arguments.args.split(',')

    return True, CommandLineArguments(project_path,
                                      build_engine,
                                      build_arguments)


def compile_project(cmd) -> str:
    build_command       = []
    compile_command     = [MAKE, OPTIMIZATION]
    debug_used_commands = ""

    os.chdir(cmd.project_directory)

    def set_environ(flag) -> str:
        environ_name, = list(COMPILER_ENVIRON.keys())

        if flag:
            environ_value, = list(COMPILER_ENVIRON.values())
            os.environ[environ_name] = environ_value

            return f"export {environ_name}={environ_value}"
        else:
            del os.environ[environ_name]

            return f"unset {environ_name}"

    debug_used_commands += set_environ(True) + '\n'

    compiler_name,  = list(CC_COMPILER.keys())
    compiler_value, = list(CC_COMPILER.values())
    build_command += ["env", f"{compiler_name}={compiler_value}"]

    if cmd.build_engine == AUTOTOOLS:
        build_command += [f"./{AUTOTOOLS}"]
    else:
        build_command += [CMAKE]

    if cmd.build_arguments:
        build_command += cmd.build_arguments
    else:
        if cmd.build_engine == CMAKE:
            build_command += ['.']

    try:
        debug_used_commands += " ".join(build_command[1:]) + '\n'
        subprocess.run(build_command, check=True)

        debug_used_commands += " ".join(compile_command) + '\n'
        subprocess.run(compile_command, check=True)

        debug_used_commands += set_environ(False) + '\n'
    except subprocess.CalledProcessError:
        debug_used_commands += "*** An error occurred, abort.\n"

    # Debug purpose only
    return debug_used_commands


def main():
    flag, result = filter_input()
    if not flag:
        sys.exit(result)

    debug_used_command = compile_project(result)

    if DEBUG:
        print("==COMPILATION COMMANDS==")
        print(debug_used_command, end="")
        print("==")

    print("Success.\n")


if __name__ == "__main__":
    main()
