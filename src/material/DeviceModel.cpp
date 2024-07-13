//
// Created by Yihua Liu on 2024-7-1.
//

#include <numeric>
#include <ranges>
#include <set>
#include <QDir>
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
requires std::same_as<T, QString> || std::same_as<T, double>
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
                } else {  // std::same_as<T, double>
                    property[i++] = data.at(rc).at(index).toDouble();
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
    if (properties.find("layer_type") == properties.cend()) {
        qWarning("No layer type (layer_type) defined in .csv."
                 "layer_type must be defined when using .csv input file");
        return false;
    }
    qsizetype layer_type_index = properties.at("layer_type");  // ELECTRODE, LAYER, INTERFACE, ACTIVE
    if (csv_data.at(1).at(layer_type_index) == "electrode") {
        start_row = 2;
    } else {  // no front surface electrode
        start_row = 1;
        qWarning("Missing front surface electrode; RAT calculation is disabled.");
    }
    if (csv_data.at(maxRow - 1).at(layer_type_index) == "electrode") {
        end_row = maxRow - 2;
    } else {  // no back surface electrode
        end_row = maxRow - 1;
        qWarning("Missing back surface electrode; RAT calculation is disabled.");
    }
    QList<QString> layer_type(end_row - start_row + 1);
    for (const auto [i, rc] : std::views::enumerate(std::views::iota(start_row, end_row))) {
        layer_type[static_cast<qsizetype>(i)] = csv_data.at(rc).at(layer_type_index);
    }
    try {
        // This material list contains both layers and interfaces (typical structure has a size of 5).
        QList<QString> material = import_single_property<QString>(csv_data, properties, {"material", "stack"}, start_row, end_row);
        QList<double> d = import_single_property<double>(csv_data, properties, {"dcell", "d", "thickness"}, start_row, end_row);
        const qsizetype sz_mat = material.size();
        if (d.size() not_eq sz_mat) {
            qWarning("Size of thickness does not match size of material!");
            return false;
        }
        QString side_str = csv_data.at(properties.at("side")).at(1);
        bool side = false;  // left
        if (side_str == "right" or side_str == "2") {
            side = true;
        } else if (side_str not_eq "left" and side_str not_eq "1") {
            qWarning("Side property is not correctly specified.");
            return false;
        }
        if (start_row == 2 and end_row == maxRow - 1) {  // wl & RAT for this case only
            QList<QString> opt_material = import_single_property<QString>(csv_data, properties, {"material", "stack"}, 1, maxRow - 1);  // include electrodes
            // Access DbSysModel singleton
            // QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
            // if (not engine) {
            //     qWarning("QML Engine not found!");
            //     return false;
            // }
            // auto *db_system = engine->singletonInstance<DbSysModel*>("com.github.yihuajack.DbSysModel", "DbSysModel");
            // https://stackoverflow.com/questions/25403363/how-to-implement-a-singleton-provider-for-qmlregistersingletontype
            // https://stackoverflow.com/questions/50073626/reference-to-qml-singleton-class-instance
            DbSysModel *db_system = DbSysModel::instance();
            if (not db_system) {
                qWarning("QML singleton instance DbSysModel does not exist.");
                return false;
            }
            QList<double> opt_d = import_single_property<double>(csv_data, properties, {"dcell", "d", "thickness"}, 1, maxRow - 1);  // include front surface electrode
            std::vector<std::pair<OpticMaterial<QList<double>> *, double>> structure;
            if (opt_material.front().isEmpty()) {  // Default front surface
                opt_material.front() = "ITO";
            }
            if (opt_material.end()->isEmpty()) {  // Default back surface
                opt_material.back() = "Ag";
            }
            if (opt_d.front() == 0) {  // Default front surface thickness
                opt_d.front() = 5e-8;
            }
            if (side) {  // right; need to reverse
                for (qsizetype i = maxRow - 2; i >= 0; i--) {  // does not include the substrate
                    if (layer_type.at(i) not_eq "interface") {
                        structure.emplace_back(db_system->getMatByName(opt_material.at(i)), d.at(i));
                    }
                }
            } else {
                for (qsizetype i = 0; i < maxRow - 1; i++) {
                    if (layer_type.at(i) not_eq "interface") {
                        structure.emplace_back(db_system->getMatByName(opt_material.at(i)), d.at(i));
                    }
                }
            }
            wavelengths = structure.front().first->nWl();  // Warning: structure will be "moved" next!
            auto *stack = new OpticStack<QList<double>>(std::move(structure), false, db_system->getMatByName(material.back()));
            const rat_dict<double> rat_out = calculate_rat<QList<double>>(stack, std::forward<QList<double>>(wavelengths), 0, 's');
            const std::valarray<double> R_va = std::get<std::valarray<double>>(rat_out.at("R"));
            R = {std::begin(R_va), std::end(R_va)};
            const std::valarray<double> A_va = std::get<std::valarray<double>>(rat_out.at("A"));
            A = {std::begin(A_va), std::end(A_va)};
            const std::valarray<double> T_va = std::get<std::valarray<double>>(rat_out.at("T"));
            T = {std::begin(T_va), std::end(T_va)};
            delete stack;
        }
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
