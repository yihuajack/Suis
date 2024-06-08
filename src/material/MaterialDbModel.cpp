//
// Created by Yihua Liu on 2024/6/4
//

#include <stdexcept>
#include <QDir>
#include <QFile>
#include <QUrl>

#include "MaterialDbModel.h"

MaterialDbModel::MaterialDbModel(QObject *parent) : QObject(parent) {}

QVariantMap MaterialDbModel::readSolcoreDb(const QString &folderPath) {
    QUrl url(folderPath);
    QDir db_dir = url.toLocalFile();
    QVariantMap result;
    if (not db_dir.exists()) {
        qWarning("Cannot find Solcore's material data directory %s.", qUtf8Printable(folderPath));
        result["status"] = 1;
        result["matlist"] = "";
    } else {
        db_dir.setFilter(QDir::Dirs);
        QStringList name_filters;
        name_filters << "*-Material";
        db_dir.setNameFilters(name_filters);
        QStringList subdir_list = db_dir.entryList();  // db_dir.entryList(nameFilters, filter)
        for (const QString& subdir_name: subdir_list) {
            const QString mat_name = subdir_name.split('-').front();
            try {
                OpticMaterial opt_mat(DbType::SOLCORE, mat_name, subdir_name);
                m_list.insert(mat_name, opt_mat);
            } catch (std::runtime_error& e) {
                result["status"] = 2;
                result["matlist"] = "";
                return result;
            }
        }
        result["status"] = 0;
        result["matlist"] = subdir_list;
    }
    return result;
}

QVariantMap MaterialDbModel::readDfDb(const QString &filePath) {
    QFile db_file(filePath);
    if (not db_file.exists()) {
        qWarning("Cannot find DriftFusion's material data file %s.", qUtf8Printable(filePath));
    }
    return {};
}
