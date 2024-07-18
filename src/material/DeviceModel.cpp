//
// Created by Yihua Liu on 2024-7-1.
//

#include <numeric>
#include <ranges>
#include <set>
#include <QDir>
#include <QUrl>
#include <QtGui/QGuiApplication>

#include "DbSysModel.h"
#include "DeviceModel.h"
#include "optics/TransferMatrix.h"

DeviceModel::DeviceModel(QObject *parent) : QAbstractTableModel(parent) {}

int DeviceModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return static_cast<int>(ParameterClass<QList, double, QString, QVariant>::size);
}

int DeviceModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return static_cast<int>(par.layer_type.size());
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
    if (not index.isValid() or index.row() >= ParameterClass<QList, double, QString, QVariant>::size) {
        qWarning("QModelIndex of DeviceModel is invalid.");
        return {};
    }
    if (role == Qt::DisplayRole) {
        return par.get(index.row()).toList().at(index.column());
    } else {
        return {};
    }
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    // if (not index.isValid() or index.row() >= ParameterClass<QList, double, QString, QVariant>::size or role not_eq Qt::EditRole) {
    //     return false;
    // }
    if (role == Qt::DisplayRole) {
        QVariant cellData = par.get(index.row()).toList().at(index.column());
        if (value == cellData) {
            return false;
        }
        par.set(value, index.row(), index.column());
    }
    emit dataChanged(index, index, {role});
    qInfo("setData of DeviceModel");
    return true;
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex &index) const {
    // if (not index.isValid()) {
    //     return Qt::NoItemFlags;
    // }
    // return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QString DeviceModel::name() const {
    return m_name;
}

QList<double> DeviceModel::wavelength() const {
    return wavelengths;
}

QList<double> DeviceModel::readR() const {
    return R;
}

QList<double> DeviceModel::readA() const {
    return A;
}

QList<double> DeviceModel::readT() const {
    return T;
}

Q_INVOKABLE QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role not_eq Qt::DisplayRole) {
        return {};
    }
    return orientation == Qt::Horizontal ? par.layer_type.at(section) : par.headers.at(section);
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
                    QString double_str = data.at(rc).at(index);
                    property[i++] = double_str.isEmpty() ? NAN : double_str.toDouble();
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

