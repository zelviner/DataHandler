#include "script.h"

#include <memory>
#include <zel/core.h>

using namespace zel::file_system;

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

    auto files = script_dir.files(true);

    std::string line;
    for (auto &file : *files) {
        line.clear();

        if (file.name().find("ClearCard") != std::string::npos || file.name().find("Restore") != std::string::npos) {
            // 请卡脚本
            if (file.exists()) {
                file.readLine(line);
                auto matches = zel::utility::String::matches(line, R"(RST\(([^)]*)\))");
                if (matches.size() > 0) {
                    script_info_->clear_atrs = zel::utility::String::split(matches[0], ",");
                }
                script_info_->clear_buffer   = file.read();
                script_info_->clear_filename = file.name();
                script_info_->clear_path     = file.path();
            } else {
                return nullptr;
            }
        } else if (file.name().find("Verify") != std::string::npos || file.name().find("Check") != std::string::npos) {
            // 检测脚本
            if (file.exists()) {
                file.readLine(line);
                auto matches = zel::utility::String::matches(line, R"(RST\(([^)]*)\))");
                if (matches.size() > 0) {
                    script_info_->finished_atrs = zel::utility::String::split(matches[0], ",");
                }
                script_info_->check_buffer   = file.read();
                script_info_->check_filename = file.name();
                script_info_->check_path     = file.path();
            } else {
                return nullptr;
            }
        } else if (file.name().find("PostPerso") != std::string::npos) {
            // 后个人化脚本
            if (file.exists()) {
                file.readLine(line);
                auto matches = zel::utility::String::matches(line, R"(RST\(([^)]*)\))");
                if (matches.size() > 0) {
                    script_info_->white_atrs = zel::utility::String::split(matches[0], ",");
                }
                script_info_->post_person_buffer        = file.read();
                script_info_->post_person_filename      = file.name();
                script_info_->post_person_path          = file.path();
                script_info_->has_ds                    = script_info_->post_person_buffer.find("ds.") == std::string::npos ? false : true;
                script_info_->auto_post_person_filename = "Auto_" + file.name();
                script_info_->auto_post_person_path     = file.dirPath() + "/" + script_info_->auto_post_person_filename;
            } else {
                return nullptr;
            }
        } else if (file.name().find("Perso") != std::string::npos) {
            // 预个人化脚本
            if (file.exists()) {
                file.readLine(line);
                auto matches = zel::utility::String::matches(line, R"(RST\(([^)]*)\))");
                if (matches.size() > 0) {
                    script_info_->bare_atrs = zel::utility::String::split(matches[0], ",");
                }
                script_info_->person_buffer        = file.read();
                script_info_->person_filename      = file.name();
                script_info_->person_path          = file.path();
                script_info_->auto_person_filename = "Auto_" + file.name();
                script_info_->auto_person_path     = file.dirPath() + "/" + script_info_->auto_person_filename;
            } else {
                return nullptr;
            }
        }
    }

    script_info_->aka_auth_filename = "auth.script";
    script_info_->aka_auth_path     = "./" + script_info_->aka_auth_filename;

    return script_info_;
}

bool Script::autoPersonScript() {
    zel::file_system::File auto_person(script_info_->auto_person_path);

    if (auto_person.exists()) {
        return true;
    }

    auto clear_script = zel::utility::String::replace(script_info_->clear_buffer, "[ds.SYSPIN]", "0102030405060708");

    auto_person.create();
    script_info_->auto_post_person_buffer += ";Clear\nRST([atr])\n";

    // 清卡
    for (auto &atr : script_info_->clear_atrs) {
        script_info_->auto_post_person_buffer += "if ([atr] == " + atr + ") {\n";
        script_info_->auto_post_person_buffer += clear_script;
        script_info_->auto_post_person_buffer += "}\n";
    }

    // 预个人化
    script_info_->auto_post_person_buffer += "\n;Perso\n";
    script_info_->auto_post_person_buffer += script_info_->person_buffer;

    auto_person.write(script_info_->auto_post_person_buffer);

    return true;
}

bool Script::autoPostPersonScript() {
    zel::file_system::File auto_post_person(script_info_->auto_post_person_path);

    if (auto_post_person.exists()) {
        return true;
    }

    auto_post_person.create();
    script_info_->auto_post_person_buffer += "RST([atr])\n\n;Clear\n";

    // 清卡
    for (auto &atr : script_info_->finished_atrs) {
        script_info_->auto_post_person_buffer += "if ([atr] == " + atr + ") {\n";
        script_info_->auto_post_person_buffer += script_info_->clear_buffer;
        script_info_->auto_post_person_buffer += "}\n";
    }

    // 预个人化
    script_info_->auto_post_person_buffer += "\n;Perso\n";
    for (auto &atr : script_info_->bare_atrs) {
        script_info_->auto_post_person_buffer += "RST([atr])\nif ([atr] == " + atr + ") {\n";
        script_info_->auto_post_person_buffer += script_info_->person_buffer;
        script_info_->auto_post_person_buffer += "}\n";
    }

    // 后个人化
    script_info_->auto_post_person_buffer += "\n\n;PostPerso\n" + script_info_->post_person_buffer;

    auto_post_person.write(script_info_->auto_post_person_buffer);
    return true;
}