
# C++ 命名规范（基于 Google Style)

## 一、项目级命名约定

| 项目元素 | 命名方式 | 示例 | 说明 |
|-----------|-----------|------|------|
| 文件名 | snake_case | `user_service.cpp`, `config_loader.hpp` | 文件名一律小写，用 `_` 分隔 |
| 目录名 | snake_case | `dao/`, `service/`, `util/` | 目录统一小写，下划线分词 |
| 命名空间 | snake_case | `namespace my_project::service {}` | 避免首字母大写，命名空间不混用驼峰 |

---

## 二、类型命名（Class / Struct / Enum）

| 类型 | 命名方式 | 示例 |
|------|-----------|------|
| 类名（Class） | PascalCase | `class UserService {};` |
| 结构体（Struct） | PascalCase | `struct UserInfo {};` |
| 枚举类型（Enum） | PascalCase | `enum class StatusCode {};` |
| 类型别名（using/typedef） | PascalCase | `using UserList = std::vector<User>;` |


---

## 三、函数命名

| 作用域 | 命名方式 | 示例 | 说明 |
|--------|-----------|------|------|
| Public / Protected 函数 | lowerCamelCase | `bool connectDb();` | 对外接口使用小驼峰 |
| Private 成员函数 | snake_case | `void init_connection();` | 内部函数采用下划线，便于区分 |
| 自由函数（非类成员） | lowerCamelCase | `parseJsonFile();` | 与 public 成员保持一致 |
| 虚函数 / 重载函数 | 保持原风格 | `void onEvent() override;` | 与基类保持一致即可 |

---

## 四、变量命名

| 变量类型 | 命名方式 | 示例 | 说明 |
|-----------|-----------|------|------|
| 局部变量 | snake_case | `int retry_count;` | 局部范围清晰 |
| 函数参数 | snake_case | `void connect(const std::string& db_name);` | 一律小写下划线 |
| 类成员变量 | snake_case + `_` 后缀 | `int retry_count_;` | `_` 表示类成员 |
| 静态成员变量 | snake_case | `static int active_sessions;` | 无需 `_`，避免冲突 |
| 全局变量（不推荐） | g + PascalCase | `gLogger` | 如果必须使用，带前缀 |
| 常量 / 枚举值 | `k` + PascalCase | `const int kMaxRetryCount = 5;` | 所有常量都加 `k` 前缀 |

---

## 五、模板、宏与类型参数

| 类型 | 命名方式 | 示例 |
|------|-----------|------|
| 模板参数 | PascalCase 单字母或语义化 | `template <typename T>`, `template <typename KeyType>` |
| 宏定义 | 全大写 + `_` | `#define MAX_BUFFER_SIZE 1024` |
| constexpr 常量 | `k` + PascalCase | `constexpr size_t kMaxBuffer = 512;` |

---

## 六、命名示例

```cpp
namespace my_project::service {

class UserService {
 public:
  bool connectDb(const std::string& db_name);
  bool queryUser(int user_id);
  void closeConnection();

 private:
  void init_connection();
  bool check_credentials();

 private:
  std::string db_name_;
  int retry_count_ = 0;
  static int active_sessions;

  static constexpr int kMaxRetryCount = 3;
};

}  // namespace my_project::service
```

---
