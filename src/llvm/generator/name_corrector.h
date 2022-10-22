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

#ifndef FUZZTRIGGER_NAME_CORRECTOR_H
#define FUZZTRIGGER_NAME_CORRECTOR_H

#include "module.h"

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Verifier.h>

class NameCorrector : public llvm::ModulePass {
public:
    static char ID;  // Declare for Pass internal operations

    explicit NameCorrector(const std::shared_ptr<Function>&, bool&);

    // Explicitly specify what kind of pass has to be done on the run.
    llvm::StringRef getPassName() const override;
    // Process the module on invocation
    bool runOnModule(llvm::Module&) override;

private:
    const std::shared_ptr<Function>& function_dump_;

    bool&                            success_;

    bool DropManglingOnTargetFunction(llvm::Module&, const std::string&);

    inline void UpdateIRModule(llvm::Module&);
    inline void Debug(llvm::Module&);
};


#endif //FUZZTRIGGER_NAME_CORRECTOR_H
