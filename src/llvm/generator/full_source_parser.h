#ifndef AUTOFUZZ_FULL_SOURCE_PARSER_H
#define AUTOFUZZ_FULL_SOURCE_PARSER_H

#include "module.h"
#include "function_declaration.h"

#include <algorithm>

// ------------------------------------------------------------------------- //
//                           Frontend Visitor                                //
// ------------------------------------------------------------------------- //
class FullSourceVisitor : public clang::RecursiveASTVisitor<FullSourceVisitor> {
public:
    explicit FullSourceVisitor(clang::ASTContext*,
                               const StandaloneFunctions&,
                                       SourceEntity&);

    bool VisitFunctionDecl(clang::FunctionDecl*);

private:
    clang::ASTContext*         context_;
    const StandaloneFunctions& standalone_functions_;
    SourceEntity&              source_entity_;

    bool IsLocal(clang::FunctionDecl*) const;
    bool IsStandalone(const std::string&) const;
    bool IsRegistered(const std::string&) const;
};

// ------------------------------------------------------------------------- //
//                            Frontend Consumer                              //
// ------------------------------------------------------------------------- //
class FullSourceConsumer : public clang::ASTConsumer {
public:
    explicit FullSourceConsumer(clang::ASTContext*,
                                const StandaloneFunctions&,
                                        SourceEntity&);

    void HandleTranslationUnit(clang::ASTContext&) override;

private:
    FullSourceVisitor visitor_;
};

// ------------------------------------------------------------------------- //
//                             Frontend Action                               //
// ------------------------------------------------------------------------- //
class FullSourceParser : public clang::ASTFrontendAction {
public:
    explicit FullSourceParser(const StandaloneFunctions&, SourceEntity&);

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
                                            clang::CompilerInstance&,
                                            llvm::StringRef
                                            ) override;

private:
    const StandaloneFunctions& standalone_functions_;
    SourceEntity&              source_entity_;
};

#endif //AUTOFUZZ_FULL_SOURCE_PARSER_H
