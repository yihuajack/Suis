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
        case Qt::DisplayRole:
            return m_data[index.row()][index.column()];
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
                } else if constexpr (std::same_as<T, qsizetype>) {
                    property[i++] = data.at(rc).at(index).toLongLong();
                } else {
                    static_assert(false, "Invalid type for import_single_property");
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
    qsizetype maxCol = csv_data.front().size();
    std::map<QString, qsizetype> properties;
    for (qsizetype cc = 0; cc < maxCol; cc++) {
        properties[csv_data.front().at(cc)] = cc;
    }
    return import_properties(csv_data, properties);
}

bool DeviceModel::import_properties(const QList<QStringList> &csv_data, const std::map<QString, qsizetype> &properties) {
    qsizetype start_row;
    qsizetype end_row;
    qsizetype maxRow = csv_data.size();
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
    par.layer_type.resize(end_row - start_row + 1);
    for (const auto [i, rc] : std::views::enumerate(std::views::iota(start_row, end_row + 1))) {
        par.layer_type[static_cast<qsizetype>(i)] = csv_data.at(rc).at(layer_type_index);
    }
    try {
        // Material name array
        // This material list contains both layers and interfaces (typical structure has a size of 5).
        par.material = import_single_property<QString>(csv_data, properties, {"material", "stack"}, start_row, end_row);
        // Layer thickness array
        par.d = import_single_property<double>(csv_data, properties, {"dcell", "d", "thickness"}, start_row, end_row);
        const qsizetype sz_mat = par.material.size();
        if (par.d.size() not_eq sz_mat) {
            qWarning("Size of thickness does not match size of material!");
            return false;
        }
        if (start_row == 2 and end_row == maxRow - 2) {  // wl & RAT for this case only
            opt_material = import_single_property<QString>(csv_data, properties, {"material", "stack"}, 1, maxRow - 1);  // include electrodes
            opt_d = import_single_property<double>(csv_data, properties, {"dcell", "d", "thickness"}, 1, maxRow - 1);  // include front surface electrode
        }
        // Layer points array
        par.layer_points = import_single_property<qsizetype>(csv_data, properties, {"layer_points", "points"}, start_row, end_row);;
        // Spatial mesh coefficient for non-linear meshes
        par.xmesh_coeff = import_single_property<double>(csv_data, properties, {"xmesh_coeff"}, start_row, end_row);
        // Electron affinity array
        par.Phi_EA = import_single_property<double>(csv_data, properties, {"Phi_EA", "EA"}, start_row, end_row);
        QString side_str = csv_data.at(1).at(properties.at("side"));  // Row first!
        if (side_str == "right" or side_str.toDouble() == 2) {  // not toInt() or toUInt()!
            par.side = true;
        } else if (side_str == "left" or side_str.toDouble() == 1) {
            par.side = false;
        } else {
            qWarning("Side property is not correctly specified.");
            return false;
        }
    } catch (std::out_of_range &e) {
        // DEBUG -> INFO -> WARNING -> CRITICAL -> FATAL in <QtGlobal>
        qWarning() << "Out of range in readDfDev " << e.what();
        return false;
    }
    return true;
}

void DeviceModel::calcRAT() {
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
        return;
    }
    std::vector<std::pair<OpticMaterial<QList<double>> *, double>> structure;
    if (par.side) {  // right; need to reverse
        if (opt_material.back().isEmpty()) {  // cbegin() and cend() are not empty!
            opt_material.back() = "ITO";
        }
        if (opt_material.front().isEmpty()) {
            opt_material.front() = "Ag";
        }
        if (opt_d.back() == 0) {
            opt_d.back() = 5e-8;
        }
        structure.emplace_back(db_system->getMatByName(opt_material.back()), opt_d.back());
        for (qsizetype i = par.layer_type.size() - 1; i >= 0; i--) {  // does not include the substrate
            if (par.layer_type.at(i) not_eq "interface") {
                structure.emplace_back(db_system->getMatByName(opt_material.at(i + 1)), opt_d.at(i + 1));
            }
        }
    } else {
        if (opt_material.front().isEmpty()) {  // Default front surface
            opt_material.front() = "ITO";
        }
        if (opt_material.back().isEmpty()) {  // Default back surface
            opt_material.back() = "Ag";
        }
        if (opt_d.front() == 0) {  // Default front surface thickness
            opt_d.front() = 5e-8;
        }
        structure.emplace_back(db_system->getMatByName(opt_material.front()), opt_d.front());
        for (qsizetype i = 0; i < par.layer_type.size(); i++) {  // maxRow - 3
            if (par.layer_type.at(i) not_eq "interface") {
                structure.emplace_back(db_system->getMatByName(opt_material.at(i + 1)), opt_d.at(i + 1));
            }
        }
    }
    // For convenience, the wavelengths are expected to be sorted already, but still minmax here.
    // Since Ubuntu 24.04 has gcc libstdc++ 14, we are able to use std::ranges::to here for supported compilers.
    // https://en.cppreference.com/w/cpp/compiler_support
    const std::vector<std::pair<double, double>> minmax_wls = structure |
            std::views::transform([](const std::pair<OpticMaterial<QList<double>> *, double> &pair) -> std::pair<double, double> {
        const QList<double> q_wls = pair.first->nWl();
        const auto [min, max] = std::ranges::minmax_element(q_wls);
        return {*min, *max};  // Warning: do not return dangling or (const) iterators
    }) | std::ranges::to<std::vector<std::pair<double, double>>>();
    const auto [min_minmax_wl, max_min_max_wl] = std::ranges::minmax_element(minmax_wls);
    const double min_wl = min_minmax_wl->first;
    const double max_wl = max_min_max_wl->second;
    std::vector<double> wls_vec = Utils::Math::linspace(min_wl, max_wl, static_cast<std::size_t>((max_wl - min_wl) / 1e-9 + 1));
    wavelengths = {wls_vec.cbegin(), wls_vec.cend()};
    try {
        auto stack = par.side ? std::make_unique<OpticStack<QList<double>>>(std::move(structure), false, db_system->getMatByName(opt_material.back())) : std::make_unique<OpticStack<QList<double>>>(std::move(structure), false, db_system->getMatByName(material.front()));
        // calculate_rat<QList<double>&>
        const rat_dict<double> rat_out = calculate_rat(std::move(stack), wavelengths, 0, 's');
        const std::valarray<double> R_va = std::get<std::valarray<double>>(rat_out.at("R"));
        R = {std::begin(R_va), std::end(R_va)};
        const std::valarray<double> A_va = std::get<std::valarray<double>>(rat_out.at("A"));
        A = {std::begin(A_va), std::end(A_va)};
        const std::valarray<double> T_va = std::get<std::valarray<double>>(rat_out.at("T"));
        T = {std::begin(T_va), std::end(T_va)};
    } catch (std::runtime_error &e) {
        qWarning() << "Runtime error in calcRAT " << e.what();
        return;
    }
}
