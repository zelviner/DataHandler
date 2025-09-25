#pragma once

#include <memory>
#include <qstring>

struct ScriptInfo {
    std::string person_filename;      // 预个人化脚本文件名
    std::string post_person_filename; // 后个人化脚本文件名
    std::string check_filename;       // 检测脚本文件名
    std::string clear_filename;       // 清卡脚本文件名

    std::string person_buffer;      // 预个人化脚本
    std::string post_person_buffer; // 后个人化脚本
    std::string check_buffer;       // 检测脚本
    std::string clear_buffer;       // 清卡脚本

    bool has_ds; // 是否有ds标识符
};

class Script {
  public:
    Script(const std::string &script_path);
    ~Script();

    std::shared_ptr<ScriptInfo> scriptInfo();

  private:
    std::shared_ptr<ScriptInfo> script_info_;
    std::string                 script_path_;
};