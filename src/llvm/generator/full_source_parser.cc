#include "full_source_parser.h"

// ------------------------------------------------------------------------- //
//                           Frontend Visitor                                //
// ------------------------------------------------------------------------- //
FullSourceVisitor::FullSourceVisitor(
        clang::ASTContext* context,
        SourceEntity& source_entity)
        : context_(context), source_entity_(source_entity) {}

bool FullSourceVisitor::VisitFunctionDecl(clang::FunctionDecl* declaration) {
    std::string function_name = declaration->getNameAsString();
    std::string function_declaration;

    // Function declaration is found
    function_declaration = FunctionDeclaration::Extract(context_, declaration);

    // Save function data
    FunctionEntity function_entity(function_name);

    if (function_declaration.empty()) {
        // No function declaration was found
        function_entity.declaration_ = "";
    } else {
        // Function declaration was found, save it
        function_entity.declaration_ = function_declaration;
    }

    source_entity_.push_back(std::move(function_entity));

    // Proceed the traversal of the AST
    return true;
}

// ------------------------------------------------------------------------- //
//                            Frontend Consumer                              //
// ------------------------------------------------------------------------- //
FullSourceConsumer::FullSourceConsumer(
        clang::ASTContext* context,
        SourceEntity& source_entity
) : visitor_(context, source_entity) {}

void FullSourceConsumer::HandleTranslationUnit(clang::ASTContext& context) {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
}

// ------------------------------------------------------------------------- //
//                             Frontend Action                               //
// ------------------------------------------------------------------------- //
FullSourceParser::FullSourceParser(
        SourceEntity& source_entity)
        : clang::ASTFrontendAction(), source_entity_(source_entity) {}

std::unique_ptr<clang::ASTConsumer>
        FullSourceParser::CreateASTConsumer(
                clang::CompilerInstance& compiler,
                [[maybe_unused]] llvm::StringRef input_file) {

    // Ignore all errors/warnings during parsing of a source file
    compiler.getDiagnostics().setSuppressAllDiagnostics(true);

    return std::make_unique<FullSourceConsumer>(
            &compiler.getASTContext(),
            source_entity_);
}
