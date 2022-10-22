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
