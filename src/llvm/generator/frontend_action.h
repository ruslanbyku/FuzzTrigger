#ifndef AUTOFUZZ_FRONTEND_ACTION_H
#define AUTOFUZZ_FRONTEND_ACTION_H

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Tooling/Tooling.h>

struct FunctionLocation {
    explicit FunctionLocation(const std::string& name)
    : name_(name), is_filled_(false) {}

    const std::string& name_;
    std::string        entity_;    // There is no "\n" at the end
    bool               is_filled_;
};

// Delete comments from sources (https://gist.github.com/ChunMinChang/88bfa5842396c1fbbc5b)
// r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"', re.DOTALL | re.MULTILINE

// ------------------------------------------------------------------------- //
//                           Frontend Visitor                                //
// ------------------------------------------------------------------------- //
class FrontendVisitor : public clang::RecursiveASTVisitor<FrontendVisitor> {
public:
    explicit FrontendVisitor(clang::ASTContext*, FunctionLocation&);

    bool VisitFunctionDecl(clang::FunctionDecl*);

private:
    clang::ASTContext* context_;
    FunctionLocation&  function_location_;
};

// ------------------------------------------------------------------------- //
//                            Frontend Consumer                              //
// ------------------------------------------------------------------------- //
class FrontendConsumer : public clang::ASTConsumer {
public:
    explicit FrontendConsumer(clang::ASTContext*, FunctionLocation&);

    void HandleTranslationUnit(clang::ASTContext&) override;

private:
    FrontendVisitor visitor_;
};

// ------------------------------------------------------------------------- //
//                             Frontend Action                               //
// ------------------------------------------------------------------------- //
class FrontendAction : public clang::ASTFrontendAction {
public:
    explicit FrontendAction(FunctionLocation&);

    std::unique_ptr<clang::ASTConsumer>CreateASTConsumer(
            clang::CompilerInstance&,
            llvm::StringRef
            ) override;

private:
    FunctionLocation& function_location_;
};

#endif //AUTOFUZZ_FRONTEND_ACTION_H
