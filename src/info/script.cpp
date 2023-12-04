#include "script.h"

#include "public/filesystem/directory.h"
using namespace zel::filesystem;

#include "public/utility/string.h"
using namespace zel::utility;

Script::Script(std::string script_path)
    : script_info_(nullptr)
    , script_path_(script_path) {}

Script::~Script() {}

ScriptInfo *Script::scriptInfo(QString &error) {
    script_info_ = new ScriptInfo();
    Directory script_dir(script_path_);

    if (!script_dir.exists()) {
        error = "script dir not exists";
        return nullptr;
    }

    auto files = script_dir.files();

    for (auto file : files) {
        if (file.name().find("ClearCard") != std::string::npos || file.name().find("Restore") != std::string::npos) {
            if (file.exists()) {
                script_info_->clear_buffer   = file.read();
                script_info_->clear_filename = QString::fromStdString(file.name());
            } else {
                error = "open Clear Card Script failed";
                return nullptr;
            }
        } else if (file.name().find("Verify") != std::string::npos || file.name().find("Check") != std::string::npos) {
            if (file.exists()) {
                script_info_->check_buffer   = file.read();
                script_info_->check_filename = QString::fromStdString(file.name());
            } else {
                error = "open Check Script failed";
                return nullptr;
            }
        } else if (file.name().find("PostPerso") != std::string::npos) {
            if (file.exists()) {
                script_info_->post_person_buffer   = file.read();
                script_info_->post_person_filename = QString::fromStdString(file.name());
                script_info_->has_ds = script_info_->post_person_buffer.find("ds.") == std::string::npos ? false : true;

            } else {
                error = "open perso script failed";
                return nullptr;
            }
        } else {
            if (file.exists()) {
                script_info_->person_buffer   = file.read();
                script_info_->person_filename = QString::fromStdString(file.name());
            } else {
                error = "open clear script failed";
                return nullptr;
            }
        }
    }
    return script_info_;
}