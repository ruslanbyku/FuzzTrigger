#include "pass_launcher.h"

PassLauncher::PassLauncher(std::string ir_module)
: ir_module_(std::move(ir_module)) {}

bool PassLauncher::LaunchAnalysis(std::unique_ptr<Module>& module_dump) {
    llvm::SMDiagnostic error;
    llvm::LLVMContext context;

    // Load IR text representation into memory
    std::unique_ptr<llvm::Module>
            module(llvm::parseIRFile(ir_module_, error, context));
    if (!module) {
        return false;
    }

    // Tell the pass that analysis operations (CallGraph) will be done further
    llvm::PassRegistry* passReg = llvm::PassRegistry::getPassRegistry();
    llvm::initializeAnalysis(*passReg);

    // Register the pass and run it;
    llvm::legacy::PassManager pass_manager;
    pass_manager.add(new Analysis(module_dump));
    pass_manager.run(*module);

    return true;
}

bool PassLauncher::LaunchSanitizer(
        const std::unique_ptr<Function>& function_dump) {
    llvm::SMDiagnostic error;
    llvm::LLVMContext context;

    // Load IR text representation into memory
    std::unique_ptr<llvm::Module>
            module(llvm::parseIRFile(ir_module_, error, context));
    if (!module) {
        return false;
    }

    // Register the pass manager to run a pass
    llvm::legacy::PassManager pass_manager;
    bool status = false;

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Deep sanitization launched.";
    }

    // Run the sanitizer pass (first run is a deep sanitization)
    pass_manager.add(new Sanitizer(function_dump, status));
    pass_manager.run(*module);

    // Check status of the sanitizer pass
    // Relaunch the pass with GlobalVariable sanitization off (deep = false)
    if (!status) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_WARNING) << "Deep sanitization for "
                                   << function_dump->name_ << " failed.";
            LOG(LOG_LEVEL_INFO) << "Launch shallow sanitization for "
                                << function_dump->name_ << ".";
        }

        pass_manager.add(new Sanitizer(function_dump, status, false));
        pass_manager.run(*module);
    }

    return status;
}

bool PassLauncher::LaunchNameCorrector(
        const std::unique_ptr<Function>& function_dump) {
    llvm::SMDiagnostic error;
    llvm::LLVMContext context;

    // Load IR text representation into memory
    std::unique_ptr<llvm::Module>
            module(llvm::parseIRFile(ir_module_, error, context));
    if (!module) {
        return false;
    }

    // Register the pass manager to run a pass
    llvm::legacy::PassManager pass_manager;
    bool status = false;

    // Run the name mangling pass
    pass_manager.add(new NameCorrector(function_dump, status));
    pass_manager.run(*module);

    return status;
}
