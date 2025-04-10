//
// Created by Yihua Liu on 2024/6/4.
//

#include <map>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <QDir>
#include <QFile>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QString>
#include "xlsxabstractsheet.h"
#include "xlsxdocument.h"
#include "xlsxworkbook.h"

#include "IniConfigParser.h"
#include "MaterialDbModel.h"

#include "DbSysModel.h"
#include "ParameterSystem.h"

MaterialDbModel::MaterialDbModel(QObject *parent, QString name) : QAbstractListModel(parent), m_progress(0),
                                                                  m_name(std::move(name)) {}

int MaterialDbModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return static_cast<int>(m_list.size());  // count() is an overloaded function of size()
}

QVariant MaterialDbModel::data(const QModelIndex& index, int role) const {
    // 'operator+' is deprecated (in 6.2): Use std::next; QMap iterators are not random access
    QMap<QString, OpticMaterial<QList<double>> *>::const_iterator it = m_list.begin();
    std::advance(it, index.row());
    switch (role) {
        case NameRole:
            return it.key();
        case WlRole:
            return QVariant::fromValue(it.value()->wl());
        case NDataRole:
            return QVariant::fromValue(it.value()->nData());
        case KDataRole:
            return QVariant::fromValue(it.value()->kData());
        default:
            return {};
    }
}

double MaterialDbModel::progress() const {
    return m_progress;
}

void MaterialDbModel::setProgress(const double progress) {
    if (m_progress not_eq progress) {
        m_progress = progress;
        emit progressChanged();
    }
}

QString MaterialDbModel::name() const {
    return m_name;
}

bool MaterialDbModel::checked() const {
    return m_checked;
}

void MaterialDbModel::setChecked(const bool checked) {
    if (m_checked not_eq checked) {
        m_checked = checked;
        emit checkedChanged();
    }
}

QString MaterialDbModel::path() const {
    return m_path;
}

void MaterialDbModel::setPath(const QString &path) {
    if (m_path not_eq path) {
        m_path = path;
        emit pathChanged();
    }
}

