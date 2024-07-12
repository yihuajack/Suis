//
// Created by Yihua Liu on 2024-7-1.
//

#include <set>
#include <QDir>
#include <QQmlEngine>
#include <QUrl>

#include "DbSysModel.h"
#include "DeviceModel.h"
#include "optics/TransferMatrix.h"

DeviceModel::DeviceModel(QObject *parent) : QAbstractTableModel(parent) {}

int DeviceModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return static_cast<int>(m_data.size());
}

int DeviceModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_data.empty() ? 0 : static_cast<int>(m_data["d"].size());
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
    if (not index.isValid() or index.row() >= m_data.count()) {
        qWarning("QModelIndex of DeviceModel is invalid.");
        return {};
    }
    QMap<QString, QList<double>>::const_iterator it = m_data.cbegin();
    std::advance(it, index.row());
    switch (role) {
        case NameRole:
            return it.key();
        case WlRole:
            return QVariant::fromValue(wavelengths);
        case RRole:
            return QVariant::fromValue(R);
        case ARole:
            return QVariant::fromValue(A);
        case TRole:
            return QVariant::fromValue(T);
        default:
            return {};
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[WlRole] = "wavelength";
    roles[RRole] = "R";
    roles[ARole] = "A";
    roles[TRole] = "T";
    return roles;
}

QString DeviceModel::name() const {
    return m_name;
}

template<typename T>
QList<T> import_single_property(const QList<QStringList> &data,
                                const std::map<QString, qsizetype> &property_in,
                                const std::set<QString> &possible_headers,
                                qsizetype start_row,
                                qsizetype end_row) {
    for (const QString &possible_header : possible_headers) {
        const std::map<QString, qsizetype>::const_iterator it = property_in.find(possible_header);
        if (it not_eq property_in.cend()) {
            const qsizetype index = it->second;
            QList<T> property(end_row - start_row + 1);
            int i = 0;
            for (qsizetype rc = start_row; rc <= end_row; rc++) {
                if constexpr (std::same_as<T, QString>) {
                    property[i++] = data.at(rc).at(index);
                } else if constexpr (std::same_as<T, double>) {
                    property[i++] = data.at(rc).at(index).toDouble();
                } else {
                    static_assert(std::is_same_v<T, void>,
                            "Function import_single_property does not accept types other than QString and double.");
                }
            }
            return property;
        }
    }
    throw std::out_of_range("No column headings match: " + possible_headers.cbegin()->toStdString() +
                            ", using default in PC.");
}

bool DeviceModel::readDfDev(const QString &db_path) {
    const QUrl url(db_path);
    QString db_path_imported = db_path;
    if (url.isLocalFile()) {
        db_path_imported = QDir::toNativeSeparators(url.toLocalFile());
    }
    QFile doc(db_path_imported);
    if (not doc.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot load DriftFusion's device data file %s ", qUtf8Printable(db_path));
        return false;
    }
    QList<QStringList> csv_data;
    QTextStream csv_in(&doc);
    while (not csv_in.atEnd()) {
        QString csv_ln = csv_in.readLine();
        QStringList fields = csv_ln.split(",");
        csv_data.append(fields);
    }
    doc.close();
    qsizetype maxRow = csv_data.size();
    qsizetype maxCol = csv_data.front().size();
    std::map<QString, qsizetype> properties;
    for (qsizetype cc = 0; cc < maxCol; cc++) {
        properties[csv_data.front().at(cc)] = cc;
    }
    qsizetype start_row;
    qsizetype end_row;
    try {  // ELECTRODE, LAYER, INTERFACE, ACTIVE
        qsizetype layer_type_index = properties.at("layer_type");
        if (csv_data.at(1).at(layer_type_index) == "electrode") {
            start_row = 2;
            end_row = maxRow - 2;
        } else {  // no electrode
            start_row = 1;
            end_row = maxRow - 1;
        }
        QList<QString> layer_type(start_row + end_row - 1);
        int i = 0;
        for (qsizetype rc = start_row; rc <= end_row; rc++) {
            layer_type[i++] = csv_data.at(rc).at(layer_type_index);
        }
    } catch (std::out_of_range &e) {
        qWarning("No layer type (layer_type) defined in .csv."
                 "layer_type must be defined when using .csv input file");
        return false;
    }
    try {
        QList<QString> material = import_single_property<QString>(csv_data, properties, {"material", "stack"}, start_row, end_row);
        // Access DbSysModel singleton
        QQmlEngine *engine = qmlEngine(this);
        if (not engine) {
            qWarning("QML Engine not found!");
            return false;
        }
        auto *db_system = engine->singletonInstance<DbSysModel*>("com.github.yihuajack.DbSysModel", "DbSysModel");
        if (not db_system) {
            qWarning("QML singleton instance DbSysModel does not exist.");
            return false;
        }
        const qsizetype sz_mat = material.size();
        std::vector<std::pair<OpticMaterial<QList<double>> *, double>> structure(sz_mat);
        QList<double> d = import_single_property<double>(csv_data, properties, {"dcell", "d", "thickness"}, start_row, end_row);
        if (d.size() not_eq sz_mat) {
            qWarning("Size of thickness does not match size of material!");
            return false;
        }
        for (qsizetype i = 0; i < sz_mat - 1; i++) {
            structure.emplace_back(db_system->getMatByName(material.at(i)), d.at(i));
        }
        auto *stack = (start_row == 2) ? new OpticStack<QList<double>>(std::move(structure), false, db_system->getMatByName(material.back())) : new OpticStack<QList<double>>(std::move(structure));
        wavelengths = structure.front().first->nWl();
        const rat_dict<double> rat_out = calculate_rat<QList<double>>(stack, std::forward<QList<double>>(wavelengths), 0, 's', false);
        const std::valarray<double> R_va = std::get<std::valarray<double>>(rat_out.at("R"));
        R = {std::begin(R_va), std::end(R_va)};
        const std::valarray<double> A_va = std::get<std::valarray<double>>(rat_out.at("A"));
        A = {std::begin(A_va), std::end(A_va)};
        const std::valarray<double> T_va = std::get<std::valarray<double>>(rat_out.at("T"));
        T = {std::begin(T_va), std::end(T_va)};
        delete stack;
        m_data["d"] = d;
    } catch (std::out_of_range &e) {
        qWarning(e.what());
        return false;
    } catch (std::runtime_error &e) {
        qWarning(e.what());
        return false;
    }
    return true;
}
