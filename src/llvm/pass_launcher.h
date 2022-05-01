#ifndef AUTOFUZZ_PASS_LAUNCHER_H
#define AUTOFUZZ_PASS_LAUNCHER_H

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
    explicit PassLauncher(std::string);

    bool LaunchAnalysis(std::unique_ptr<Module>&);
    bool LaunchSanitizer(const std::shared_ptr<Function>&);
    bool LaunchNameCorrector(const std::shared_ptr<Function>&);

private:
    std::string ir_module_;
};


#endif //AUTOFUZZ_PASS_LAUNCHER_H
