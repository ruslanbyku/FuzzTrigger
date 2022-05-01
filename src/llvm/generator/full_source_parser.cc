#include "full_source_parser.h"

// ------------------------------------------------------------------------- //
//                           Frontend Visitor                                //
// ------------------------------------------------------------------------- //
FullSourceVisitor::FullSourceVisitor(
        clang::ASTContext* context,
        const StandaloneFunctions& standalone_functions,
        SourceEntity& source_entity)
: context_(context), standalone_functions_(standalone_functions),
source_entity_(source_entity) {}

bool FullSourceVisitor::VisitFunctionDecl(clang::FunctionDecl* declaration) {
    std::string function_name = declaration->getNameAsString();

    //printf("%s %s\n", IsLocal(declaration) ? "1" : "0", function_name.c_str());

    if (!IsLocal(declaration)) {
        // A function is not local (from header files)
        // Proceed the traversal of the AST
        return true;
    }

    if (!IsStandalone(function_name)) {
        // Not a standalone function
        // Proceed the traversal of the AST
        return true;
    }

    if (IsRegistered(function_name)) {
        // Found declaration for a function more than one time
        // Something is wrong, but still do not add the same function entity
        // object to the storage
        // Proceed the traversal of the AST
        return true;
    }

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

bool FullSourceVisitor::IsLocal(clang::FunctionDecl* declaration) const {
    clang::SourceManager& manager = context_->getSourceManager();

    return manager.isInMainFile(declaration->getLocation());
}

bool FullSourceVisitor::IsStandalone(const std::string& function_name) const {
    return std::find_if(standalone_functions_.begin(),
                        standalone_functions_.end(),
                        [function_name](const auto& function_dump) {
                            return function_dump->name_ == function_name;
                        }) != standalone_functions_.end();
}

bool FullSourceVisitor::IsRegistered(const std::string& function_name) const {
    return std::find_if(source_entity_.begin(),
                        source_entity_.end(),
                        [function_name](const auto& function_entity) {
                            return function_entity.name_ == function_name;
                        }) != source_entity_.end();
}

// ------------------------------------------------------------------------- //
//                            Frontend Consumer                              //
// ------------------------------------------------------------------------- //
FullSourceConsumer::FullSourceConsumer(
        clang::ASTContext* context,
        const StandaloneFunctions& standalone_functions,
        SourceEntity& source_entity)
: visitor_(context, standalone_functions, source_entity) {}

void FullSourceConsumer::HandleTranslationUnit(clang::ASTContext& context) {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
}

// ------------------------------------------------------------------------- //
//                             Frontend Action                               //
// ------------------------------------------------------------------------- //
FullSourceParser::FullSourceParser(
        const StandaloneFunctions& standalone_functions,
        SourceEntity& source_entity)
: clang::ASTFrontendAction(),
standalone_functions_(standalone_functions),
source_entity_(source_entity) {}

std::unique_ptr<clang::ASTConsumer>
        FullSourceParser::CreateASTConsumer(
                clang::CompilerInstance& compiler,
                [[maybe_unused]] llvm::StringRef input_file) {

    // Ignore all errors/warnings during parsing of a source file
    compiler.getDiagnostics().setSuppressAllDiagnostics(true);

    // Delete unnecessary information ?
    //compiler.getPreprocessor().SetCommentRetentionState(true, true);

    return std::make_unique<FullSourceConsumer>(
            &compiler.getASTContext(),
            standalone_functions_,
            source_entity_);
}
