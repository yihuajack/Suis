//
// Created by Yihua Liu on 2024/8/12.
//

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRelationalTableModel>

#include "SqlTreeModel.h"

SqlTreeModel::SqlTreeModel(QObject *parent) : QAbstractItemModel(parent) {
    if (const QStringList drivers = QSqlDatabase::drivers(); drivers.empty()) {
        qWarning() << tr("No database drivers found.")
                   << tr("This part requires at least one Qt database driver. "
                         "Please check the documentation how to build the "
                         "Qt SQL plugins.");
    } else if (not drivers.contains("QOCI")) {
        qWarning() << tr("QOCI driver not available.");
    }

    QVariantList rootData(1);

    rootItem = std::make_unique<SqlTreeItem>(rootData);
}

SqlTreeModel::~SqlTreeModel() = default;

QVariant SqlTreeModel::data(const QModelIndex &index, const int role) const {
    if (not index.isValid()) {
        return {};
    }

    const SqlTreeItem *item = getItem(index);
    // for std::shared_ptr needs Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
    // https://www.kdab.com/psa-qpointer-has-a-terrible-name/
    auto [fst, snd] = item->data(index.column()).value<std::pair<QString, QSharedPointer<QSqlRelationalTableModel>>>();

    if (role == Qt::DisplayRole) {
        return fst;
    } else if (role == SqlTableRole) {
        return QVariant::fromValue(snd);
    } else {
        return {};
    }
}

QVariant SqlTreeModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
    return orientation == Qt::Horizontal and role == Qt::DisplayRole ? rootItem->data(section) : QVariant{};
}

QModelIndex SqlTreeModel::index(const int row, const int column, const QModelIndex &parent) const {
    if (parent.isValid() and parent.column() not_eq 0) {
        return {};
    }

    SqlTreeItem *parentItem = getItem(parent);
    if (not parentItem) {
        return {};
    }

    if (SqlTreeItem *childItem = parentItem->child(row)) {
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
    if (parent.isValid() and parent.column() > 0) {
        return 0;
    }

    const SqlTreeItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

int SqlTreeModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return rootItem->columnCount();
}

Qt::ItemFlags SqlTreeModel::flags(const QModelIndex &index) const {
    if (not index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

bool SqlTreeModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
    if (role not_eq Qt::EditRole) {
        return false;
    }

    SqlTreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

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

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

bool SqlTreeModel::addConnection(const QString &driver, const QString &database, const QString &user,
                                 const QString &passwd) {
    static int cCount = 0;

    QSqlError err;
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
        err = db.lastError();
        qInfo() << err;
        QSqlDatabase::removeDatabase(conn_name);
        return true;
    }
    insertRows(rowCount(), 1);
    rootItem->child(rootItem->childCount() - 1)->setData(0, conn_name);
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
    const QString conn_name = parent.isValid() ? parent.data().toString() : current.data().toString();
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
        QStringList tables = db.tables();
        int num_tables = static_cast<int>(tables.size());
        insertRows(0, num_tables, parent);

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
    insertRows(0, static_cast<int>(connectionNames.size()));
    for (int cn = 0; cn < connectionNames.size(); cn++) {
        const QSqlDatabase db = QSqlDatabase::database(connectionNames.at(cn), false);
        if (not db.isOpen()) {
            qWarning() << "Database not open.";
            return;
        }
        QStringList tables = db.tables();
        int num_tables = static_cast<int>(tables.size());
        const QModelIndex db_index = index(cn, 0);
        insertRows(0, num_tables, db_index);

        for (int t = 0; t < num_tables; t++) {
            QSharedPointer<QSqlRelationalTableModel> table_model = QSharedPointer<QSqlRelationalTableModel>(new QSqlRelationalTableModel(
                    nullptr, db), &QSqlRelationalTableModel::deleteLater);
            table_model->setTable(tables.at(t));
            table_model->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
            table_model->select();
            getItem(db_index)->child(t)->setData(0, QVariant::fromValue(table_model));
        }
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
