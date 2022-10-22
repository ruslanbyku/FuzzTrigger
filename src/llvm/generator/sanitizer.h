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

#ifndef FUZZTRIGGER_SANITIZER_H
#define FUZZTRIGGER_SANITIZER_H

#include "module.h"

#include <map>

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Operator.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Attributes.h>

using Visibility = llvm::GlobalValue::VisibilityTypes;
using Linkage    = llvm::GlobalValue::LinkageTypes;

class Sanitizer : public llvm::ModulePass {
public:
    static char ID;  // Declare for Pass internal operations

    explicit Sanitizer(const std::shared_ptr<Function>&, bool&);

    // Explicitly specify what kind of pass has to be done on the run.
    llvm::StringRef getPassName() const override;
    // Process the module on invocation
    bool runOnModule(llvm::Module&) override;

private:
    const std::shared_ptr<Function>& function_dump_;
    llvm::Function*                  target_function_;

    bool&                            success_;

    std::map<llvm::Constant*, bool>  visited_constants_;

    bool SanitizeModule(llvm::Module&);
    void AppendAddressSanitizer();

    bool IsDroppable(llvm::User*);
    bool IsMemberOfTargetFunction(const llvm::Instruction&);

    inline void UpdateIRModule(llvm::Module&);
    inline void Debug(llvm::Module&);
};


#endif //FUZZTRIGGER_SANITIZER_H
