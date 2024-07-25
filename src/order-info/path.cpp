#include "path.h"
#include <vector>

Path::Path(std::string datagram)
    : datagram(datagram) {}

Path::~Path() {}

void Path::show() {
    printf("datagram_path: %s\n", datagram.c_str());
    printf("directory_path: %s\n", directory.c_str());
    printf("data_path: %s\n", data.c_str());
    printf("temp_path: %s\n", temp.c_str());
    printf("screenshot_path: %s\n", screenshot.c_str());
    printf("print_path: %s\n", print.c_str());
    printf("tag_data_path: %s\n", tag_data.c_str());
    printf("script_path: %s\n", script.c_str());
}