QHash<int, QByteArray> MaterialDbModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[WlRole] = "wl";
    roles[NDataRole] = "n_data";
    roles[KDataRole] = "k_data";
    return roles;
}

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
        qDebug("SOLCORE_USER_DATA does not exist or is empty.");
        // the same as QDir::homePath()
        // writableLocation is just the highest priority path in the list of standardLocations
        // Do not use displayName()! It will return something like "主文件夹"
        user_path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);  // C:/Users/<username>
        user_path_str = user_path.filePath(".solcore");
        user_path = user_path_str;
    } else {
        qDebug("Found non-empty SOLCORE_USER_DATA path.");
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

int MaterialDbModel::readSolcoreDb(const QString& db_path) {
    using namespace Qt::Literals::StringLiterals;
    const QUrl url(db_path);
    QString db_path_imported = db_path;
    if (url.isLocalFile()) {
        db_path_imported = QDir::toNativeSeparators(url.toLocalFile());
    }
    const QString user_config = findSolcoreUserConfig();
    QFile ini_file;
    if (QFile::exists(user_config)) {
        qDebug() << "Using user configuration file " << user_config;
        ini_file.setFileName(user_config);
    } else {
        ini_file.setFileName(db_path_imported);
    }
    const QFileInfo ini_finfo(db_path_imported);  // for later get the parent directory as the root path
    if (not ini_finfo.exists() or not ini_finfo.isFile()) {
        qWarning("Database path is not an existing file!");
        return 1;
    }
    if (not ini_file.open(QIODevice::ReadOnly)) {
        qWarning("Cannot open the configuration ini file %s.", qUtf8Printable(db_path));
        return 1;
    }
    // db_dir.setFilter(QDir::Dirs);
    // QStringList name_filters;
    // name_filters << "*-Material";
    // db_dir.setNameFilters(name_filters);
    // for (const QFileInfo& subdir_info : subdir_list) {
    //     const QString mat_name = subdir_info.fileName().split('-').front();
    IniConfigParser solcore_config(ini_file.fileName());
    // The problem is, even though you can automatically read solcore_config.txt from the home location,
    // you have to manually set the SOLCORE_ROOT for parameter system, etc., so the import step cannot be omitted.
    const QMap<QString, QString> mat_map = solcore_config.loadGroup("Materials");
    const QMap<QString, QString> others_map = solcore_config.loadGroup("Others");
    ParameterSystem::SetInstance(solcore_config.loadGroup("Parameters"), ini_finfo.absolutePath());
    for (QMap<QString, QString>::const_iterator it = mat_map.cbegin(); it not_eq mat_map.cend(); ++it) {
        try {
            const QString& mat_name = it.key();
            QString mat_path = it.value();
            mat_path.replace("SOLCORE_ROOT", ini_finfo.absolutePath());
            auto *opt_mat = new OpticMaterial<QList<double>>(mat_name, DbType::SOLCORE, mat_path);
            beginInsertRows(QModelIndex(), static_cast<int>(m_list.size()), static_cast<int>(m_list.size()));
            m_list.insert(mat_name, opt_mat);
            endInsertRows();
            // emit dataChanged(index(0), index(static_cast<int>(m_list.size()) - 1));
            setProgress(static_cast<double>(std::distance(mat_map.cbegin(), it) + 1) / static_cast<double>(mat_map.size()));
        } catch (std::runtime_error& e) {
            qWarning() << e.what();
            return 2;
        }
    }
    // read SOPRA db embedded in solcore
    if (others_map.contains("sopra")) {
        for (const MaterialDbModel *db : DbSysModel::instance()->m_db) {
            if (db->name() == u"Sopra"_s and db->checked()) {
                QString sopra_path = others_map["sopra"];
                sopra_path.replace("SOLCORE_ROOT", ini_finfo.absolutePath());
                setPath(sopra_path);  // signal emitted
                return readSopraDb(sopra_path);
            }
        }
    }
    return 0;
}

// Optical Data from Sopra SA http://www.sspectra.com/sopra.html
int MaterialDbModel::readSopraDb(const QString& db_path) {
    const QDir sopra_dir(db_path);
    QFile sopra_db = sopra_dir.filePath("SOPRA_DB_Updated.csv");
    try {
        if (not sopra_db.open(QIODevice::ReadOnly)) {
            throw std::runtime_error("Cannot open file " + QFileInfo(sopra_db).filePath().toStdString());
        }
        QTextStream sopra_stream(&sopra_db);
        // std::array<std::vector<QString>, 4> info;
        sopra_stream.readLine();  // skip header
        while (not sopra_stream.atEnd()) {
            QString line = sopra_stream.readLine();
            QStringList ln_data = line.split(',');
            if (ln_data.length() not_eq 4 or ln_data.front() == "Filename") {
                continue;
            }
            const QString& mat_name = ln_data.front();
            const QString path = sopra_dir.filePath(mat_name + ".MAT");
            // info.front().emplace_back(ln_data.at(2));  // Material
            // info.at(2).emplace_back(ln_data.at(3));  // Wavelength (nm)
            // info.at(3).emplace_back(ln_data.back());  // File Info
            // info.back().emplace_back(path);  // File Path
            try {
                auto *opt_mat = new OpticMaterial<QList<double>>(mat_name, DbType::SOPRA, path);
                beginInsertRows(QModelIndex(), static_cast<int>(m_list.size()), static_cast<int>(m_list.size()));
                m_list.insert(mat_name, opt_mat);
                endInsertRows();
            } catch (std::runtime_error &e) {
                qWarning() << e.what();
                return 2;
            }
        }
    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 1;
    }
    return 0;
}

int MaterialDbModel::readDfDb(const QString& db_path) {
    const QUrl url(db_path);
    QString db_path_imported = db_path;
    if (url.isLocalFile()) {
        db_path_imported = QDir::toNativeSeparators(url.toLocalFile());
    }
    QXlsx::Document doc(db_path_imported);
    if (not doc.load()) {
        qWarning("Cannot load DriftFusion's material data file %s ", qUtf8Printable(db_path));
        return 1;
    }
    doc.selectSheet("data");
    // QXlsx::AbstractSheet is not a derived class of QObject
    const QXlsx::AbstractSheet *data_sheet = doc.sheet("data");
    if (data_sheet == nullptr) {
        qWarning("Data sheet in data file %s does not exist!", qUtf8Printable(db_path));
        return 2;
    }
    data_sheet->workbook()->setActiveSheet(0);
    const auto *wsheet = dynamic_cast<QXlsx::Worksheet*>(data_sheet->workbook()->activeSheet());
    if (not wsheet) {
        qWarning("Data sheet not found");
        return 2;
    }
    // const int maxRow = wsheet->dimension().rowCount();  // qsizetype is long long (different from std::size_t)
    const int maxCol = wsheet->dimension().columnCount();
    std::unordered_set<QString> mat_name_set;
    // Scan the header first.
    for (int cc = 2; cc < maxCol; cc += 2) {
        // const QString mat_name = clList.at(cc).cell->readValue().toString();
        const QStringList mat_name_list = wsheet->cellAt(1, cc)->readValue().toString().split('_');
        const QStringList mat_name_list2 = wsheet->cellAt(1, cc + 1)->readValue().toString().split('_');
        const std::size_t mat_name_list_sz = mat_name_list.size();
        const QString& mat_name = mat_name_list.front();
        if (mat_name_list.back() not_eq "n") {
            qWarning("Header at Column %d not ended with n", cc);
        } else if (mat_name_list2.back() not_eq "k") {
            qWarning("Header at Column %d not ended with k", cc + 1);
        } else if (mat_name_list_sz not_eq 2 and mat_name_list_sz not_eq 3) {
            qWarning("Invalid header at Column %d", cc);
        } else if (mat_name not_eq mat_name_list2.front()) {
            qWarning("Adjacent columns %d and %d are different materials", cc, cc + 1);
        }
        // const double fraction = mat_name_list_sz == 2 ? 1 : mat_name_list.at(2).toDouble();
        if (mat_name_set.contains(mat_name)) {
            if (mat_name_list_sz == 2) {
                qWarning("Duplicate header detected at column %d", cc);
            }
        } else {
            mat_name_set.insert(mat_name);
            // std::vector<double> n_wl = {wls.begin(), wls.begin() + static_cast<std::vector<double>::difference_type>(n_list.size())};
            // std::vector<double> k_wl = {wls.begin(), wls.begin() + static_cast<std::vector<double>::difference_type>(k_list.size())};
            // Warning: must dynamically new the object! Do not insert a reference; otherwise, it will change for each loop!
            // You are using wls multiple times! Do not try to move wls to k_wl!
            // Otherwise, qlist.h inline T& last() { Q_ASSERT(!isEmpty()); return *(end()-1); } assertion will fail.
            // auto *opt_mat = new OpticMaterial<QList<double>>(it.key(), wls, std::move(n_series), wls, std::move(k_series));
            auto *opt_mat = new OpticMaterial<QList<double>>(mat_name, DbType::DF, db_path_imported);
            beginInsertRows(QModelIndex(), static_cast<int>(m_list.size()), static_cast<int>(m_list.size()));
            m_list.insert(mat_name, opt_mat);
            endInsertRows();
        }
        setProgress(static_cast<double>(cc + 2) / static_cast<double>(maxCol));
    }
    return 0;
}

int MaterialDbModel::readGclDb(const QString& transfer_path) {

    return 0;
}

OpticMaterial<QList<double>> *MaterialDbModel::getMatByName(const QString &mat_name) const {
    if (m_list.find(mat_name) not_eq m_list.cend()) {
        OpticMaterial<QList<double>> *opt_mat = m_list[mat_name];
        return opt_mat;
    }
    qDebug() << mat_name << "not found in MaterialDbModel" << m_name;
    return nullptr;
}
