//
// Created by Yihua Liu on 2024/3/31.
//

#include <filesystem>
#include <QDir>

#include "OpticMaterial.h"

OpticMaterial::OpticMaterial(const DbType& db_type, QString mat_name, const QString& mat_path) : mat_name(std::move(mat_name)) {
    readMat(db_type, mat_path);
}

void OpticMaterial::readMat(const DbType& db_type, const QString& path) {
    const std::filesystem::path nk_path(path.toStdString());
    QFile n_file = nk_path / "n.txt";
    QFile k_file = nk_path / "k.txt";
    if (not n_file.open(QIODevice::ReadOnly) or not k_file.open(QIODevice::ReadOnly)) {
        qWarning() << n_file.errorString() << k_file.errorString();
        return;
    }
    QTextStream n_stream(&n_file);
    QTextStream k_stream(&k_file);
    while (not n_stream.atEnd()) {
        const QString n_line = n_stream.readLine();
        const QStringList n_linedata = n_line.split(' ');
        n_wl.push_back(n_linedata.front().toDouble());
        n_data.push_back(n_linedata.back().toDouble());
    }
    while (not k_stream.atEnd()) {
        const QString k_line = k_stream.readLine();
        const QStringList k_linedata = k_line.split(' ');
        k_wl.push_back(k_linedata.front().toDouble());
        k_data.push_back(k_linedata.back().toDouble());
    }
    n_file.close();
    k_file.close();
}
