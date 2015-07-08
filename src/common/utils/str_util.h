#ifndef STR_UTIL_H
#define STR_UTIL_H

#include <string>

namespace util {
namespace str {

const char* Blank();

const std::string& BlankStr();

std::string int_to_str(int val);

int str_to_int(const char* str);

}
}

#endif
