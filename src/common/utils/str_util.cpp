#include "str_util.h"

#include <sstream>
#include <stdlib.h>

namespace {
const std::string s_blank = "";
}

namespace util {
namespace str {

const char* Blank() {
    return s_blank.c_str();
}

const std::string& BlankStr() {
    return s_blank;
}

std::string int_to_str(int64_t val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}

int str_to_int(const char* str) {
    return atoi(str);
}

}
}
