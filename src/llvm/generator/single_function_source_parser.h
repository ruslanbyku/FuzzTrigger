#ifndef AUTOFUZZ_SINGLE_FUNCTION_SOURCE_PARSER_H
#define AUTOFUZZ_SINGLE_FUNCTION_SOURCE_PARSER_H

#include "function_declaration.h"

struct FunctionEntity {
    explicit FunctionEntity(const std::string& name)
            : name_(name), is_set_(false) {}

    const std::string& name_;
    std::string        declaration_;    // There is no "\n" at the end
    bool               is_set_;
};

// ------------------------------------------------------------------------- //
//                           Frontend Visitor                                //
// ------------------------------------------------------------------------- //
class FrontendVisitor : public clang::RecursiveASTVisitor<FrontendVisitor> {
public:
    explicit FrontendVisitor(clang::ASTContext*, FunctionEntity&);

    bool VisitFunctionDecl(clang::FunctionDecl*);

private:
    clang::ASTContext* context_;
    FunctionEntity&    function_entity_;
};

// ------------------------------------------------------------------------- //
//                            Frontend Consumer                              //
// ------------------------------------------------------------------------- //
class FrontendConsumer : public clang::ASTConsumer {
public:
    explicit FrontendConsumer(clang::ASTContext*, FunctionEntity&);

    void HandleTranslationUnit(clang::ASTContext&) override;

private:
    FrontendVisitor visitor_;
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
