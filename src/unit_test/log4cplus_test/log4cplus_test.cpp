#include "logger/logger.h"

bool test_log4cplus() {
    if (logger::init_logger("./config/log.conf") == false) {
        return false;
    }

    DEBUG_LOG("%s", "this is debug log!");
    ERROR_LOG("%s", "this is error log!");
    INFO_LOG("%s", "this is info log!");
    TRACE_LOG("%s", "this is trace log!");

    return true;
}
