//
// Created by Yihua Liu on 2024/8/12.
//

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRelationalTableModel>

#include "SqlTreeModel.h"

SqlTreeModel::SqlTreeModel(QObject *parent) : QAbstractItemModel(parent), rootItem(std::make_unique<SqlTreeItem>(QVariantList(1))) {  // Initialize the column size to 1!
    if (const QStringList drivers = QSqlDatabase::drivers(); drivers.empty()) {
        qWarning() << tr("No database drivers found.")
                   << tr("This part requires at least one Qt database driver. "
                         "Please check the documentation how to build the "
                         "Qt SQL plugins.");
    } else if (not drivers.contains("QOCI")) {
        qWarning() << tr("QOCI driver not available.");
    }
}

SqlTreeModel::~SqlTreeModel() = default;

QVariant SqlTreeModel::data(const QModelIndex &index, const int role) const {
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    const SqlTreeItem *item = getItem(index);
    // for std::shared_ptr needs Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
    // https://www.kdab.com/psa-qpointer-has-a-terrible-name/

    if (role == Qt::DisplayRole) {
        return item->data(index.column()).value<QString>();
    }
    if (role == SqlTableRole) {
        return QVariant::fromValue(item->data(index.column()).value<QSharedPointer<QSqlRelationalTableModel>>());
    }
    return {};
}

QVariant SqlTreeModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
    return orientation == Qt::Horizontal and role == Qt::DisplayRole ? rootItem->data(section) : QVariant{};
}

QModelIndex SqlTreeModel::index(const int row, const int column, const QModelIndex &parent) const {
    if (not hasIndex(row, column, parent)) {  // parent.isValid() and parent.column() not_eq 0
        return {};
    }

    const SqlTreeItem *parentItem = getItem(parent);
    if (not parentItem) {
        return {};
    }

    if (const SqlTreeItem *childItem = parentItem->child(row)) {
        return createIndex(row, column, childItem);
    }
    return {};
}

QModelIndex SqlTreeModel::parent(const QModelIndex &index) const {
    if (not index.isValid()) {
        return {};
    }

    SqlTreeItem *childItem = getItem(index);
    SqlTreeItem *parentItem = childItem ? childItem->parent() : nullptr;

    return parentItem not_eq rootItem.get() and parentItem
           ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex();
}

int SqlTreeModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0) {
        return 0;
    }

    const SqlTreeItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

int SqlTreeModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return static_cast<SqlTreeItem*>(parent.internalPointer())->columnCount();
    }
    return rootItem->columnCount();
}

Qt::ItemFlags SqlTreeModel::flags(const QModelIndex &index) const {
    return index.isValid() ? Qt::ItemIsEditable | QAbstractItemModel::flags(index) : Qt::NoItemFlags;
}

bool SqlTreeModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
    if (role not_eq Qt::EditRole) {
        return false;
    }

    SqlTreeItem *item = getItem(index);
    const bool result = item->setData(index.column(), value);

    if (result) {
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }

    return result;
}

bool SqlTreeModel::setHeaderData(const int section, const Qt::Orientation orientation, const QVariant &value, const int role) {
    if (role not_eq Qt::EditRole or orientation not_eq Qt::Horizontal) {
        return false;
    }

    const bool result = rootItem->setData(section, value);

    if (result) {
        emit headerDataChanged(orientation, section, section);
    }

    return result;
}

bool SqlTreeModel::insertColumns(const int position, const int columns, const QModelIndex &parent) {
    beginInsertColumns(parent, position, position + columns - 1);
    const bool success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool SqlTreeModel::removeColumns(const int position, const int columns, const QModelIndex &parent) {
    if (columns < 1) {
        return true;
    }
    beginRemoveColumns(parent, position, position + columns - 1);
    const bool success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0) {
        removeRows(0, rowCount());
    }

    return success;
}

