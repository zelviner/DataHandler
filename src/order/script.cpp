#include "script.h"

#include <memory>

using namespace zel::file_system;

const std::vector<Script::Rule> Script::rules_ = {
    {{"ClearCard", "Restore", "reboot"}, Script::Type::CLEAR},
    {{"Verify", "Check"}, Script::Type::CHECK},
    {{"PostPerso"}, Script::Type::POST_PERSO},
    {{"Perso"}, Script::Type::PERSO},
};

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
        if (file.extension() != ".txt" || file.name().find("Auto_") != std::string::npos) continue;

        auto type = match_type(file.name());
        if (!type.has_value()) {
            if (file.name().length() > 30) {
                if (!process_file(file, script_info_->bare_atrs, script_info_->person_buffer, script_info_->person_filename, script_info_->person_path))
                    return nullptr;

                script_info_->auto_person_filename = "Auto_" + file.name();
                script_info_->auto_person_path     = file.dirPath() + "/" + script_info_->auto_person_filename;
            }
            continue;
        }

        switch (*type) {
        case Type::CLEAR:
            if (!process_file(file, script_info_->clear_atrs, script_info_->clear_buffer, script_info_->clear_filename, script_info_->clear_path))
                return nullptr;
            break;

        case Type::CHECK:
            if (!process_file(file, script_info_->finished_atrs, script_info_->check_buffer, script_info_->check_filename, script_info_->check_path))
                return nullptr;
            break;

        case Type::POST_PERSO:
            if (!process_file(file, script_info_->white_atrs, script_info_->post_person_buffer, script_info_->post_person_filename,
                              script_info_->post_person_path))
                return nullptr;

            script_info_->has_ds = script_info_->post_person_buffer.find("ds.") != std::string::npos;

            script_info_->auto_post_person_filename = "Auto_" + file.name();
            script_info_->auto_post_person_path     = file.dirPath() + "/" + script_info_->auto_post_person_filename;
            break;

        case Type::PERSO:
            if (!process_file(file, script_info_->bare_atrs, script_info_->person_buffer, script_info_->person_filename, script_info_->person_path))
                return nullptr;

            script_info_->auto_person_filename = "Auto_" + file.name();
            script_info_->auto_person_path     = file.dirPath() + "/" + script_info_->auto_person_filename;
            break;
        }
    }

    script_info_->aka_auth_filename = "auth.if";
    script_info_->aka_auth_path     = "./scripts/" + script_info_->aka_auth_filename;

    return script_info_;
}

bool Script::autoPersonScript() {
    zel::file_system::File auto_person(script_info_->auto_person_path);

    if (auto_person.exists()) {
        return true;
    }

    auto clear_script = zel::utility::String::replace(script_info_->clear_buffer, "[ds.SYSPIN]", "0102030405060708");

    auto_person.create();
    script_info_->auto_person_buffer += ";Clear\nRST([atr])\n";

    // 清卡
    for (auto &atr : script_info_->clear_atrs) {
        script_info_->auto_person_buffer += "if ([atr] == " + atr + ") {\n";
        script_info_->auto_person_buffer += clear_script;
        script_info_->auto_person_buffer += "\n}\n";
    }

    // 预个人化
    script_info_->auto_person_buffer += "\n;Perso\n";
    script_info_->auto_person_buffer += script_info_->person_buffer;

    auto_person.write(script_info_->auto_person_buffer);
    script_info_->auto_person_buffer.clear();

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
    for (auto &atr : script_info_->clear_atrs) {
        if (atr == script_info_->finished_atrs[0]) {
            script_info_->auto_post_person_buffer += "if ([atr] == " + atr + ") {\n";
            script_info_->auto_post_person_buffer += script_info_->clear_buffer;
            script_info_->auto_post_person_buffer += "\n}\n";
        } else {
            script_info_->auto_post_person_buffer += "if ([atr] == " + atr + ") {\n";
            script_info_->auto_post_person_buffer +=
                "A0A40000023F00(0000)\nA0A40000022FE2(0000)\nA0B000000A([iccid]9000)\nif ([iccid] != FFFFFFFFFFFFFFFFFFFF) {\n";
            script_info_->auto_post_person_buffer += script_info_->clear_buffer;
            script_info_->auto_post_person_buffer += "\n}\n}\n";
        }
    }

    // 预个人化
    script_info_->auto_post_person_buffer += "\n;Perso\n";
    for (auto &atr : script_info_->bare_atrs) {
        script_info_->auto_post_person_buffer += "RST([atr])\nif ([atr] == " + atr + ") {\n";
        script_info_->auto_post_person_buffer += script_info_->person_buffer;
        script_info_->auto_post_person_buffer += "\n}\n";
    }

    // 后个人化
    script_info_->auto_post_person_buffer += "\n\n;PostPerso\n" + script_info_->post_person_buffer;

    auto_post_person.write(script_info_->auto_post_person_buffer);
    script_info_->auto_post_person_buffer.clear();
    return true;
}

std::string Script::trim_script(const std::string &str) {
    std::string result = str;

    size_t pos = result.find_last_of(")");
    return result.substr(0, pos + 1);
}

bool Script::process_file(File &file, std::vector<std::string> &atrs, std::string &buffer, std::string &filename, std::string &path) {
    if (!file.exists()) return false;

    std::string line;
    file.readLine(line);

    auto matches = zel::utility::String::matches(line, R"(RST\(([^)]*)\))");
    if (!matches.empty()) {
        atrs = zel::utility::String::split(matches[0], ",");
    }

    buffer   = trim_script(file.read());
    filename = file.name();
    path     = file.path();

    return true;
}

std::optional<Script::Type> Script::match_type(const std::string &name) {
    for (const auto &rule : rules_) {
        for (const auto &kw : rule.keywords) {
            if (name.find(kw) != std::string::npos) {
                return rule.type;
            }
        }
    }
    return std::nullopt;
}