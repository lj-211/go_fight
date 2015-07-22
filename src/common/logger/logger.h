#ifndef LOGGER_H
#define LOGGER_H

#include <log4cplus/loglevel.h>

#include <string>

namespace logger {
bool init_logger(const std::string& log_cfg);

void enable_debug_log(bool val);

void set_log_level(log4cplus::LogLevel lv);

// 项目日志规范
// 1.debug日志上线后会屏蔽,用于功能开发期调试(单独功能需要加独立调试日志开关)
// 2.error日志用于出现逻辑错误或者底层错误使用
// 3.info日志用于常规程序流程使用
// 4.trace用于定位关键点信息,上线后继续保留
void debug_log(const char* filename, const int fileline, const char* format,  ...);
void error_log(const char* filename, const int fileline, const char* format,  ...);
void info_log(const char* filename, const int fileline, const char* format,  ...);
void trace_log(const char* filename, const int fileline, const char* format,  ...);

};

#define DEBUG_LOG(format, ...) \
    logger::debug_log(__FILE__, __LINE__, format, __VA_ARGS__)

#define ERROR_LOG(format, ...) \
    logger::error_log(__FILE__, __LINE__, format, __VA_ARGS__)

#define INFO_LOG(format, ...) \
    logger::info_log(__FILE__, __LINE__, format, __VA_ARGS__)

#define TRACE_LOG(format, ...) \
    logger::trace_log(__FILE__, __LINE__, format, __VA_ARGS__)

#define DEBUG_LOG_NOARG(format) \
    logger::debug_log(__FILE__, __LINE__, format)

#define ERROR_LOG_NOARG(format) \
    logger::error_log(__FILE__, __LINE__, format)

#define INFO_LOG_NOARG(format) \
    logger::info_log(__FILE__, __LINE__, format)

#define TRACE_LOG_NOARG(format) \
    logger::trace_log(__FILE__, __LINE__, format)

#endif
