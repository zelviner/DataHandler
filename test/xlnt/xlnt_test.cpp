#include <gtest/gtest.h>
#include "xlnt/xlnt.hpp"
#include <fstream>

TEST(Xlnt, class) {

    try {
        // 打开一个现有的.xlsx文件
        xlnt::workbook wb;
        wb.load("./test/xlnt/MO011218 预个人化表格.xlsx");

        // 获取活动工作表
        xlnt::worksheet ws = wb.active_sheet();

        // 打印工作表名称
        std::cout << "Active sheet name: " << ws.title() << std::endl;

        // 遍历工作表中的每个单元格并打印它们的值
        for (auto row : ws.rows(false)) {
            for (auto cell : row) {
                std::cout << cell.to_string() << "\t";
            }
            std::cout << std::endl;
        }
    } catch (const xlnt::exception &e) {
        std::cerr << "xlnt::exception : " << e.what() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "std::exception : " << e.what() << std::endl;
    }
}

void replace_string_in_xlsx(const std::string &filename, const std::string &old_str, const std::string &new_str) {
    // 打开 Excel 文件
    xlnt::workbook workbook;
    workbook.load(filename);

    // 获取活动工作表
    xlnt::worksheet worksheet = workbook.active_sheet();

    // 遍历工作表中的所有单元格
    for (auto row : worksheet.rows()) {
        for (auto cell : row) {
            // 检查单元格是否包含字符串
            std::string cell_value = cell.to_string();

            // 替换字符串
            size_t pos = 0;
            while ((pos = cell_value.find(old_str, pos)) != std::string::npos) {
                cell_value.replace(pos, old_str.length(), new_str);
                pos += new_str.length();
            }
            // 更新单元格内容
            worksheet.cell(cell.reference()).value(cell_value);
        }
    }

    // 保存 Excel 文件
    workbook.save(filename);
}

TEST(Xlnt, replace) {

    replace_string_in_xlsx("./test/xlnt/MO011218 预个人化表格.xlsx", "MO011218", "TEST1111");

}