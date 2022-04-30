#include "function_declaration.h"

namespace FunctionDeclaration {
    std::string Extract(clang::ASTContext* context,
                            clang::FunctionDecl* declaration) {
        std::string function_declaration;

        // Get the function's location in the file
        const clang::SourceLocation start_location =
                declaration->getDefinition()->getSourceRange().getBegin();
        const clang::SourceLocation end_location   =
                declaration->getBody()->getSourceRange().getBegin();

        clang::SourceRange range(start_location, end_location);

        clang::SourceManager& manager = context->getSourceManager();

        llvm::StringRef code_snippet = clang::Lexer::getSourceText(
                clang::CharSourceRange::getTokenRange(range),
                manager,
                context->getLangOpts()
        );

        if (code_snippet.empty()) {
            // No function declaration was found
            return function_declaration;
        }

        // Function declaration was found, dump it
        function_declaration =
                code_snippet.substr(0, code_snippet.size() - 1).str();
        while (function_declaration.ends_with('\n')) {
            function_declaration.pop_back();
        }
        function_declaration += ";";

        return function_declaration;
    }
} // FunctionDeclaration