Q_INVOKABLE bool DeviceModel::readDfDev(const QString &db_path) {
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
    m_name = QFileInfo(doc).baseName();
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
    bool has_electrodes = false;
    if (csv_data.at(1).at(layer_type_index) == "electrode") {
        start_row = 2;
    } else {  // no front surface electrode
        start_row = 1;
        qWarning("Missing front surface electrode; RAT calculation is disabled.");
    }
    if (csv_data.at(maxRow - 1).at(layer_type_index) == "electrode") {
        end_row = maxRow - 2;
        has_electrodes = start_row == 2;
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
        if (has_electrodes) {  // wl & RAT for this case only
            opt_material = import_single_property<QString>(csv_data, properties, {"material", "stack"}, 1, maxRow - 1);  // include electrodes
            opt_d = import_single_property<double>(csv_data, properties, {"dcell", "d", "thickness"}, 1, maxRow - 1);  // include front surface electrode
        }
        // Layer points array
        par.layer_points = import_single_property<qsizetype>(csv_data, properties, {"layer_points", "points"}, start_row, end_row);;
        // Spatial mesh coefficient for non-linear meshes
        par.xmesh_coeff = import_single_property<double>(csv_data, properties, {"xmesh_coeff"}, start_row, end_row);
        // Electron affinity array
        par.Phi_EA = import_single_property<double>(csv_data, properties, {"Phi_EA", "EA"}, start_row, end_row);
        // Ionization potential array
        par.Phi_IP = import_single_property<double>(csv_data, properties, {"Phi_IP", "IP"}, start_row, end_row);
        // SRH Trap Energy
        par.Et = import_single_property<double>(csv_data, properties, {"Et", "Et_bulk"}, start_row, end_row);
        // Equilibrium Fermi energy array
        QList<double> EF0 = import_single_property<double>(csv_data, properties, {"EF0", "E0"}, 1, maxRow - 1);
        par.EF0 = EF0.mid(start_row, end_row - start_row + 1);
        par.Phi_left = EF0.front();
        par.Phi_right = EF0.back();
        // Conduction band effective density of states
        par.Nc = import_single_property<double>(csv_data, properties, {"Nc", "Ncb", "NC", "NCB"}, start_row, end_row);
        // Valence band effective density of states
        par.Nv = import_single_property<double>(csv_data, properties, {"Nv", "Ncb", "NV", "NVB"}, start_row, end_row);
        // Intrinsic anion density
        par.Nani = import_single_property<double>(csv_data, properties, {"Nani"}, start_row, end_row);
        // Intrinsic cation density
        par.Ncat = import_single_property<double>(csv_data, properties, {"Ncat", "Nion"}, start_row, end_row);
        // Limiting density of anion states
        par.a_max = import_single_property<double>(csv_data, properties, {"a_max", "amax", "DOSani"}, start_row, end_row);
        // Limiting density of cation states
        par.c_max = import_single_property<double>(csv_data, properties, {"c_max", "cmax", "DOScat"}, start_row, end_row);
        // Electron mobility
        par.mu_n = import_single_property<double>(csv_data, properties, {"mu_n", "mun", "mue", "mu_e"}, start_row, end_row);
        // Hole mobility
        par.mu_p = import_single_property<double>(csv_data, properties, {"mu_p", "mup", "muh", "mu_h"}, start_row, end_row);
        // Anion mobility
        par.mu_a = import_single_property<double>(csv_data, properties, {"mu_a", "mua", "mu_ani", "muani"}, start_row, end_row);
        // Cation mobility
        par.mu_c = import_single_property<double>(csv_data, properties, {"mu_c", "muc", "mu_cat", "mucat"}, start_row, end_row);
        // Relative dielectric constant
        par.epp = import_single_property<double>(csv_data, properties, {"epp", "eppr"}, start_row, end_row);
        // Uniform volumetric generation rate
        par.g0 = import_single_property<double>(csv_data, properties, {"g0", "G0"}, start_row, end_row);
        // Band-to-band recombination coefficient
        par.B = import_single_property<double>(csv_data, properties, {"B", "krad", "kbtb"}, start_row, end_row);
        // Electron SRH time constant
        par.taun = import_single_property<double>(csv_data, properties, {"taun", "taun_SRH"}, start_row, end_row);
        // Hole SRH time constant
        par.taup = import_single_property<double>(csv_data, properties, {"taup", "taup_SRH"}, start_row, end_row);
        // Electron and hole surface recombination velocities
        if (has_electrodes) {
            QList<double> sn = import_single_property<double>(csv_data, properties, {"sn"}, 1, maxRow - 1);
            par.sn = sn.mid(start_row, end_row - start_row + 1);
            par.sn_l = sn.front();
            par.sn_r = sn.back();
            QList<double> sp = import_single_property<double>(csv_data, properties, {"sp"}, 1, maxRow - 1);
            par.sp = sp.mid(start_row, end_row - start_row + 1);
            par.sp_l = sp.front();
            par.sp_r = sp.back();
        } else {
            par.sn = import_single_property<double>(csv_data, properties, {"sn"}, start_row, end_row);
            par.sp = import_single_property<double>(csv_data, properties, {"sp"}, start_row, end_row);
        }
        QString optical_model_str;
        std::map<QString, qsizetype>::const_iterator pit = properties.find("optical_model");
        if (pit not_eq properties.cend()) {
            optical_model_str = csv_data.at(1).at(pit->second);
        } else if (pit = properties.find("OM"); pit not_eq properties.cend()) {
            optical_model_str = csv_data.at(1).at(pit->second);
        }
        if (optical_model_str == "uniform" or optical_model_str.toDouble() == 0) {
            par.optical_model = false;
        } else if (optical_model_str == "Beer-Lambert" or optical_model_str.toDouble() == 1) {
            par.optical_model = true;
        } else {
            qWarning("optical_model not recognized - defaulting to 'Beer-Lambert'");
        }
        // Illumination side
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

Q_INVOKABLE void DeviceModel::calcRAT() {
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
        if (std::isnan(opt_d.back())) {
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
        auto stack = par.side ? std::make_unique<OpticStack<QList<double>>>(std::move(structure), false, db_system->getMatByName(opt_material.back())) :
                std::make_unique<OpticStack<QList<double>>>(std::move(structure), false, db_system->getMatByName(opt_material.front()));
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
