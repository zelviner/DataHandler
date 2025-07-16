#pragma once

#include <qsortfilterproxymodel>

class FuzzyFilterProxyModel : public QSortFilterProxyModel {
  public:
    FuzzyFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent) {}

  protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        if (filterRegExp().isEmpty()) return true;

        QModelIndex index    = sourceModel()->index(source_row, filterKeyColumn(), source_parent);
        QString     itemText = sourceModel()->data(index).toString();

        // 模糊匹配：这里简单实现包含匹配（你也可以实现 Levenshtein 或拼音匹配）
        return itemText.contains(filterRegExp());
    }
};