bool SqlTreeModel::insertRows(const int position, const int rows, const QModelIndex &parent) {
    SqlTreeItem *parentItem = getItem(parent);
    if (not parentItem) {
        return false;
    }

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

bool SqlTreeModel::removeRows(const int position, const int rows, const QModelIndex &parent) {
    SqlTreeItem *parentItem = getItem(parent);
    if (not parentItem) {
        return false;
    }
    if (rows < 1) {
        return true;
    }

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

bool SqlTreeModel::addConnection(const QString &driver, const QString &database, const QString &user,
                                 const QString &passwd) {
    QStringList sl_db = database.split(':');
    QStringList subsl_db = sl_db.back().split('/');
    const QString& db_name = subsl_db.back();
    QString conn_name = driver + QLatin1Char(':');
    if (not user.isEmpty()) {
        conn_name += user + QLatin1Char('@');
    }
    conn_name += db_name;
    // https://doc.qt.io/qt-6/sql-driver.html#how-to-build-the-oci-plugin-on-windows
    // https://blog.csdn.net/yihuajack/article/details/143381286
    // https://bugreports.qt.io/browse/QTBUG-128670
    // Remember to rerun windeployqt reset CMake caches and reload the project!
    // https://forum.qt.io/topic/158713/cannot-load-qoci-sql-driver-even-successfully-built
    QSqlDatabase db = QSqlDatabase::addDatabase(driver, conn_name);
    db.setHostName(sl_db.front());
    db.setPort(subsl_db.front().toInt());
    db.setDatabaseName(db_name);
    if (not db.open(user, passwd)) {
        qInfo() << db.lastError();
        QSqlDatabase::removeDatabase(conn_name);
        return true;
    }
    // insertRows(rowCount(), 1);
    // rootItem->child(rootItem->childCount() - 1)->setData(0, conn_name);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    rootItem->appendChild(std::make_unique<SqlTreeItem>(QVariantList{conn_name}, rootItem.get()));
    endInsertRows();
    refresh(index(rootItem->childCount() - 1, 0));
    return false;
}

QHash<int, QByteArray> SqlTreeModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[SqlTableRole] = "table";
    return roles;
}

void SqlTreeModel::refresh(const QModelIndex &current) {
    // root does not have QModelIndex
    const QModelIndex parent = current.parent();  // db if table, top if db, invalid if top
    QString conn_name;
    if (parent.isValid()) {
        if (parent.data().canConvert<QString>()) {
            conn_name = parent.data().toString();
        } else {
            qWarning() << "Parent index data is not a QString.";
            return;
        }
    } else if (current.isValid()) {
        if (current.data().canConvert<QString>()) {
            conn_name = current.data().toString();
        } else {
            qWarning() << "Current index data is not a QString.";
            return;
        }
    } else {
        qWarning() << "No current or parent index.";
        return;
    }
    const QSqlDatabase db = QSqlDatabase::database(conn_name, false);
    if (not db.isOpen()) {
        qWarning() << "Database not open.";
        return;
    }
    if (parent.isValid()) {  // current = leaf table node
        const int row = current.row();
        removeRows(row, 1, parent);
        insertRows(row, 1, parent);

        const QString table_name = parent.data().toString();
        const QSharedPointer<QSqlRelationalTableModel> table_model = QSharedPointer<QSqlRelationalTableModel>(new QSqlRelationalTableModel(
                                                                                            nullptr, db), &QSqlRelationalTableModel::deleteLater);
        table_model->setTable(table_name);
        table_model->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
        table_model->select();
        getItem(parent)->child(row)->setData(0, QVariant::fromValue(table_model));
    } else {  // current = intermediate db node
        removeRows(0, rowCount(current), current);
        // https://forum.qt.io/topic/159677/qsqldatabase-tables-get-stuck
        const QStringList tables = db.tables();
        const int num_tables = static_cast<int>(tables.size());
        insertRows(0, num_tables, current);

        for (int t = 0; t < num_tables; t++) {
            QSharedPointer<QSqlRelationalTableModel> table_model = QSharedPointer<QSqlRelationalTableModel>(new QSqlRelationalTableModel(
                    nullptr, db), &QSqlRelationalTableModel::deleteLater);
            table_model->setTable(tables.at(t));
            table_model->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
            table_model->select();
            getItem(current)->child(t)->setData(0, QVariant::fromValue(table_model));
        }
    }
}

void SqlTreeModel::refreshAll() {
    const QStringList connectionNames = QSqlDatabase::connectionNames();
    beginResetModel();
    removeRows(0, rowCount());
    // insertRows(0, static_cast<int>(connectionNames.size()));
    for (int cn = 0; cn < connectionNames.size(); cn++) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        rootItem->appendChild(std::make_unique<SqlTreeItem>(QVariantList{connectionNames.at(cn)}, rootItem.get()));
        endInsertRows();
        refresh(index(cn, 0));
    }
    endResetModel();
}

SqlTreeItem *SqlTreeModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        if (auto *item = static_cast<SqlTreeItem *>(index.internalPointer())) {
            return item;
        }
    }
    return rootItem.get();
}
