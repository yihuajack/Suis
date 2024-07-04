//
// Created by Yihua Liu on 2024-7-1.
//

#include <set>
#include <QDir>
#include <QUrl>

#include "DeviceModel.h"

DeviceModel::DeviceModel(QObject *parent) : QAbstractTableModel(parent) {}

int DeviceModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return static_cast<int>(m_data.size());
}

int DeviceModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_data.empty() ? 0 : static_cast<int>(m_data["layer_type"].size());
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
    QMap<QString, QList<QVariant>>::const_iterator it = m_data.cbegin();
    std::advance(it, index.row());
    switch (role) {
        case NameRole:
            return it.key();
        default:
            return {};
    }
}

double DeviceModel::getProgress() const {
    return import_progress;
}

void DeviceModel::setProgress(double progress) {
    if (import_progress not_eq progress) {
        import_progress = progress;
        emit progressChanged(import_progress);
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    return roles;
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

int DeviceModel::readDfDb(const QString &db_path) {
    const QUrl url(db_path);
    QString db_path_imported = db_path;
    if (url.isLocalFile()) {
        db_path_imported = QDir::toNativeSeparators(url.toLocalFile());
    }
    QFile doc(db_path_imported);
    if (not doc.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot load DriftFusion's device data file %s ", qUtf8Printable(db_path));
        return 1;
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
        if (csv_data.at(2).at(layer_type_index) == "electrode") {
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
        return 2;
    }
    try {
        QList<QString> material = import_single_property<QString>(csv_data, properties, {"material", "stack"}, start_row, end_row);
        QList<double> d = import_single_property<double>(csv_data, properties, {"dcell", "d", "thickness"}, start_row, end_row);
    } catch (std::out_of_range &e) {
        qWarning(e.what());
        return 2;
    }
    return 0;
}
