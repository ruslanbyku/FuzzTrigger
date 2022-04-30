#ifndef AUTOFUZZ_FUNCTION_DECLARATION_H
#define AUTOFUZZ_FUNCTION_DECLARATION_H

#include <string>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Tooling/Tooling.h>

// Delete comments from sources (https://gist.github.com/ChunMinChang/88bfa5842396c1fbbc5b)
// r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"', re.DOTALL | re.MULTILINE
namespace FunctionDeclaration {
    // https://stackoverflow.com/a/49710635
    std::string Extract(clang::ASTContext*, clang::FunctionDecl*);
}

#endif //AUTOFUZZ_FUNCTION_DECLARATION_H
