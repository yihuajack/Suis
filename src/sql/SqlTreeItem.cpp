//
// Created by Yihua Liu on 2024/8/14.
//

#include "SqlTreeItem.h"

SqlTreeItem::SqlTreeItem(QVariantList data, SqlTreeItem *parent) : itemData(std::move(data)), m_parentItem(parent) {}

void SqlTreeItem::appendChild(std::unique_ptr<SqlTreeItem>&& child) {
    m_childItems.emplace_back(std::move(child));
}


SqlTreeItem *SqlTreeItem::child(const int number) const {
    return number >= 0 and number < childCount() ? m_childItems.at(number).get() : nullptr;
}

int SqlTreeItem::childCount() const {
    return static_cast<int>(m_childItems.size());
}

int SqlTreeItem::row() const {
    if (not m_parentItem) {
        return 0;
    }
    const auto it = std::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
                                 [this](const std::unique_ptr<SqlTreeItem> &SqlTreeItem) -> bool {
                                     return SqlTreeItem.get() == this;
                                 });

    if (it not_eq m_parentItem->m_childItems.cend()) {
        return static_cast<int>(std::distance(m_parentItem->m_childItems.cbegin(), it));
    }
    Q_ASSERT(false);  // should not happen
    return -1;
}

int SqlTreeItem::columnCount() const {
    return static_cast<int>(itemData.count());
}

QVariant SqlTreeItem::data(const int column) const {
    return itemData.value(column);
}

bool SqlTreeItem::insertChildren(const int position, const int count, const int columns) {
    if (position < 0 or position > static_cast<int>(m_childItems.size())) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        QVariantList data(columns);
        m_childItems.insert(m_childItems.cbegin() + position,
                            std::make_unique<SqlTreeItem>(data, this));
    }

    return true;
}

bool SqlTreeItem::insertColumns(const int position, const int columns) {
    if (position < 0 or position > itemData.size()) {
        return false;
    }

    for (int column = 0; column < columns; ++column) {
        itemData.insert(position, QVariant());
    }

    for (const std::unique_ptr<SqlTreeItem> &child : std::as_const(m_childItems)) {
        child->insertColumns(position, columns);
    }

    return true;
}

SqlTreeItem *SqlTreeItem::parent() const {
    return m_parentItem;
}

bool SqlTreeItem::removeChildren(const int position, const int count) {
    if (position < 0 or position + count > static_cast<qsizetype>(m_childItems.size())) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        m_childItems.erase(m_childItems.cbegin() + position);
    }

    return true;
}

bool SqlTreeItem::removeColumns(const int position, const int columns) {
    if (position < 0 or position + columns > itemData.size()) {
        return false;
    }

    for (int column = 0; column < columns; ++column) {
        itemData.remove(position);
    }

    for (const std::unique_ptr<SqlTreeItem> &child : std::as_const(m_childItems)) {
        child->removeColumns(position, columns);
    }

    return true;
}

bool SqlTreeItem::setData(const int column, const QVariant &value) {
    if (column < 0 or column >= itemData.size()) {
        return false;
    }

    itemData[column] = value;
    return true;
}
