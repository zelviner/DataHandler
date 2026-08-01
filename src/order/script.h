#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <zel/core.h>

struct ScriptInfo {
    std::string              person_path;     // 预个人化脚本路径
    std::string              person_filename; // 预个人化脚本文件名
    std::string              person_buffer;   // 预个人化脚本
    std::vector<std::string> bare_atrs;       // 裸卡ATR列表

    std::string              post_person_path;     // 后个人化脚本路径
    std::string              post_person_filename; // 后个人化脚本文件名
    std::string              post_person_buffer;   // 后个人化脚本
    std::vector<std::string> white_atrs;           // 预个人化完成ATR列表

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

    std::string auto_post_person_path;     // 自动后个人化脚本路径
    std::string auto_post_person_filename; // 自动后个人化脚本文件名

    bool has_ds; // 是否有ds标识符
};

class Script {
  public:
    enum class Type { CLEAR, CHECK, POST_PERSO, PERSO };
    struct Rule {
        std::vector<std::string> keywords;
        Type                     type;
    };

  public:
    Script(const std::string &script_path);

    std::shared_ptr<ScriptInfo> scriptInfo();

    bool autoPersonScript();
    bool autoPostPersonScript();

  private:
    std::string trim_script(const std::string &str);
    bool        process_file(zel::fs::File &file, std::vector<std::string> &atrs, std::string &buffer, std::string &filename, std::string &path);
    std::optional<Type> match_type(const std::string &name);

  private:
    std::shared_ptr<ScriptInfo>    script_info_;
    std::string                    script_path_;
    static const std::vector<Rule> rules_;
};
