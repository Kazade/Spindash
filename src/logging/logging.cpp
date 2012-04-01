
#include "logging.h"

namespace logging {


void debug(const std::string& text, const std::string& file, int32_t line) {
    get_logger("/")->debug(text, file, line);
}

void info(const std::string& text, const std::string& file, int32_t line) {
    get_logger("/")->info(text, file, line);
}

void warn(const std::string& text, const std::string& file, int32_t line) {
    get_logger("/")->warn(text, file, line);
}

void error(const std::string& text, const std::string& file, int32_t line) {
    get_logger("/")->error(text, file, line);
}

static Logger root("/");
static std::map<std::string, Logger::ptr> loggers_;

Logger* get_logger(const std::string& name) {
    if(name.empty() || name == "/") {
        return &root;
    } else {
        if(loggers_.find(name) == loggers_.end()) {
            loggers_[name].reset(new Logger(name));
        }
        return loggers_[name].get();
    }
}

}
