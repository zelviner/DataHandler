#include "order_parser.h"

#include <mupdf/pdf.h>

std::shared_ptr<OrderInfo> OrderParser::parse(const std::string &pdf) {
    auto        info = std::make_shared<OrderInfo>();
    fz_context *ctx  = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    if (!ctx) return nullptr;

    fz_register_document_handlers(ctx);

    fz_try(ctx) {
        fz_document *doc = fz_open_document(ctx, pdf.c_str());

        info = read_form_fields(ctx, doc);
    }
    fz_catch(ctx) {}

    fz_drop_context(ctx);

    return info;
}

std::shared_ptr<OrderInfo> OrderParser::read_form_fields(fz_context *ctx, fz_document *doc) {
    auto          info = std::make_shared<OrderInfo>();
    pdf_document *pdf  = pdf_specifics(ctx, doc);

    int pages = pdf_count_pages(ctx, pdf);

    for (int i = 0; i < pages; i++) {
        pdf_page *page = pdf_load_page(ctx, pdf, i);

        for (pdf_annot *widget = pdf_first_widget(ctx, page); widget; widget = pdf_next_widget(ctx, widget)) {
            pdf_obj *field = pdf_annot_obj(ctx, widget);

            // const char *name  = pdf_field_name(ctx, field);
            auto label = std::string(pdf_field_label(ctx, field));
            auto value = std::string(pdf_field_value(ctx, field));

            if (label == "Order Number") {
                info->order_number = value;
            } else if (label == "Order Quantity") {
                info->quantity = std::stoi(value);
            } else if (label == "Project Name") {
                info->project_name = value;
            } else if (label == "Product Type") {
                info->product_type = value;
            } else if (label == "RF Code") {
                info->rf_code = value;
            } else if (label == "Script Package") {
                info->script_package = value;
            } else if (label == "Chip Model") {
                info->chip_model = value;
            }
        }

        pdf_drop_page(ctx, page);
    }

    return info;
}