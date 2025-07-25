//
// Created by Yihua Liu on 2024/8/12.
//

#include <QDir>
#include <QRegularExpression>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRelationalTableModel>

#include "xlsxdocument.h"
#include "xlsxworkbook.h"
#include "SqlTreeModel.h"
#include "DevSysModel.h"
#include "utils/DataIO.h"

SqlTreeModel::SqlTreeModel(QObject *parent) : QAbstractItemModel(parent), rootItem(std::make_unique<SqlTreeItem>(QVariantList(1))) {
    // Initialize the column size!
    // We do not need two columns that one for the name and the other one for the table.
    if (const QStringList drivers = QSqlDatabase::drivers(); drivers.empty()) {
        qWarning() << tr("No database drivers found.")
                   << tr("This part requires at least one Qt database driver. "
                         "Please check the documentation how to build the Qt SQL plugins.");
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
        auto *table_model = new QSqlRelationalTableModel(nullptr, db);
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
        // qsqlquery will check driver() is not empty, open, no open error, and query is not empty
        // table_query.exec("SELECT TABLE_NAME FROM ALL_TABLES WHERE OWNER=%1"_L1 % db.userName().toUpper());
        // Prepared statement
        // https://forum.qt.io/topic/161859/access-violation-at-qsql_oci-when-executing-preparing-qsqlquery
        // https://bugreports.qt.io/browse/QTBUG-136024
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
        int num_tables = static_cast<int>(tables.size());
        if (num_tables > m_maxTables) {
            num_tables = m_maxTables;
            qWarning() << "Database contains more than" << m_maxTables << "tables, will only insert the first"
                << m_maxTables << "tables.";
        }
        insertRows(0, num_tables, current);

        for (int t = 0; t < num_tables; t++) {
            auto *table_model = new QSqlRelationalTableModel(nullptr, db);
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

void SqlTreeModel::execQuery(const QString &query) const {
    const QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::connectionNames().at(m_dbId));  // Do not use the default connection (defaultConnection)
    if (not db.isOpen()) {
        qWarning() << "database not open";
        return;
    }
    QSqlQuery sql_query(db);
    sql_query.exec(query);
}

bool SqlTreeModel::upload(const QString &path, const int id) const {
    const QUrl url(path);
    QString upload_path = path;
    if (url.isLocalFile()) {
        upload_path = QDir::toNativeSeparators(url.toLocalFile());
    }
    const QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::connectionNames().at(m_dbId));  // Do not use the default connection (defaultConnection)
    if (not db.isOpen()) {
        qWarning() << "database not open";
        return false;
    }
    const std::array<double, 13> stats = Utils::DataIO::readSingleStats(upload_path.toStdString());
    QSqlQuery sql_query(db);
    sql_query.prepare(
        "INSERT INTO AI_STATS (DEVICE_ID, JSC_F, VOC_F, MPP_F, EFFICIENCY_F, MPPV_F, FF_F, JSC_R, VOC_R, MPP_R, "
        "EFFICIENCY_R, MPPV_R, FF_R, HF) VALUES (:DEVICE_ID, :JSC_F, :VOC_F, :MPP_F, :EFFICIENCY_F, :MPPV_F, :FF_F,"
        ":JSC_R, :VOC_R, :MPP_R, :EFFICIENCY_R, :MPPV_R, :FF_R, :HF)");
    sql_query.bindValue(":DEVICE_ID", id);
    sql_query.bindValue(":JSC_F", stats.front());
    sql_query.bindValue(":VOC_F", stats.at(2));
    sql_query.bindValue(":MPP_F", stats.at(3));
    sql_query.bindValue(":EFFICIENCY_F", stats.at(4));
    sql_query.bindValue(":MPPV_F", stats.at(5));
    sql_query.bindValue(":FF_F", stats.at(6));
    sql_query.bindValue(":JSC_R", stats.at(7));
    sql_query.bindValue(":VOC_R", stats.at(8));
    sql_query.bindValue(":MPP_R", stats.at(9));
    sql_query.bindValue(":EFFICIENCY_R", stats.at(10));
    sql_query.bindValue(":MPPV_R", stats.at(11));
    sql_query.bindValue(":FF_R", stats.at(12));
    sql_query.bindValue(":HF", stats.back());
    if (not sql_query.exec()) {
        qWarning() << "Failed to insert stats to AI_STATS:" << sql_query.lastError();
        return false;
    }
    return true;
}

void readRefrLib(QSqlQuery& query, QXlsx::Document& doc, const unsigned long long id) {
    const QString opticalPropertyTable = "X_OPTICAL_PROPERTY";
    const QString opticalRelateTable = "X_OPTICAL_PROPERTY_RELATE";
    query.prepare(QString("SELECT ID, REMARK, MATERIAL_SN FROM %1 WHERE ATTRIBUTE_ID = :idValue").arg(opticalPropertyTable));
    query.bindValue(":idValue", id);
    if (not query.exec()) {
        qDebug() << "Query execution failed:" << query.lastError().text();
    }
    const int maxCol = doc.dimension().columnCount();

    if (query.next()) {
        QList<std::array<QString, 3>> nkTable;
        const unsigned long long opticalId = query.value("ID").toULongLong();
        const QString remark = query.value("REMARK").toString();
        const QString sn = query.value("MATERIAL_SN").toString();
        if (not remark.isEmpty()) {
            const QStringList lines = remark.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
            for (qsizetype i = 1; i < lines.size(); ++i) {  // Skip the first line
                const QString& line = lines.at(i);
                const QStringList lineData = line.split(QRegularExpression("\\s+"));
                if (lineData.size() == 3) {
                    nkTable.append({lineData.front(), lineData.at(2), lineData.back()});
                } else {
                    qWarning() << lineData << "does not have 3 columns!";
                }
            }
        } else {
            QSqlQuery relateQuery(std::move(query));
            relateQuery.prepare(QString("SELECT OPTICAL_ID, WAVELENGTH, N, K FROM %1 WHERE OPTICAL_ID = :idValue").arg(opticalRelateTable));
            relateQuery.bindValue(":idValue", opticalId);
            if (not relateQuery.exec()) {
                qDebug() << "Query execution failed:" << relateQuery.lastError().text();
            }
            while (relateQuery.next()) {
                const double wavelength = relateQuery.value("WAVELENGTH").toDouble();
                const double n = relateQuery.value("N").toDouble();
                const double k = relateQuery.value("K").toDouble();
                nkTable.append({QString::number(wavelength), QString::number(n), QString::number(k)});
            }
        }
        doc.write(1, maxCol + 1, "Wavelength (nm)");
        doc.write(1, maxCol + 2, sn + "_n");
        doc.write(1, maxCol + 3, sn + "_k");
        int row = 2;
        for (const std::array<QString, 3>& columns : nkTable) {
            doc.write(row, maxCol + 1, columns.front().toDouble());  // if not double QXlsx will write as text
            doc.write(row, maxCol + 2, columns.at(2).toDouble());
            doc.write(row, maxCol + 3, columns.back().toDouble());
            ++row;
        }
    }
}

bool SqlTreeModel::readGclDb(const QString &path) {
    const QUrl url(path);
    QString import_path = path;
    if (url.isLocalFile()) {
        import_path = QDir::toNativeSeparators(url.toLocalFile());
    }
    const QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::connectionNames().at(m_dbId));
    if (not db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    // Do not try to check if the device table exists because tables() is extremely slow
    const QString deviceTable = "X_DEVICE";
    const QString electricalPropertyTable = "X_ELECTRICAL_PROPERTY_ALL";
    // Query the device table
    QSqlQuery query(db);
    query.setForwardOnly(true);  // Hope this can make it faster, property since Qt 6.8
    query.setNumericalPrecisionPolicy(QSql::HighPrecision);  // Default precision policy, property since Qt 6.8
    query.prepare(QString("SELECT ID, DEVICE_NAME, ETL_MATERIAL_ID, HTL_MATERIAL_ID, PVK_MATERIAL_ID, INT1_MATERIAL_ID, "
                  "INT2_MATERIAL_ID, ETL_THICKNESS, HTL_THICKNESS, PVK_THICKNESS, INT1_THICKNESS, INT2_THICKNESS, "
                  "ETL_LAYER_POINT, HTL_LAYER_POINT, PVK_LAYER_POINT, INT1_LAYER_POINT, INT2_LAYER_POINT, "
                  "ETL_XMESH_COEFF, HTL_XMESH_COEFF, PVK_XMESH_COEFF, INT1_XMESH_COEFF, INT2_XMESH_COEFF, "
                  "ELECTRODE0_EF0, ELECTRODE1_EF0, ELECTRODE0_SN, ELECTRODE0_SP, INT1_SN, INT1_SP, INT2_SN, INT2_SP, "
                  "ELECTRODE1_SN, ELECTRODE1_SP, INT1_VSR_ZONE_LOC, INT2_VSR_ZONE_LOC, XMESH_TYPE, OPTICAL_MODEL, "
                  "SIDE, N_IONIC_SPECIES FROM %1").arg(deviceTable));  // Table names cannot be bound as query parameters using bindValue()!
    QString deviceName;
    int deviceId;
    unsigned long long ETL_materialId, HTL_materialId, PVK_materialId, INT1_materialId, INT2_materialId;
    double ETL_thickness, HTL_thickness, PVK_thickness, INT1_thickness, INT2_thickness;
    int ETL_layerPoint, HTL_layerPoint, PVK_layerPoint, INT1_layerPoint, INT2_layerPoint;
    double ETL_xmeshCoeff, HTL_xmeshCoeff, PVK_xmeshCoeff, INT1_xmeshCoeff, INT2_xmeshCoeff, ELECTRODE0_EF0, ELECTRODE1_EF0;
    double ELECTRODE0_sn, ELECTRODE0_sp, INT1_sn, INT1_sp, INT2_sn, INT2_sp, ELECTRODE1_sn, ELECTRODE1_sp;
    QString INT1_vsrZoneLoc, INT2_vsrZoneLoc, xmeshType;
    bool opticalModel;
    int side, NIonicSpecies;
    if (not query.exec()) {
        qDebug() << "Query execution failed:" << query.lastError().text();
        return false;
    }

    QString materialSn;
    double Phi_EA, Phi_IP, EF0, ET, NC, NV, NCAT, NANI, CMAX, AMAX;
    double MUN, MUP, MUC, MUA, EPP, G0, B, TAUN, TAUP;

    while (query.next()) {
        deviceId = query.value("ID").toInt();
        deviceName = query.value("DEVICE_NAME").toString();
        ETL_materialId = query.value("ETL_MATERIAL_ID").toULongLong();
        HTL_materialId = query.value("HTL_MATERIAL_ID").toULongLong();
        PVK_materialId = query.value("PVK_MATERIAL_ID").toULongLong();
        INT1_materialId = query.value("INT1_MATERIAL_ID").toULongLong();
        INT2_materialId = query.value("INT2_MATERIAL_ID").toULongLong();
        ETL_thickness = query.value("ETL_THICKNESS").toDouble();
        HTL_thickness = query.value("HTL_THICKNESS").toDouble();
        PVK_thickness = query.value("PVK_THICKNESS").toDouble();
        INT1_thickness = query.value("INT1_THICKNESS").toDouble();
        INT2_thickness = query.value("INT2_THICKNESS").toDouble();
        ETL_layerPoint = query.value("ETL_LAYER_POINT").toInt();
        HTL_layerPoint = query.value("HTL_LAYER_POINT").toInt();
        PVK_layerPoint = query.value("PVK_LAYER_POINT").toInt();
        INT1_layerPoint = query.value("INT1_LAYER_POINT").toInt();
        INT2_layerPoint = query.value("INT2_LAYER_POINT").toInt();
        ETL_xmeshCoeff = query.value("ETL_XMESH_COEFF").toDouble();
        HTL_xmeshCoeff = query.value("HTL_XMESH_COEFF").toDouble();
        PVK_xmeshCoeff = query.value("PVK_XMESH_COEFF").toDouble();
        INT1_xmeshCoeff = query.value("INT1_XMESH_COEFF").toDouble();
        INT2_xmeshCoeff = query.value("INT2_XMESH_COEFF").toDouble();
        ELECTRODE0_EF0 = query.value("ELECTRODE0_EF0").toDouble();
        ELECTRODE1_EF0 = query.value("ELECTRODE1_EF0").toDouble();
        ELECTRODE0_sn = query.value("ELECTRODE0_SN").toDouble();
        ELECTRODE0_sp = query.value("ELECTRODE0_SP").toDouble();
        INT1_sn = query.value("INT1_SN").toDouble();
        INT1_sp = query.value("INT1_SP").toDouble();
        INT2_sn = query.value("INT2_SN").toDouble();
        INT2_sp = query.value("INT2_SP").toDouble();
        ELECTRODE1_sn = query.value("ELECTRODE1_SN").toDouble();
        ELECTRODE1_sp = query.value("ELECTRODE1_SP").toDouble();
        INT1_vsrZoneLoc = query.value("INT1_VSR_ZONE_LOC").toString();
        INT2_vsrZoneLoc = query.value("INT2_VSR_ZONE_LOC").toString();
        xmeshType = query.value("XMESH_TYPE").toString();
        opticalModel = query.value("OPTICAL_MODEL").toInt();
        side = query.value("SIDE").toInt();
        NIonicSpecies = query.value("N_IONIC_SPECIES").toInt();

        // Open CSV file for writing
        QFileInfo scriptInfo(import_path);
        QString optPropPath = scriptInfo.absolutePath() + "/Libraries/" + deviceName + ".xlsx";
        QXlsx::Document doc;
        QSqlQuery property_query(db);  // Do not use the device query! Otherwise, the while loop only executes once.
        readRefrLib(property_query, doc, side == 1 ? ETL_materialId : HTL_materialId);

        QString devConfPath = scriptInfo.absolutePath() + "/Input_files/" + deviceName + ".csv";
        QFile file(devConfPath);
        // WriteOnly implies Truncate unless combined with ReadOnly, Append or NewOnly
        if (not file.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to create file:" << devConfPath << file.errorString();
            return false;
        }
        QTextStream out(&file);

        out << "layer_type,material,thickness,layer_points,xmesh_coeff,Phi_EA,Phi_IP,EF0,Et,Nc,Nv,Ncat,Nani,c_max,a_max,"
               "mu_n,mu_p,mu_c,mu_a,epp,g0,B,taun,taup,sn,sp,vsr_zone_loc,Red,Green,Blue,optical_model,xmesh_type,side,"
               "N_ionic_species\n";

        out << "electrode,,,,,,," << ELECTRODE0_EF0 <<",,,,,,,,,,,,,,,,," << ELECTRODE0_sn << ',' << ELECTRODE0_sp <<
            ",,,,," << opticalModel << ',' << xmeshType << ',' << side << ',' << NIonicSpecies << "\n";

        // Clazy: Use multi-arg instead
        property_query.prepare(QString("SELECT MATERIAL_SN, PHI_EA, PHI_IP, EF0, ET, NC, NV, NCAT, NANI, "
                              "CMAX, AMAX, MUN, MUP, MUC, MUA, EPP, G0, B, TAUN, TAUP FROM %1 "
                              "WHERE %2 = :idValue").arg(electricalPropertyTable, "ID"));
        property_query.bindValue(":idValue", side == 1 ? ETL_materialId : HTL_materialId);
        if (not property_query.exec()) {
            qDebug() << "Query execution failed:" << property_query.lastError().text();
            return false;
        }

        if (property_query.next()) {
            // materialName = property_query.value("MATERIAL_NAME").toString();
            materialSn = property_query.value("MATERIAL_SN").toString();
            Phi_EA = property_query.value("PHI_EA").toDouble();
            Phi_IP = property_query.value("PHI_IP").toDouble();
            EF0 = property_query.value("EF0").toDouble();
            ET = property_query.value("ET").toDouble();
            NC = property_query.value("NC").toDouble();
            NV = property_query.value("NV").toDouble();
            NCAT = property_query.value("NCAT").toDouble();
            NANI = property_query.value("NANI").toDouble();
            CMAX = property_query.value("CMAX").toDouble();
            AMAX = property_query.value("AMAX").toDouble();
            MUN = property_query.value("MUN").toDouble();
            MUP = property_query.value("MUP").toDouble();
            MUC = property_query.value("MUC").toDouble();
            MUA = property_query.value("MUA").toDouble();
            EPP = property_query.value("EPP").toDouble();
            G0 = property_query.value("G0").toDouble();
            B = property_query.value("B").toDouble();
            TAUN = property_query.value("TAUN").toDouble();
            TAUP = property_query.value("TAUP").toDouble();
        } else {
            qDebug() << "No data found in table" << electricalPropertyTable;
            return false;
        }

        if (side == 1) {
            out << "layer," << materialSn << ',' << ETL_thickness << ',' << ETL_layerPoint << ',' << ETL_xmeshCoeff << ',' << Phi_EA << ','
                << Phi_IP << ',' << EF0 << ',' << ET << ',' << NC << ',' << NV << ',' << NCAT << ',' << NANI << ',' << CMAX << ','
                << AMAX << ',' << MUN << ',' << MUP << ',' << MUC << ',' << MUA << ',' << EPP << ',' << G0 << ',' << B << ','
                << TAUN << ',' << TAUP << ",,,,0.85,0.95,0.7,,,,\n";
        } else {
            out << "layer," << materialSn << ',' << HTL_thickness << ',' << HTL_layerPoint << ',' << HTL_xmeshCoeff << ',' << Phi_EA << ','
                << Phi_IP << ',' << EF0 << ',' << ET << ',' << NC << ',' << NV << ',' << NCAT << ',' << NANI << ',' << CMAX << ','
                << AMAX << ',' << MUN << ',' << MUP << ',' << MUC << ',' << MUA << ',' << EPP << ',' << G0 << ',' << B << ','
                << TAUN << ',' << TAUP << ",,,,0.85,0.95,0.7,,,,\n";
        }

        // Maximize degree of freedom
        property_query.prepare(QString("SELECT MATERIAL_SN, PHI_EA, PHI_IP, EF0, ET, NC, NV, NCAT, NANI, CMAX, AMAX, MUN,"
                                  "MUP, MUC, MUA, EPP, G0, B, TAUN, TAUP FROM %1 WHERE %2 = :idValue").arg(electricalPropertyTable, "ID"));
        property_query.bindValue(":idValue", INT1_materialId);
        if (not property_query.exec()) {
            qDebug() << "Query execution failed:" << property_query.lastError().text();
            return false;
        }

        if (property_query.next()) {
            materialSn = property_query.value("MATERIAL_SN").toString();
            Phi_EA = property_query.value("PHI_EA").toDouble();
            Phi_IP = property_query.value("PHI_IP").toDouble();
            EF0 = property_query.value("EF0").toDouble();
            ET = property_query.value("ET").toDouble();
            NC = property_query.value("NC").toDouble();
            NV = property_query.value("NV").toDouble();
            NCAT = property_query.value("NCAT").toDouble();
            NANI = property_query.value("NANI").toDouble();
            CMAX = property_query.value("CMAX").toDouble();
            AMAX = property_query.value("AMAX").toDouble();
            MUN = property_query.value("MUN").toDouble();
            MUP = property_query.value("MUP").toDouble();
            MUC = property_query.value("MUC").toDouble();
            MUA = property_query.value("MUA").toDouble();
            EPP = property_query.value("EPP").toDouble();
            G0 = property_query.value("G0").toDouble();
            B = property_query.value("B").toDouble();
        } else {
            qDebug() << "No data found in table" << electricalPropertyTable;
            return false;
        }

        out << "interface," << materialSn << ',' << INT1_thickness << ',' << INT1_layerPoint << ',' << INT1_xmeshCoeff
            << ',' << Phi_EA << ',' << Phi_IP << ',' << EF0 << ',' << ET << ',' << NC << ',' << NV << ',' << NCAT << ','
            << NANI << ',' << CMAX << ',' << AMAX << ',' << MUN << ',' << MUP << ',' << MUC << ',' << MUA << ',' << EPP
            << ',' << G0 << ',' << B << ",,," <<  INT1_sn << ',' << INT1_sp << ',' << INT1_vsrZoneLoc
            << ",1,0.9,0.7,,,,\n";

        readRefrLib(property_query, doc, PVK_materialId);

        property_query.prepare(QString("SELECT MATERIAL_SN, PHI_EA, PHI_IP, EF0, ET, NC, NV, NCAT, NANI, CMAX, AMAX, MUN,"
                                  "MUP, MUC, MUA, EPP, G0, B, TAUN, TAUP FROM %1 WHERE %2 = :idValue").arg(electricalPropertyTable, "ID"));
        property_query.bindValue(":idValue", PVK_materialId);
        if (not property_query.exec()) {
            qDebug() << "Query execution failed:" << property_query.lastError().text();
            return false;
        }

        if (property_query.next()) {
            materialSn = property_query.value("MATERIAL_SN").toString();
            Phi_EA = property_query.value("PHI_EA").toDouble();
            Phi_IP = property_query.value("PHI_IP").toDouble();
            EF0 = property_query.value("EF0").toDouble();
            ET = property_query.value("ET").toDouble();
            NC = property_query.value("NC").toDouble();
            NV = property_query.value("NV").toDouble();
            NCAT = property_query.value("NCAT").toDouble();
            NANI = property_query.value("NANI").toDouble();
            CMAX = property_query.value("CMAX").toDouble();
            AMAX = property_query.value("AMAX").toDouble();
            MUN = property_query.value("MUN").toDouble();
            MUP = property_query.value("MUP").toDouble();
            MUC = property_query.value("MUC").toDouble();
            MUA = property_query.value("MUA").toDouble();
            EPP = property_query.value("EPP").toDouble();
            G0 = property_query.value("G0").toDouble();
            B = property_query.value("B").toDouble();
            TAUN = property_query.value("TAUN").toDouble();
            TAUP = property_query.value("TAUP").toDouble();
        } else {
            qDebug() << "No data found in table" << electricalPropertyTable;
            return false;
        }

        out << "active," << materialSn << ',' << PVK_thickness << ',' << PVK_layerPoint << ',' << PVK_xmeshCoeff << ',' << Phi_EA << ','
            << Phi_IP << ',' << EF0 << ',' << ET << ',' << NC << ',' << NV << ',' << NCAT << ',' << NANI << ',' << CMAX << ','
            << AMAX << ',' << MUN << ',' << MUP << ',' << MUC << ',' << MUA << ',' << EPP << ',' << G0 << ',' << B << ','
            << TAUN << ',' << TAUP << ",,,,1,1,1,,,,\n";

        property_query.prepare(QString("SELECT MATERIAL_SN, PHI_EA, PHI_IP, EF0, ET, NC, NV, NCAT, NANI, CMAX, AMAX, MUN,"
                                  "MUP, MUC, MUA, EPP, G0, B, TAUN, TAUP FROM %1 WHERE %2 = :idValue").arg(electricalPropertyTable, "ID"));
        property_query.bindValue(":idValue", INT2_materialId);
        if (not property_query.exec()) {
            qDebug() << "Query execution failed:" << property_query.lastError().text();
            return false;
        }

        if (property_query.next()) {
            materialSn = property_query.value("MATERIAL_SN").toString();
            Phi_EA = property_query.value("PHI_EA").toDouble();
            Phi_IP = property_query.value("PHI_IP").toDouble();
            EF0 = property_query.value("EF0").toDouble();
            ET = property_query.value("ET").toDouble();
            NC = property_query.value("NC").toDouble();
            NV = property_query.value("NV").toDouble();
            NCAT = property_query.value("NCAT").toDouble();
            NANI = property_query.value("NANI").toDouble();
            CMAX = property_query.value("CMAX").toDouble();
            AMAX = property_query.value("AMAX").toDouble();
            MUN = property_query.value("MUN").toDouble();
            MUP = property_query.value("MUP").toDouble();
            MUC = property_query.value("MUC").toDouble();
            MUA = property_query.value("MUA").toDouble();
            EPP = property_query.value("EPP").toDouble();
            G0 = property_query.value("G0").toDouble();
            B = property_query.value("B").toDouble();
        } else {
            qDebug() << "No data found in table" << electricalPropertyTable;
            return false;
        }

        out << "interface," << materialSn << ',' << INT2_thickness << ',' << INT2_layerPoint << ',' << INT2_xmeshCoeff
            << ',' << Phi_EA << ',' << Phi_IP << ',' << EF0 << ',' << ET << ',' << NC << ',' << NV << ',' << NCAT << ','
            << NANI << ',' << CMAX << ',' << AMAX << ',' << MUN << ',' << MUP << ',' << MUC << ',' << MUA << ',' << EPP
            << ',' << G0 << ',' << B << ",,," <<  INT2_sn << ',' << INT2_sp << ',' << INT2_vsrZoneLoc
            << ",1,0.9,0.7,,,,\n";

        readRefrLib(property_query, doc, side == 1 ? HTL_materialId : ETL_materialId);
        doc.saveAs(optPropPath);

        property_query.prepare(QString("SELECT MATERIAL_SN, PHI_EA, PHI_IP, EF0, ET, NC, NV, NCAT, NANI, "
                              "CMAX, AMAX, MUN, MUP, MUC, MUA, EPP, G0, B, TAUN, TAUP FROM %1 "
                              "WHERE %2 = :idValue").arg(electricalPropertyTable, "ID"));
        property_query.bindValue(":idValue", side == 1 ? HTL_materialId : ETL_materialId);
        if (not property_query.exec()) {
            qDebug() << "Query execution failed:" << property_query.lastError().text();
            return false;
        }

        if (property_query.next()) {
            materialSn = property_query.value("MATERIAL_SN").toString();
            Phi_EA = property_query.value("PHI_EA").toDouble();
            Phi_IP = property_query.value("PHI_IP").toDouble();
            EF0 = property_query.value("EF0").toDouble();
            ET = property_query.value("ET").toDouble();
            NC = property_query.value("NC").toDouble();
            NV = property_query.value("NV").toDouble();
            NCAT = property_query.value("NCAT").toDouble();
            NANI = property_query.value("NANI").toDouble();
            CMAX = property_query.value("CMAX").toDouble();
            AMAX = property_query.value("AMAX").toDouble();
            MUN = property_query.value("MUN").toDouble();
            MUP = property_query.value("MUP").toDouble();
            MUC = property_query.value("MUC").toDouble();
            MUA = property_query.value("MUA").toDouble();
            EPP = property_query.value("EPP").toDouble();
            G0 = property_query.value("G0").toDouble();
            B = property_query.value("B").toDouble();
            TAUN = property_query.value("TAUN").toDouble();
            TAUP = property_query.value("TAUP").toDouble();
        } else {
            qDebug() << "No data found in table" << electricalPropertyTable;
            return false;
        }

        if (side == 1) {
            out << "layer," << materialSn << ',' << HTL_thickness << ',' << HTL_layerPoint << ',' << HTL_xmeshCoeff
                << ',' << Phi_EA << ',' << Phi_IP << ',' << EF0 << ',' << ET << ',' << NC << ',' << NV << ',' << NCAT
                << ',' << NANI << ',' << CMAX << ',' << AMAX << ',' << MUN << ',' << MUP << ',' << MUC << ',' << MUA
                << ',' << EPP << ',' << G0 << ',' << B << ',' << TAUN << ',' << TAUP << ",,,,0.8,0.9,1,,,,\n";
        } else {
            out << "layer," << materialSn << ',' << ETL_thickness << ',' << ETL_layerPoint << ',' << ETL_xmeshCoeff
                << ',' << Phi_EA << ',' << Phi_IP << ',' << EF0 << ',' << ET << ',' << NC << ',' << NV << ',' << NCAT
                << ',' << NANI << ',' << CMAX << ',' << AMAX << ',' << MUN << ',' << MUP << ',' << MUC << ',' << MUA
                << ',' << EPP << ',' << G0 << ',' << B << ',' << TAUN << ',' << TAUP << ",,,,0.8,0.9,1,,,,\n";
        }

        out << "electrode,,,,,,," << ELECTRODE1_EF0 << ",,,,,,,,,,,,,,,,," << ELECTRODE1_sn << ',' << ELECTRODE1_sp
            <<",,,,,,,,\n";

        file.close();
        m_devId.emplace_back(deviceId);
        m_devList.emplace_back(devConfPath);
    }
    return true;
}

int SqlTreeModel::dbId() const {
    return m_dbId;
}

void SqlTreeModel::setDbId(const int dbId) {
    m_dbId = dbId;
}

int SqlTreeModel::maxTables() const {
    return m_maxTables;
}

void SqlTreeModel::setMaxTables(const int maxTables) {
    m_maxTables = maxTables;
}

QList<int> SqlTreeModel::devId() const {
    return m_devId;
}

QStringList SqlTreeModel::devList() const {
    return m_devList;
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
