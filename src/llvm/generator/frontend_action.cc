#include "frontend_action.h"

// ------------------------------------------------------------------------- //
//                           Frontend Visitor                                //
// ------------------------------------------------------------------------- //
FrontendVisitor::FrontendVisitor(
        clang::ASTContext* context,
        FunctionLocation& function_location)
        : context_(context), function_location_(function_location) {}

// https://stackoverflow.com/a/49710635
bool FrontendVisitor::VisitFunctionDecl(clang::FunctionDecl* declaration) {
    std::string function_name = declaration->getNameAsString();

    if (function_name != function_location_.name_) {
        // Proceed the traversal
        return true;
    }

    // The function is found
    // Get the function's location in the file
    const clang::SourceLocation start_location =
            declaration->getDefinition()->getSourceRange().getBegin();
    const clang::SourceLocation end_location   =
            declaration->getBody()->getSourceRange().getBegin();

    clang::SourceRange range(start_location, end_location);

    clang::SourceManager& manager = context_->getSourceManager();

    llvm::StringRef code_snippet = clang::Lexer::getSourceText(
            clang::CharSourceRange::getTokenRange(range),
            manager,
            context_->getLangOpts()
            );

    if (code_snippet.empty()) {
        // No function declaration was found
        function_location_.entity_    = "";
        function_location_.is_filled_ = false;
    } else {
        // Dump function declaration
        std::string function_declaration =
                          code_snippet.substr(0, code_snippet.size() - 1).str();
        while (function_declaration.ends_with('\n')) {
            function_declaration.pop_back();
        }
        function_declaration += ";";

        // Store function declaration
        function_location_.entity_    = function_declaration;
        function_location_.is_filled_ = true;
    }

    // Stop the traversal of the AST
    return false;
}

// ------------------------------------------------------------------------- //
//                            Frontend Consumer                              //
// ------------------------------------------------------------------------- //
FrontendConsumer::FrontendConsumer(
        clang::ASTContext* context,
        FunctionLocation& function_location
        ) : visitor_(context, function_location) {}

void FrontendConsumer::HandleTranslationUnit(clang::ASTContext& context) {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
}

// ------------------------------------------------------------------------- //
//                             Frontend Action                               //
// ------------------------------------------------------------------------- //
FrontendAction::FrontendAction(FunctionLocation& function_location)
: clang::ASTFrontendAction(), function_location_(function_location) {}

std::unique_ptr<clang::ASTConsumer>
        FrontendAction::CreateASTConsumer(
                clang::CompilerInstance& compiler,
                [[maybe_unused]] llvm::StringRef input_file) {

    // Ignore all errors/warnings
    compiler.getDiagnostics().setSuppressAllDiagnostics(true);

    return std::make_unique<FrontendConsumer>(
            &compiler.getASTContext(),
            function_location_
            );
}
