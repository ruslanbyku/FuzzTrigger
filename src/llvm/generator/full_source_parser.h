#ifndef AUTOFUZZ_FULL_SOURCE_PARSER_H
#define AUTOFUZZ_FULL_SOURCE_PARSER_H

#include "function_declaration.h"

using SourceEntity = std::vector<FunctionEntity>;

// ------------------------------------------------------------------------- //
//                           Frontend Visitor                                //
// ------------------------------------------------------------------------- //
class FullSourceVisitor : public clang::RecursiveASTVisitor<FullSourceVisitor> {
public:
    explicit FullSourceVisitor(clang::ASTContext*, SourceEntity&);

    bool VisitFunctionDecl(clang::FunctionDecl*);

private:
    clang::ASTContext* context_;
    SourceEntity&      source_entity_;
};

// ------------------------------------------------------------------------- //
//                            Frontend Consumer                              //
// ------------------------------------------------------------------------- //
class FullSourceConsumer : public clang::ASTConsumer {
public:
    explicit FullSourceConsumer(clang::ASTContext*, SourceEntity&);

    void HandleTranslationUnit(clang::ASTContext&) override;

private:
    FullSourceVisitor visitor_;
};

// ------------------------------------------------------------------------- //
//                             Frontend Action                               //
// ------------------------------------------------------------------------- //
class FullSourceParser : public clang::ASTFrontendAction {
public:
    explicit FullSourceParser(SourceEntity&);

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
                                            clang::CompilerInstance&,
                                            llvm::StringRef
                                            ) override;

private:
    SourceEntity& source_entity_;
};

#endif //AUTOFUZZ_FULL_SOURCE_PARSER_H
