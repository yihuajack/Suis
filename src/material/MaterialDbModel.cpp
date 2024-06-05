//
// Created by Yihua Liu on 2024/6/4
//

#include <QDir>
#include <QFile>

#include "MaterialDbModel.h"

MaterialDbModel::MaterialDbModel(QObject *parent) : QObject(parent) {}

QStringList MaterialDbModel::readSolcoreDb(const QString &folderPath) {
    QDir db_dir(folderPath);
    if (not db_dir.exists()) {
        qWarning("Cannot find Solcore's material data directory.");
    }
    db_dir.setFilter(QDir::Dirs);
    QStringList name_filters;
    name_filters << "*-Material";
    db_dir.setNameFilters(name_filters);
    QStringList subdir_list = db_dir.entryList();  // db_dir.entryList(nameFilters, filter)
    for (const QString& subdir_name: subdir_list) {
        const QString mat_name = subdir_name.split('-').front();
        OpticMaterial opt_mat(DbType::SOLCORE, mat_name, subdir_name);
        m_list.insert(mat_name, opt_mat);
    }
    return subdir_list;
}

QStringList MaterialDbModel::readDfDb(const QString &filePath) {
    QFile db_file(filePath);
    if (not db_file.exists()) {
        qWarning("Cannot find DriftFusion's material data file.");
    }
    return {};
}
