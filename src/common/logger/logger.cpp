#include "logger.h"

#include "utils/file_util.h"

// log4cplus
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/logger.h>

#include <stdarg.h>
#include <stdio.h>

#define DO_LOGGER(loglevel, filename, fileline, format, bufsize) \
    { \
        if (log_sys_init_ok && s_root_logger.isEnabledFor(loglevel)) \
        { \
            va_list args; \
            va_start(args, format); \
            char buf[bufsize] = {0}; \
            vsnprintf(buf, sizeof(buf), format, args); \
            va_end(args); \
            s_root_logger.forcedLog(loglevel, buf, filename, fileline); \
        } \
    }

namespace {
log4cplus::Logger s_root_logger;
const int s_log_max_length = 1024 * 4;
bool log_sys_init_ok = false;
}

namespace logger {
bool init_logger(const std::string& log_cfg) {
    do {
        if (util::file::is_file_exist(log_cfg.c_str()) == false) {
            break;
        }

        // 默认不开启DEBUG日志
        log4cplus::helpers::LogLog::getLogLog()->setInternalDebugging(false);
        log4cplus::PropertyConfigurator::doConfigure(log_cfg);
        log4cplus::Logger logger = log4cplus::Logger::getRoot();
        s_root_logger = log4cplus::Logger::getInstance("rootLogger");;
        log_sys_init_ok = true;
        
        return true;
    } while (false);

    return false;
}

void enable_debug_log(bool val) {
    log4cplus::helpers::LogLog::getLogLog()->setInternalDebugging(val);
}

void set_log_level(log4cplus::LogLevel lv) {
}

void debug_log(const char* filename, const int fileline, const char* format,  ...) {
    DO_LOGGER(log4cplus::DEBUG_LOG_LEVEL, filename, fileline, format, s_log_max_length);
}

void error_log(const char* filename, const int fileline, const char* format,  ...) {
    DO_LOGGER(log4cplus::ERROR_LOG_LEVEL, filename, fileline, format, s_log_max_length);
}

void info_log(const char* filename, const int fileline, const char* format,  ...) {
    DO_LOGGER(log4cplus::INFO_LOG_LEVEL, filename, fileline, format, s_log_max_length);
}

void trace_log(const char* filename, const int fileline, const char* format,  ...) {
    DO_LOGGER(log4cplus::TRACE_LOG_LEVEL, filename, fileline, format, s_log_max_length);
}

};
