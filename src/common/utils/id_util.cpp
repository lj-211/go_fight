#include "id_util.h"

namespace util {
namespace id {
std::string gen_uuid() {
    uuid_t uuid;
    char str[36];
    std::string uuid_str;

    uuid_generate(uuid);
    uuid_unparse(uuid, str);

    uuid_str = str;

    return uuid_str;
}
}
}