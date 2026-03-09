#include "order_parser.h"

#include <algorithm>

std::shared_ptr<OrderInfo> OrderParser::parse(const std::string &pdf) {
    auto info = std::make_shared<OrderInfo>();

    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    if (!ctx) return info;

    fz_register_document_handlers(ctx);

    std::vector<std::string> all_lines;

    fz_try(ctx) {
        fz_document *doc   = fz_open_document(ctx, pdf.c_str());
        int          pages = fz_count_pages(ctx, doc);

        for (int i = 0; i < pages; i++) {
            auto page = fz_load_page(ctx, doc, i);

            auto text = fz_new_stext_page(ctx, fz_bound_page(ctx, page));
            auto dev  = fz_new_stext_device(ctx, text, NULL);

            fz_run_page(ctx, page, dev, fz_identity, NULL);

            auto lines = extract_lines(text);
            all_lines.insert(all_lines.end(), lines.begin(), lines.end());

            fz_close_device(ctx, dev);
            fz_drop_device(ctx, dev);
            fz_drop_stext_page(ctx, text);
            fz_drop_page(ctx, page);
        }

        fz_drop_document(ctx, doc);
    }
    fz_catch(ctx) {}

    fz_drop_context(ctx);

    auto   clean = filter_noise(all_lines);
    size_t start = find_data_start(clean);

    if (start == std::string::npos) return info;

    info->order_number = clean[start];

    if (start + 1 < clean.size()) {
        std::string num = clean[start + 1];
        num.erase(std::remove(num.begin(), num.end(), ','), num.end());

        try {
            info->quantity = std::stoi(num);
        } catch (...) {
        }
    }

    if (start + 2 < clean.size()) info->project_name = clean[start + 2];

    if (start + 5 < clean.size()) info->product_type = clean[start + 5];

    if (start + 6 < clean.size()) info->rf_code = clean[start + 6];

    if (start + 7 < clean.size()) {
        info->script_package = clean[start + 7];

        if (start + 8 < clean.size() && clean[start + 8].find('_') != std::string::npos) info->script_package += "_" + clean[start + 8];
    }

    if (start + 10 < clean.size()) info->chip_model = clean[start + 10];

    return info;
}

std::vector<std::string> extract_lines(fz_stext_page *page) {
    std::vector<PdfChar> chars;

    for (auto block = page->first_block; block; block = block->next) {
        if (block->type != FZ_STEXT_BLOCK_TEXT) continue;

        for (auto line = block->u.t.first_line; line; line = line->next) {
            for (auto ch = line->first_char; ch; ch = ch->next) {
                char buf[8];
                int  len = fz_runetochar(buf, ch->c);

                PdfChar pc;
                pc.x = ch->quad.ul.x;
                pc.y = ch->quad.ul.y;
                pc.text.assign(buf, len);

                chars.push_back(pc);
            }
        }
    }

    // 按 Y 排序
    std::sort(chars.begin(), chars.end(), [](const PdfChar &a, const PdfChar &b) { return a.y > b.y; });

    std::vector<std::vector<PdfChar>> rows;

    const float threshold = 2.0f;

    for (auto &c : chars) {
        bool found = false;

        for (auto &row : rows) {
            if (fabs(row[0].y - c.y) < threshold) {
                row.push_back(c);
                found = true;
                break;
            }
        }

        if (!found) rows.push_back({c});
    }

    std::vector<std::string> lines;

    for (auto &row : rows) {
        std::sort(row.begin(), row.end(), [](const PdfChar &a, const PdfChar &b) { return a.x < b.x; });

        std::string line;

        for (auto &c : row)
            line += c.text;

        lines.push_back(line);
    }

    return lines;
}

std::vector<std::string> OrderParser::filter_noise(const std::vector<std::string> &lines) {
    std::vector<std::string> out;

    for (auto &line : lines) {
        if (line.find("Date") != std::string::npos) continue;
        if (line.find("Time") != std::string::npos) continue;
        if (line.find("OU:") != std::string::npos) continue;
        if (line.find("Email") != std::string::npos) continue;
        if (line.find("SIGN") != std::string::npos) continue;

        out.push_back(line);
    }

    return out;
}

size_t OrderParser::find_data_start(const std::vector<std::string> &lines) {
    for (size_t i = 0; i < lines.size(); i++) {
        printf("lines: %s\n", lines[i].c_str());
        if (is_order_number(lines[i])) return i;
    }

    return std::string::npos;
}

bool OrderParser::is_order_number(const std::string &s) {
    if (s.size() < 8) return false;
    if (s.find('-') == std::string::npos) return false;
    return true;
}