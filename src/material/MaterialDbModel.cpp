//
// Created by Yihua Liu on 2024/6/4
//

#include <stdexcept>
#include <QDir>
#include <QFile>
#include <QUrl>

#include "IniConfigParser.h"
#include "MaterialDbModel.h"

MaterialDbModel::MaterialDbModel(QObject *parent) : QObject(parent) {}

QVariantMap MaterialDbModel::readSolcoreDb(const QString &db_path) {
    QUrl url(db_path);
    QFile ini_file = url.toLocalFile();
    QVariantMap result;
    QStringList mat_list;
    if (not ini_file.open(QIODevice::ReadOnly)) {
        qWarning("Cannot open the configuration ini file %s.", qUtf8Printable(db_path));
        result["status"] = 1;
        result["matlist"] = mat_list;
    } else {
        // db_dir.setFilter(QDir::Dirs);
        // QStringList name_filters;
        // name_filters << "*-Material";
        // db_dir.setNameFilters(name_filters);
        // for (const QFileInfo& subdir_info : subdir_list) {
        //     const QString mat_name = subdir_info.fileName().split('-').front();
        IniConfigParser solcore_config(ini_file.fileName());
        QVariantMap mat_map = solcore_config.loadGroup("Materials");
        mat_list = mat_map.keys();
        for (const QString &mat_name : mat_map.keys()) {
            try {
                QDir mat_dir = mat_map.value(mat_name).toString();
                QFileInfoList mat_flist = mat_dir.entryInfoList();
                OpticMaterial opt_mat(DbType::SOLCORE, mat_name, mat_list.front());
                m_list.insert(mat_name, opt_mat);
            } catch (std::runtime_error& e) {
                result["status"] = 2;
                result["matlist"] = mat_list;
                return result;
            }
        }
        result["status"] = 0;
        result["matlist"] = mat_list;
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
