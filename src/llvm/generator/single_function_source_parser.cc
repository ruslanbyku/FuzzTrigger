#include "single_function_source_parser.h"

// ------------------------------------------------------------------------- //
//                           Frontend Visitor                                //
// ------------------------------------------------------------------------- //
FrontendVisitor::FrontendVisitor(
        clang::ASTContext* context,
        FunctionEntity& function_entity)
        : context_(context), function_entity_(function_entity) {}

bool FrontendVisitor::VisitFunctionDecl(clang::FunctionDecl* declaration) {
    std::string function_name = declaration->getNameAsString();

    if (function_name != function_entity_.name_) {
        // Proceed the traversal of the AST
        return true;
    }

    //
    // The function is found in a source file
    //

    std::string function_declaration;

    function_declaration = FunctionDeclaration::Extract(context_, declaration);

    if (function_declaration.empty()) {
        // No function declaration was found
        function_entity_.declaration_ = "";
        function_entity_.is_set_      = false;
    } else {
        // Function declaration was found, save it
        function_entity_.declaration_ = function_declaration;
        function_entity_.is_set_      = true;
    }

    // Stop the traversal of the AST
    return false;
}

// ------------------------------------------------------------------------- //
//                            Frontend Consumer                              //
// ------------------------------------------------------------------------- //
FrontendConsumer::FrontendConsumer(
        clang::ASTContext* context,
        FunctionEntity& function_entity
) : visitor_(context, function_entity) {}

void FrontendConsumer::HandleTranslationUnit(clang::ASTContext& context) {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
}

// ------------------------------------------------------------------------- //
//                             Frontend Action                               //
// ------------------------------------------------------------------------- //
SingleFunctionSourceParser::SingleFunctionSourceParser(
        FunctionEntity& function_entity)
: clang::ASTFrontendAction(), function_entity_(function_entity) {}

std::unique_ptr<clang::ASTConsumer>
        SingleFunctionSourceParser::CreateASTConsumer(
                clang::CompilerInstance& compiler,
                [[maybe_unused]] llvm::StringRef input_file) {

    // Ignore all errors/warnings during parsing of a source file
    compiler.getDiagnostics().setSuppressAllDiagnostics(true);

    return std::make_unique<FrontendConsumer>(
            &compiler.getASTContext(),
            function_entity_
            );
}

