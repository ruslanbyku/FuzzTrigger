#include "project_wrapper.h"

ProjectWrapper::ProjectWrapper(std::string ir_project,
                               std::string sources, bool auto_deletion,
                               bool random_on, bool override) noexcept(false)
: ir_project_(std::move(ir_project)),
  sources_(std::move(sources)),
  working_directory_(ir_project_.GetParentPath()),
  module_dump_(std::make_unique<Module>()),
  auto_deletion_(auto_deletion),
  random_on_(random_on),
  override_(override) {
    InitializeState();
}

ProjectWrapper::~ProjectWrapper() {}

void ProjectWrapper::InitializeState() {
    if (!ir_project_.IsAbsolute()) {
        std::string exception_message;

        exception_message += "File path of ";
        exception_message += ir_project_.GetPath();
        exception_message += " is relative.";

        throw std::runtime_error(exception_message);
    }

    if (!ir_project_.Exists()) {
        std::string exception_message;

        exception_message += "File ";
        exception_message += ir_project_.GetPath();
        exception_message += " does not exist.";

        throw std::runtime_error(exception_message);
    }

    if (!sources_.IsAbsolute()) {
        std::string exception_message;

        exception_message += "File path of ";
        exception_message += sources_.GetPath();
        exception_message += " is relative.";

        throw std::runtime_error(exception_message);
    }

    if (!sources_.Exists()) {
        std::string exception_message;

        exception_message += "File ";
        exception_message += sources_.GetPath();
        exception_message += " does not exist.";

        throw std::runtime_error(exception_message);
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File " << ir_project_.GetPath() << " found.";
        LOG(LOG_LEVEL_INFO) << "File " << sources_.GetPath() << " found.";
        //
        //
        //
        LOG(LOG_LEVEL_INFO) << "Current working directory "
                            << working_directory_ << ".";
        //
        //
        //
        LOG(LOG_LEVEL_INFO) << "Check whether " << ir_project_.GetPath()
                            << " looks like an IR file.";
    }

    if (!Compiler::IsCompilable(ir_project_)) {
        std::string exception_message;

        exception_message += "File ";
        exception_message += ir_project_.GetPath();
        exception_message += " does look like an IR file.";

        throw std::runtime_error(exception_message);
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File " << ir_project_.GetPath()
                            << " looks like an IR file.";
        //
        //
        //
        LOG(LOG_LEVEL_INFO) << "Extract source paths from "
                            << sources_.GetPath() << ":";
    }

    // If a source path is not valid, proceed anyway.
    uint64_t lines = 0;
    std::ifstream sources_file(sources_.GetPath());
    std::string single_source_path;
    while (std::getline(sources_file, single_source_path)) {
        ++lines;

        // \n is deleted automatically
        File source(single_source_path);

        if (source.IsAbsolute() &&
            source.Exists() &&
            Compiler::IsCompilable(source)) {

            source_paths_.insert(single_source_path);

            if (LOGGER_ON) {
                LOG(LOG_LEVEL_INFO) << single_source_path << " found.";
            }
        } else {
            if (LOGGER_ON) {
                LOG(LOG_LEVEL_WARNING) << "Can not recognize "
                                       << single_source_path << ".";
            }
        }
    }

    if (source_paths_.empty()) {
        std::string exception_message;

        exception_message += "No source paths were found in ";
        exception_message += sources_.GetPath();
        exception_message += ".";

        throw std::runtime_error(exception_message);
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "[recognized files/total lines]: ["
                            << source_paths_.size()
                            << "/"
                            << lines
                            << "]";
        //
        //
        //
        std::string random_parameter   = random_on_      ? "true" : "false";
        std::string deletion_parameter = auto_deletion_  ? "true" : "false";
        std::string override_parameter = override_       ? "true" : "false";

        LOG(LOG_LEVEL_INFO) << "Additional parameters are used:";
        LOG(LOG_LEVEL_INFO) << "file name randomization = " << random_parameter;
        LOG(LOG_LEVEL_INFO) << "tmp file auto deletion  = "
                            << deletion_parameter;
        LOG(LOG_LEVEL_INFO) << "file override           = "
                            << override_parameter;
        //
        //
        //
        LOG(LOG_LEVEL_INFO) << "Environment configuration is done. "
                               "Ready to proceed.";
    }

}

bool ProjectWrapper::LaunchRoutine() {
    return true;
}

bool ProjectWrapper::PerformAnalysis() {
    return true;
}

bool ProjectWrapper::PerformGeneration(
        const std::unique_ptr<Function>& function_dump) {
    return true;
}
