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
//TODO: In some files function declarations are in macros definitions.
//      The built-in preprocessor just hides all the unnecessary macros,
//      thereby deletes definitions for some standalone functions. For now
//      I do know how to turn this conduct off.
//      Example: in 'curl-7.81.0/lib/http2.c' the function
//      'drained_transfer' is not seen.
//      Also there are multiple similar functions that are hidden into
//      macros definitions. They are in the IR, but only the first
//      encountered version. On the other hand, the preprocessor only
//      sees the version that is appropriate to macro definition. It makes
//      some sort of ambiguity that prevents from proper analysis and
//      generation.
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
