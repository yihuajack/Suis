//
// Created by Yihua Liu on 2024-7-1.
//

#include <numeric>
#include <ranges>
#include <QDir>
#include <QUrl>
#include <QtGui/QGuiApplication>

#include "DbSysModel.h"
#include "DeviceModel.h"
#include "optics/TransferMatrix.h"

DeviceModel::DeviceModel(QObject *parent) : QAbstractTableModel(parent) {}

int DeviceModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return ParameterClass<QList, double, QString>::size;
}

int DeviceModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return static_cast<int>(par->col_size());
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
    if (not index.isValid() or index.row() >= ParameterClass<QList, double, QString>::size) {
        qWarning("QModelIndex of DeviceModel is invalid.");
        return {};
    }
    if (role == Qt::DisplayRole) {
        return par->get<QVariant>(index.row()).toList().at(index.column());
    }
    return {};
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    // if (not index.isValid() or index.row() >= ParameterClass<QList, double, QString, QVariant>::size or role not_eq Qt::EditRole) {
    //     return false;
    // }
    if (role == Qt::DisplayRole) {
        if (QVariant cellData = par->get<QVariant>(index.row()).toList().at(index.column()); value == cellData) {
            return false;
        }
        par->set(value, index.row(), index.column());
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

qsizetype DeviceModel::readColSize() const {
    return par->col_size();
}

QList<double> DeviceModel::readD() const {
    return par->d;
}

QList<double> DeviceModel::readCBM() const {
    return par->Phi_EA;
}

QList<double> DeviceModel::readVBM() const {
    return par->Phi_IP;
}

Q_INVOKABLE QVariant DeviceModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
    if (role not_eq Qt::DisplayRole) {
        return {};
    }
    return orientation == Qt::Horizontal ? par->layer_type.at(section) : par->headers.at(section);
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
    try {
        if (csv_data.at(1).at(properties.at("layer_type")) == "electrode") {  // wl & RAT for this case only
            opt_material = ParameterClass<QList, double, QString>::import_single_property<QString>(csv_data, properties, {"material", "stack"}, 1, csv_data.size() - 1);  // include electrodes
            opt_d = ParameterClass<QList, double, QString>::import_single_property<double>(csv_data, properties, {"dcell", "d", "thickness"}, 1, csv_data.size() - 1);  // include front surface electrode
        }
        par = std::make_unique<ParameterClass<QList, double, QString>>(csv_data, properties);
    } catch (std::out_of_range &e) {
        // DEBUG -> INFO -> WARNING -> CRITICAL -> FATAL in <QtGlobal>
        qWarning() << "Out of range in readDfDev" << e.what();
        return false;
    } catch (std::runtime_error &e) {
        qWarning() << "Runtime error in initializing PC" << e.what();
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
    if (par->side) {  // right; need to reverse
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
        for (qsizetype i = par->layer_type.size() - 1; i >= 0; i--) {  // does not include the substrate
            if (par->layer_type.at(i) not_eq "interface") {
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
        for (qsizetype i = 0; i < par->layer_type.size(); i++) {  // maxRow - 3
            if (par->layer_type.at(i) not_eq "interface") {
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
        auto stack = par->side ? std::make_unique<OpticStack<QList<double>>>(std::move(structure), false, db_system->getMatByName(opt_material.back())) :
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
