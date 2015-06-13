#include "file_util.h"

#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>

namespace util {
namespace file {

bool is_file_exist(const std::string& path) {
	struct stat buf;
    if (stat(path.c_str(), &buf) == ENOENT) {
		return false;
	}

	return true;
}

}
};
