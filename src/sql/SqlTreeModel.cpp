//
// Created by Yihua Liu on 2024/8/12.
//

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRelationalTableModel>

#include "SqlTreeModel.h"

SqlTreeModel::SqlTreeModel(QObject *parent) : QAbstractItemModel(parent), rootItem(std::make_unique<SqlTreeItem>(QVariantList(1))) {
    // Initialize the column size!
    // We do not need two columns that one for the name and the other one for the table.
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
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    // for std::shared_ptr needs Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
    // https://www.kdab.com/psa-qpointer-has-a-terrible-name/
    const QVariant data = getItem(index)->data(index.column());

    if (role == Qt::DisplayRole) {
        if (data.canConvert<QString>()) {
            return data.value<QString>();
        }
        if (not hasChildren(index)) {
            return data.value<std::pair<QString, QSqlRelationalTableModel*>>().first;
        }
        qWarning("Display role data is not a QString.");
    }
    if (role == SqlTableRole) {
        if (not hasChildren(index)) {
            return QVariant::fromValue(
                data.value<std::pair<QString, QSqlRelationalTableModel*>>().second);
        }
        qWarning("Non-leaf nodes do not have table role.");
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

void SqlTreeModel::refresh(const QModelIndex &current) {
    // root does not have QModelIndex
    using namespace Qt::Literals::StringLiterals;
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
    if (not db.isValid()) {
        qWarning() << "Database is invalid.";
        return;
    }
    if (not db.isOpen()) {
        qWarning() << "Database not open.";
        return;
    }
    if (parent.isValid()) {  // current = leaf table node
        const int row = current.row();
        removeRows(row, 1, parent);
        insertRows(row, 1, parent);

        const QString table_name = parent.data().toString();
        // const QSharedPointer<QSqlRelationalTableModel> table_model = QSharedPointer<QSqlRelationalTableModel>(new QSqlRelationalTableModel(
        //                                                                                     nullptr, db), &QSqlRelationalTableModel::deleteLater);
        QSqlRelationalTableModel *table_model = new QSqlRelationalTableModel(nullptr, db);
        table_model->setTable(table_name);
        table_model->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
        table_model->select();
        getItem(parent)->child(row)->setData(0, QVariant::fromValue(std::pair(table_name, table_model)));
    } else {  // current = intermediate db node
        removeRows(0, rowCount(current), current);
        // https://forum.qt.io/topic/159677/qsqldatabase-tables-get-stuck
        // Warning: QSqlDatabase::tables(QSql::TableType type = QSql::Tables) will query all the tables visible to the user.
        // For the test database, there will be 35610 tables queried!
        // const QStringList tables = db.tables();
        QSqlQuery table_query(db);
        // table_query.setForwardOnly(true);
        // table_query.exec("SELECT TABLE_NAME FROM ALL_TABLES WHERE OWNER=%1"_L1 % db.userName().toUpper());
        // Prepared statement
        if (const bool prepared = table_query.prepare("SELECT TABLE_NAME FROM ALL_TABLES WHERE OWNER=:owner"_L1); not prepared) {
            qWarning() << "Failed to prepare SELECT TABLE_NAME query:" << table_query.lastError();
        }
        table_query.bindValue(":owner", db.userName().toUpper());
        table_query.exec();  // set active if return true
        // const QSqlResult *sql_result = table_query.result();
        // QVariant v = sql_result->handle();
        // if (v.isValid() and qstrcmp(v.typeName(), "OCIStmt*") == 0) {
        //     OCIstmt *handle = *static_cast<OCIstmt **>(v.data());
        // }
        if (not table_query.isActive()) {  // Remember to add "not"
            // See https://forum.qt.io/topic/159693/qsqlquery-exec-returns-false
            qWarning() << "Failed to execute SELECT TABLE_NAME query:" << table_query.executedQuery() << table_query.lastError();
            return;
        }
        QStringList tables;
        while (table_query.next()) {
            tables.emplace_back(table_query.value(0).toString());
        }
        const int num_tables = static_cast<int>(tables.size());
        if (num_tables > 200) {
            qWarning("Database contains more than 200 tables, aborting.");
            return;
        }
        insertRows(0, num_tables, current);

        for (int t = 0; t < num_tables; t++) {
            QSqlRelationalTableModel *table_model = new QSqlRelationalTableModel(nullptr, db);
            table_model->setTable(tables.at(t));
            table_model->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
            table_model->select();
            getItem(current)->child(t)->setData(0, QVariant::fromValue(std::pair(tables.at(t), table_model)));
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

void SqlTreeModel::execQuery(const QString &query, const int db_id) {
    const QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::connectionNames().at(db_id));  // Do not use the default connection (defaultConnection)
    if (not db.isOpen()) {
        qWarning() << "database not open";
        return;
    }
    QSqlQuery sql_query(db);
    sql_query.exec(query);
}

QHash<int, QByteArray> SqlTreeModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[SqlTableRole] = "table";
    return roles;
}

SqlTreeItem *SqlTreeModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        if (auto *item = static_cast<SqlTreeItem *>(index.internalPointer())) {
            return item;
        }
    }
    return rootItem.get();
}
