/*
 * Copyright 2022 Ruslan Byku
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FUZZTRIGGER_PASS_LAUNCHER_H
#define FUZZTRIGGER_PASS_LAUNCHER_H

#include "analysis.h"
#include "sanitizer.h"
#include "name_corrector.h"
#include "logger.h"

#include <exception>

#include <llvm/PassRegistry.h>
#include <llvm/InitializePasses.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

class PassLauncher {
public:
    explicit PassLauncher(std::string ir_module)
                                     : ir_module_(std::move(ir_module)) {}

    bool LaunchAnalysis(std::unique_ptr<Module>&);

    template <typename T>
    bool LaunchOnFunction(const std::shared_ptr<Function>&);

private:
    std::string ir_module_;
};

inline bool PassLauncher::LaunchAnalysis(
                               std::unique_ptr<Module>& module_dump) {
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

    // Register the pass manager to run a pass
    llvm::legacy::PassManager pass_manager;

    // Run the pass
    pass_manager.add(new Analysis(module_dump));
    pass_manager.run(*module);

    return true;
}

template<typename T>
bool PassLauncher::LaunchOnFunction(
                          const std::shared_ptr<Function>& function_dump) {
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
    bool operation_status = false;

    // Run the pass
    pass_manager.add(new T(function_dump, operation_status));
    pass_manager.run(*module);

    return operation_status;
}

#endif //FUZZTRIGGER_PASS_LAUNCHER_H
