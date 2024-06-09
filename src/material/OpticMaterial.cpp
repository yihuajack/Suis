//
// Created by Yihua Liu on 2024/3/31.
//

#include <stdexcept>
#include <QDir>

#include "OpticMaterial.h"

OpticMaterial::OpticMaterial(const DbType& db_type, QString mat_name, const QFileInfo& mat_f) : mat_name(std::move(mat_name)) {
    readMat(db_type, mat_f);
}

void OpticMaterial::readMat(const DbType& db_type, const QFileInfo& mat_f) {
    if (db_type == DbType::SOLCORE) {
        QDir mat_dir = mat_f.absoluteFilePath();  // including the entry's name
        if (not mat_dir.exists()) {
            qWarning() << mat_dir.absolutePath();
            throw std::runtime_error("mat_dir does not exist!");
        }
        QFile n_file = mat_dir.filePath("n.txt");
        QFile k_file = mat_dir.filePath("k.txt");
        if (not n_file.open(QIODevice::ReadOnly)) {
            qWarning() << n_file.errorString() << n_file.fileName();
            throw std::runtime_error((n_file.errorString() + n_file.fileName()).toStdString());
        }
        if (not k_file.open(QIODevice::ReadOnly)) {
            qWarning() << k_file.errorString() << k_file.fileName();
            throw std::runtime_error((k_file.errorString() + k_file.fileName()).toStdString());
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
}
