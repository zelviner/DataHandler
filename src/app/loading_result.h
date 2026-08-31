#pragma once

#include <qlabel.h>

namespace LoadingResult {

inline void setText(QLabel *label, const QString &text) {
    label->setToolTip(text);
    label->setText(label->fontMetrics().elidedText(text, Qt::ElideRight, label->contentsRect().width()));
}

} // namespace LoadingResult
