#include "script.h"

#include <memory>
#include <zel/zel.h>

using namespace zel::filesystem;

Script::Script(const std::string &script_path)
    : script_info_(nullptr)
    , script_path_(script_path) {}

Script::~Script() {}

std::shared_ptr<ScriptInfo> Script::scriptInfo() {
    script_info_ = std::make_shared<ScriptInfo>();
    Directory script_dir(script_path_);

    if (!script_dir.exists()) {
        return nullptr;
    }

    auto files = script_dir.files();

    for (auto &file : *files) {
        if (file.name().find("ClearCard") != std::string::npos || file.name().find("Restore") != std::string::npos) {
            // 请卡脚本
            if (file.exists()) {
                script_info_->clear_buffer   = file.read();
                script_info_->clear_filename = file.name();
            } else {
                return nullptr;
            }
        } else if (file.name().find("Verify") != std::string::npos || file.name().find("Check") != std::string::npos) {
            // 检测脚本
            if (file.exists()) {
                script_info_->check_buffer   = file.read();
                script_info_->check_filename = file.name();
            } else {
                return nullptr;
            }
        } else if (file.name().find("PostPerso") != std::string::npos) {
            // 后个人化脚本
            if (file.exists()) {
                script_info_->post_person_buffer   = file.read();
                script_info_->post_person_filename = file.name();
                script_info_->has_ds               = script_info_->post_person_buffer.find("ds.") == std::string::npos ? false : true;
            } else {
                return nullptr;
            }
        } else {
            // 预个人化脚本
            if (file.exists()) {
                script_info_->person_buffer   = file.read();
                script_info_->person_filename = file.name();
            } else {
                return nullptr;
            }
        }
    }

    return script_info_;
}