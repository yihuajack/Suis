//
// Created by Yihua Liu on 2024/8/14.
//

#ifndef SUISAPP_SQLTREEITEM_H
#define SUISAPP_SQLTREEITEM_H

#include <QVariantList>

class SqlTreeItem {
public:
    explicit SqlTreeItem(QVariantList data, SqlTreeItem *parent = nullptr);

    SqlTreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    SqlTreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int row() const;
    bool setData(int column, const QVariant &value);

private:
    std::vector<std::unique_ptr<SqlTreeItem>> m_childItems;
    QVariantList itemData;
    SqlTreeItem *m_parentItem;
};

#endif  // SUISAPP_SQLTREEITEM_H
