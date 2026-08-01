#include "script.h"

#include <algorithm>
#include <memory>

using namespace zel::fs;

namespace {

bool contains_atr(const std::vector<std::string> &atrs, const std::string &atr) {
    return std::find(atrs.begin(), atrs.end(), atr) != atrs.end();
}

std::string join_atrs(const std::vector<std::string> &atrs) {
    std::string result;
    for (const auto &atr : atrs) {
        if (!result.empty()) result += ",";
        result += atr;
    }
    return result;
}

void append_if_atr(std::string &script, const std::string &atr, const std::string &content) {
    script += "if ([atr] == " + atr + ") {\n";
    script += content;
    script += "\n}\n";
}

bool recreate_file(File &file) {
    if (file.exists()) {
        file.clear();
        return true;
    }
    return file.create();
}

} // namespace

const std::vector<Script::Rule> Script::rules_ = {
    {{"ClearCard", "Restore", "reboot"}, Script::Type::CLEAR},
    {{"Verify", "Check"}, Script::Type::CHECK},
    {{"PostPerso"}, Script::Type::POST_PERSO},
    {{"Perso"}, Script::Type::PERSO},
};

Script::Script(const std::string &script_path)
    : script_info_(nullptr)
    , script_path_(script_path) {}

std::shared_ptr<ScriptInfo> Script::scriptInfo() {
    script_info_ = std::make_shared<ScriptInfo>();
    Directory script_dir(script_path_);

    if (!script_dir.exists()) {
        return nullptr;
    }

    auto files = script_dir.files(true);

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
    if (script_info_ == nullptr || script_info_->bare_atrs.empty() || script_info_->white_atrs.empty() || script_info_->finished_atrs.empty() ||
        script_info_->clear_atrs.empty()) {
        return false;
    }

    zel::fs::File auto_person(script_info_->auto_person_path);
    auto clear_script = zel::utility::String::replace(script_info_->clear_buffer, "[ds.SYSPIN]", "0102030405060708");

    if (!recreate_file(auto_person)) return false;

    std::string script = ";Clear\nRST([atr])\n";

    // 预个人化机只清可用固定 SYSPIN 清卡的中间态。
    // 成卡不能使用固定 SYSPIN 清卡，必须在最终 ATR 校验处报错。
    for (auto &atr : script_info_->clear_atrs) {
        if (!contains_atr(script_info_->white_atrs, atr) && !contains_atr(script_info_->finished_atrs, atr)) {
            append_if_atr(script, atr, clear_script);
        }
    }

    script += "\n;Perso\nRST([atr])\n";
    for (auto &atr : script_info_->bare_atrs) {
        append_if_atr(script, atr, script_info_->person_buffer);
    }
    script += "\n;Verify\nRST(" + join_atrs(script_info_->white_atrs) + ")\n";

    return auto_person.write(script);
}

bool Script::autoPostPersonScript() {
    if (script_info_ == nullptr || script_info_->bare_atrs.empty() || script_info_->white_atrs.empty() || script_info_->finished_atrs.empty() ||
        script_info_->clear_atrs.empty()) {
        return false;
    }

    zel::fs::File auto_post_person(script_info_->auto_post_person_path);

    if (!recreate_file(auto_post_person)) return false;

    std::string script = "RST([atr])\n\n;Clear\n";

    // 预个人化完成卡直接后个人化；中间态和成卡先清卡后重做。
    for (auto &atr : script_info_->clear_atrs) {
        if (!contains_atr(script_info_->white_atrs, atr)) {
            append_if_atr(script, atr, script_info_->clear_buffer);
        }
    }

    script += "\n;Perso\nRST([atr])\n";
    for (auto &atr : script_info_->bare_atrs) {
        append_if_atr(script, atr, script_info_->person_buffer);
    }

    script += "\n;PostPerso\nRST([atr])\n";
    for (auto &atr : script_info_->white_atrs) {
        append_if_atr(script, atr, script_info_->post_person_buffer);
    }
    script += "\n;Verify\nRST(" + join_atrs(script_info_->finished_atrs) + ")\n";

    return auto_post_person.write(script);
}

std::string Script::trim_script(const std::string &str) {
    size_t pos = str.find_last_of(")");
    if (pos == std::string::npos) return {};
    return str.substr(0, pos + 1);
}

bool Script::process_file(File &file, std::vector<std::string> &atrs, std::string &buffer, std::string &filename, std::string &path) {
    if (!file.exists()) return false;

    buffer = trim_script(file.read());
    if (buffer.empty()) return false;

    auto line_end = buffer.find_first_of("\r\n");
    auto first_line = buffer.substr(0, line_end);
    auto matches = zel::utility::String::matches(first_line, R"(RST\(([^)]*)\))");
    if (matches.empty() || matches[0].find_first_of("[]") != std::string::npos) return false;

    atrs = zel::utility::String::split(matches[0], ",");
    if (atrs.empty()) return false;

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
