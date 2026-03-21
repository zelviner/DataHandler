#pragma once

#include <memory>
#include <qstring>
#include <vector>

struct ScriptInfo {
    std::string              person_path;     // 预个人化脚本路径
    std::string              person_filename; // 预个人化脚本文件名
    std::string              person_buffer;   // 预个人化脚本
    std::vector<std::string> bare_atrs;       // 裸卡ATR列表

    std::string              post_person_path;     // 后个人化脚本路径
    std::string              post_person_filename; // 后个人化脚本文件名
    std::string              post_person_buffer;   // 后个人化脚本
    std::vector<std::string> white_atrs;           // 白卡ATR列表

    std::string              check_path;     // 检测脚本路径
    std::string              check_filename; // 检测脚本文件名
    std::string              check_buffer;   // 检测脚本
    std::vector<std::string> finished_atrs;  // 成卡ATR列表

    std::string              clear_path;     // 清卡脚本路径
    std::string              clear_filename; // 清卡脚本文件名
    std::string              clear_buffer;   // 清卡脚本
    std::vector<std::string> clear_atrs;     // 清卡ATR列表

    std::string aka_auth_path;     // 鉴权脚本路径
    std::string aka_auth_filename; // 鉴权脚本文件名

    std::string auto_person_path;     // 自动个人化脚本路径
    std::string auto_person_filename; // 自动个人化脚本文件名
    std::string auto_person_buffer;   // 自动预个人化脚本

    std::string auto_post_person_path;     // 自动后个人化脚本路径
    std::string auto_post_person_filename; // 自动后个人化脚本文件名
    std::string auto_post_person_buffer;   // 自动后个人化脚本

    bool has_ds; // 是否有ds标识符
};

class Script {
  public:
    Script(const std::string &script_path);
    ~Script();

    std::shared_ptr<ScriptInfo> scriptInfo();

    bool autoPersonScript();
    bool autoPostPersonScript();

  private:
    std::string trim_script(const std::string &str);

  private:
    std::shared_ptr<ScriptInfo> script_info_;
    std::string                 script_path_;
};