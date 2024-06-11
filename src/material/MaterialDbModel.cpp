//
// Created by Yihua Liu on 2024/6/4.
//

#include <stdexcept>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QUrl>

#include "IniConfigParser.h"
#include "MaterialDbModel.h"
#include "ParameterSystem.h"

MaterialDbModel::MaterialDbModel(QObject *parent) : QObject(parent) {}

QVariantMap MaterialDbModel::readSolcoreDb(const QString &db_path) {
    const QUrl url(db_path);
    QFile ini_file = url.toLocalFile();
    const QFileInfo ini_finfo(ini_file);
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
        const ParameterSystem par_sys(solcore_config.loadGroup("Parameters"), ini_finfo.absolutePath());
        const QMap<QString, QString> mat_map = solcore_config.loadGroup("Materials");
        for (const QString &mat_name : mat_map.keys()) {
            try {
                const QDir mat_dir = mat_map.value(mat_name);
                if (par_sys.isComposition(mat_name, "x")) {
                    const QDir n_dir = mat_dir.filePath("n");
                    const QDir k_dir = mat_dir.filePath("k");
                    if (not n_dir.exists() or not k_dir.exists()) {
                        throw std::runtime_error("Cannot find n and k folder for composition material " + mat_name.toStdString());
                    }
                    const QFileInfoList n_flist = n_dir.entryInfoList();
                    const QFileInfoList k_flist = k_dir.entryInfoList();
                    std::vector<std::pair<double, std::vector<double>>> n_wl;
                    std::vector<std::pair<double, std::vector<double>>> n_data;
                    std::vector<std::pair<double, std::vector<double>>> k_wl;
                    std::vector<std::pair<double, std::vector<double>>> k_data;
                    for (const QFileInfo& n_info : n_flist) {
                        // not_eq "critical_points"
                        const QString main_fraction_str = n_info.baseName().split('_').front();
                        mat_list.append(mat_name + main_fraction_str);
                        QFile n_file(n_info.fileName());
                        if (not n_file.open(QIODevice::ReadOnly)) {
                            throw std::runtime_error("Cannot open file " + n_info.fileName().toStdString());
                        }
                        QTextStream n_stream(&n_file);
                        std::vector<double> frac_n_wl;
                        std::vector<double> frac_n_data;
                        while (not n_stream.atEnd()) {
                            const QString line = n_stream.readLine();
                            // Clazy: Don't create temporary QRegularExpression objects. Use a static QRegularExpression object instead
                            static const QRegularExpression ws_regexp("\\s+");
                            const QStringList ln_data = line.split(ws_regexp);
                            if (ln_data.length() not_eq 2) {
                                throw std::runtime_error("Error parsing file " + n_info.fileName().toStdString());
                            }
                            frac_n_wl.push_back(ln_data.front().toDouble());
                            frac_n_data.push_back(ln_data.back().toDouble());
                        }
                        n_file.close();
                        n_wl.emplace_back(main_fraction_str.toDouble(), frac_n_wl);
                        n_data.emplace_back(main_fraction_str.toDouble(), frac_n_data);
                    }
                    for (const QFileInfo& k_info : k_flist) {
                        // not_eq "critical_points"
                        const QString main_fraction_str = k_info.baseName().split('_').front();
                        mat_list.append(mat_name + main_fraction_str);
                        QFile k_file(k_info.fileName());
                        if (not k_file.open(QIODevice::ReadOnly)) {
                            throw std::runtime_error("Cannot open file " + k_info.fileName().toStdString());
                        }
                        QTextStream k_stream(&k_file);
                        std::vector<double> frac_k_wl;
                        std::vector<double> frac_k_data;
                        while (not k_stream.atEnd()) {
                            const QString line = k_stream.readLine();
                            static const QRegularExpression ws_regexp("\\s+");
                            const QStringList ln_data = line.split(ws_regexp);
                            if (ln_data.length() not_eq 2) {
                                throw std::runtime_error("Error parsing file " + k_info.fileName().toStdString());
                            }
                            frac_k_wl.push_back(ln_data.front().toDouble());
                            frac_k_data.push_back(ln_data.back().toDouble());
                        }
                        k_file.close();
                        k_wl.emplace_back(main_fraction_str.toDouble(), frac_k_wl);
                        k_data.emplace_back(main_fraction_str.toDouble(), frac_k_data);
                    }
                    CompOpticMaterial opt_mat(mat_name, n_wl, n_data, k_wl, k_data);
                    m_comp_list.insert(mat_name, opt_mat);
                }
            } catch (std::runtime_error& e) {
                qWarning(e.what());
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
