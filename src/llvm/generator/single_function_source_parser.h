#ifndef AUTOFUZZ_SINGLE_FUNCTION_SOURCE_PARSER_H
#define AUTOFUZZ_SINGLE_FUNCTION_SOURCE_PARSER_H

#include "function_declaration.h"

// ------------------------------------------------------------------------- //
//                           Frontend Visitor                                //
// ------------------------------------------------------------------------- //
class SingleFunctionSourceVisitor
        : public clang::RecursiveASTVisitor<SingleFunctionSourceVisitor> {
public:
    explicit SingleFunctionSourceVisitor(clang::ASTContext*, FunctionEntity&);

    bool VisitFunctionDecl(clang::FunctionDecl*);

private:
    clang::ASTContext* context_;
    FunctionEntity&    function_entity_;
};

// ------------------------------------------------------------------------- //
//                            Frontend Consumer                              //
// ------------------------------------------------------------------------- //
class SingleFunctionSourceConsumer : public clang::ASTConsumer {
public:
    explicit SingleFunctionSourceConsumer(clang::ASTContext*, FunctionEntity&);

    void HandleTranslationUnit(clang::ASTContext&) override;

private:
    SingleFunctionSourceVisitor visitor_;
};

// ------------------------------------------------------------------------- //
//                             Frontend Action                               //
// ------------------------------------------------------------------------- //
class SingleFunctionSourceParser : public clang::ASTFrontendAction {
public:
    explicit SingleFunctionSourceParser(FunctionEntity&);

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
                                            clang::CompilerInstance&,
                                            llvm::StringRef
                                            ) override;

private:
    FunctionEntity& function_entity_;
};

#endif //AUTOFUZZ_SINGLE_FUNCTION_SOURCE_PARSER_H