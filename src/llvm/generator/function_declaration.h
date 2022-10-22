/*
 * Copyright 2022 Ruslan Byku
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FUZZTRIGGER_FUNCTION_DECLARATION_H
#define FUZZTRIGGER_FUNCTION_DECLARATION_H

#include <string>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Tooling/Tooling.h>

struct FunctionEntity {
    explicit FunctionEntity(std::string name) : name_(std::move(name)) {}

    std::string name_;

    // If the declaration is found, it is NOT empty
    std::string declaration_; // There is no "\n" at the end
};

using SourceEntity = std::vector<FunctionEntity>;

// Delete comments from sources (https://gist.github.com/ChunMinChang/88bfa5842396c1fbbc5b)
// r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"', re.DOTALL | re.MULTILINE
namespace FunctionDeclaration {
    // https://stackoverflow.com/a/49710635
    std::string Extract(clang::ASTContext*, clang::FunctionDecl*);
}

#endif //FUZZTRIGGER_FUNCTION_DECLARATION_H
