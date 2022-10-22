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

#ifndef FUZZTRIGGER_LOGGER_H
#define FUZZTRIGGER_LOGGER_H

#include "utils.h"

#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_YELLOW "\033[33m"

#define LOGGER_ON      true
#define LOGGER_FILE_ON false  // If false, write to stdout

enum LogLevel : uint8_t {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
};

#define MAX_LOG_LEVEL_LENGTH 7
const char* const log_level_string[] = {
        "ERROR",
        "WARNING",
        "INFO",
        "DEBUG"
};

// Additional resources about logger:
// https://stackoverflow.com/a/5028917
//

// ------------------------------------------------------------------------- //
//                              Logger class                                 //
// ------------------------------------------------------------------------- //
template <typename T>
class Logger {
public:
    Logger()                          = default;
    virtual ~Logger();
    Logger(const Logger&)             = delete;
    Logger& operator =(const Logger&) = delete;

    std::ostringstream& Get(LogLevel);
    static std::string  ToString(LogLevel);

protected:
    std::ostringstream string_stream_;
};

template <typename T>
Logger<T>::~Logger() {
    string_stream_ << std::endl;
    T::Output(string_stream_.str());
}

template <typename T>
std::ostringstream& Logger<T>::Get(LogLevel level) {
    std::string log_level = ToString(level);
    uint8_t     offset    = MAX_LOG_LEVEL_LENGTH - log_level.length();
    std::string timestamp = Utils::GetTimeStamp();

    string_stream_ << "- ";

    if (!timestamp.empty()) {
        string_stream_ << timestamp << " ";
    }

    switch (level) {
        case LOG_LEVEL_ERROR:
            string_stream_ << COLOR_RED;
            break;
        case LOG_LEVEL_INFO:
            string_stream_ << COLOR_GREEN;
            break;
        case LOG_LEVEL_WARNING:
            string_stream_ << COLOR_YELLOW;
        default:
            break;
    }

    string_stream_ << log_level << COLOR_RESET << ": ";
    string_stream_ << std::string(offset, ' ');

    return string_stream_;
}

template <typename T>
std::string Logger<T>::ToString(LogLevel level) {
    return log_level_string[level];
}

// ------------------------------------------------------------------------- //
//                           FileHandler class                               //
// ------------------------------------------------------------------------- //

class FileHandler {
public:
    static FILE*& GetStream();
    static void   Output(const std::string&);
};

inline FILE*& FileHandler::GetStream() {
    static FILE* stream = stdout;

    return stream;
}

inline void FileHandler::Output(const std::string& message) {
    FILE* stream = GetStream();
    if (!stream) {
        return;
    }

    fprintf(stream, "%s", message.c_str());
    fflush(stream);
}

// ------------------------------------------------------------------------- //
//                             FileLogger class                              //
// ------------------------------------------------------------------------- //

class FileLogger : public Logger<FileHandler> {};

#define LOG_MAX_LEVEL LOG_LEVEL_DEBUG
#define LOG_MIN_LEVEL LOG_LEVEL_ERROR

#define LOG(level) \
if ((level) > LOG_MAX_LEVEL || (level) < LOG_MIN_LEVEL); \
else FileLogger().Get(level)


// ------------------------------------------------------------------------- //
//                            Auxiliary functions                            //
// ------------------------------------------------------------------------- //

inline bool InitLogger(const std::string& file) {
    FILE* file_stream = fopen(file.c_str(), "w");

    // Check if the file was successfully open
    if (!file_stream) {
        // Could not create the file
        return false;
    }

    // Save the file stream for further operation
    FileHandler::GetStream() = file_stream;

    return true;
}

inline void StartLogging() {
    // --------------------------------------------------------------------- //
    //                        No logging enabled                             //
    // --------------------------------------------------------------------- //
    if (!LOGGER_ON) {
        return;
    }

    // --------------------------------------------------------------------- //
    //                     Logging to stdout enabled                         //
    // --------------------------------------------------------------------- //
    if (!LOGGER_FILE_ON) {
        LOG(LOG_LEVEL_INFO) << "Initialize logging.";

        return;
    }

    // --------------------------------------------------------------------- //
    //                      Logging to a file enabled                        //
    // --------------------------------------------------------------------- //
    std::string program_path = Utils::GetCurrentProgramPath();
    if (program_path.empty()) {
        // No program path was found, write to stdout by default
        LOG(LOG_LEVEL_WARNING) << "Could not generate a log file. "
                                  "Current program name is invalid.";
        LOG(LOG_LEVEL_INFO) << "Initialize logging.";

        return;
    }

    std::string log_path;
    log_path += program_path;
    log_path += ".log";

    bool result = InitLogger(log_path);
    if (!result) {
        // Log file was not created, write to stdout by default
        LOG(LOG_LEVEL_WARNING) << "Could not create the log file "
                               << log_path << ".";

    }

    LOG(LOG_LEVEL_INFO) << "Initialize logging.";
}

inline void EndLogger() {
    FILE* file_stream = FileHandler::GetStream();

    if (file_stream) {
        // Do not close stdout
        if (file_stream != stdout) {
            fclose(FileHandler::GetStream());
        }
    }
}

inline void FinishLogging() {
    // Check if logging was turned on
    if (!LOGGER_ON) {
        return;
    }

    LOG(LOG_LEVEL_INFO) << "Finish logging.";
    EndLogger();
}

#endif //FUZZTRIGGER_LOGGER_H
