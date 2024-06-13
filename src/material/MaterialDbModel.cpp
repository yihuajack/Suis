//
// Created by Yihua Liu on 2024/6/4.
//

#include <stdexcept>
#include <QDir>
#include <QFile>
#include <QPointer>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include "xlsxabstractsheet.h"
#include "xlsxdocument.h"
#include "xlsxworkbook.h"

#include "IniConfigParser.h"
#include "MaterialDbModel.h"
#include "ParameterSystem.h"

MaterialDbModel::MaterialDbModel(QObject *parent) : QObject(parent) {}

QString findSolcoreUserConfig() {
    /* Let us expect P1031R2: Low level file i/o library (https://wg21.link/p1031r2)
     * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1031r2.pdf (https://ned14.github.io/llfio/)
    #ifdef _MSC_VER
    char *user_path;
    std::size_t len_userdata;
    errno_t err_userdata = _dupenv_s(&user_path, &len_userdata, "SUIS_USER_DATA");
    if (err_userdata) {
        throw std::runtime_error("_dupenv_s fails to get user data environment variable, errno is " + std::to_string(err_userdata) + ".");
    }
    #else
    char *user_path = std::getenv("SUIS_USER_DATA");
    #endif
    if (!user_path) {
        throw std::runtime_error("getenv fails to get user data environment variable.");
    }
    free(user_path);
     */
    const QProcessEnvironment sysenv = QProcessEnvironment::systemEnvironment();
    QString user_path_str = sysenv.value("SOLCORE_USER_DATA", "");
    QDir user_path;
    if (user_path_str.isEmpty()) {
        // the same as QDir::homePath()
        qDebug("SOLCORE_USER_DATA does not exist or is empty.");
        user_path = QStandardPaths::displayName(QStandardPaths::HomeLocation);
        user_path_str = user_path.filePath(".solcore");
        user_path = user_path_str;
    } else {
        user_path = user_path_str;
    }
    // std::filesystem::path::format has native_format, generic_format, and auto_format.
    // generic_format uses slashes, while native_format of Windows paths uses backslashes.
    // We can convert std::filesystem::path to std::string by std::filesystem::path::generic_string() bu
    // QDir() constructor accepts both QString and std::filesystem::path
    // if (not user_path.exists()) {
        // QDir::mkdir() is enough. Alternatively use QDir::mkpath() to create all parent directories.
        // user_path.mkdir(user_path_str);
    // }
    return user_path.filePath("solcore_config.txt");
}

QVariantMap MaterialDbModel::readSolcoreDb(const QString &db_path) {
    const QUrl url(db_path);
    const QString user_config = findSolcoreUserConfig();
    QFile ini_file = QFile::exists(user_config) ? user_config : url.toLocalFile();
    const QFileInfo ini_finfo(ini_file);
    QVariantMap result;
    QStringList mat_list;
    if (not ini_file.open(QIODevice::ReadOnly)) {
        qWarning("Cannot open the configuration ini file %s.", qUtf8Printable(db_path));
        result["status"] = 1;
        result["matlist"] = mat_list;
        return result;
    }
    // db_dir.setFilter(QDir::Dirs);
    // QStringList name_filters;
    // name_filters << "*-Material";
    // db_dir.setNameFilters(name_filters);
    // for (const QFileInfo& subdir_info : subdir_list) {
    //     const QString mat_name = subdir_info.fileName().split('-').front();
    IniConfigParser solcore_config(ini_file.fileName());
    const ParameterSystem par_sys(solcore_config.loadGroup("Parameters"), ini_finfo.absolutePath());
    const QMap<QString, QString> mat_map = solcore_config.loadGroup("Materials");
    for (const QString& mat_name : mat_map.keys()) {
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
    return result;
}

QVariantMap MaterialDbModel::readDfDb(const QString &db_path) {
    const QUrl url(db_path);
    QXlsx::Document doc(url.toLocalFile());
    QVariantMap result;
    QStringList mat_list;
    if (not doc.load()) {
        qWarning("Cannot load DriftFusion's material data file %s ", qUtf8Printable(db_path));
        result["status"] = 1;
        result["matlist"] = mat_list;
        return result;
    }
    doc.selectSheet("data");
    // QXlsx::AbstractSheet is not a derived class of QObject
    QXlsx::AbstractSheet *data_sheet = doc.sheet("data");
    if (data_sheet == nullptr) {
        qWarning("Data sheet in data file %s does not exist!", qUtf8Printable(db_path));
        result["status"] = 2;
        result["matlist"] = mat_list;
        return result;
    }
    data_sheet->workbook()->setActiveSheet(0);
    QXlsx::Worksheet *wsheet = (QXlsx::Worksheet *)data_sheet->workbook()->activeSheet();
    if (wsheet == nullptr) {
        qWarning("Data sheet not found");
        result["status"] = 2;
        result["matlist"] = mat_list;
        return result;
    }
    int maxRow = -1;  // qsizetype is long long (different from std::size_t)
    int maxCol = -1;
    // QVector is an alias for QList.
    QList<QXlsx::CellLocation> clList = wsheet->getFullCells(&maxRow, &maxCol);
    QList<double> wls(maxRow);
    qDebug() << "Max Row = " << maxRow;
    for (int hc = 0; hc < maxRow; hc++) {
        wls[hc] = clList.at(hc).cell->readValue().toDouble();
    }
    return result;
}
