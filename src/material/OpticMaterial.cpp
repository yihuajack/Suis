//
// Created by Yihua Liu on 2024/3/31.
//

#include <format>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include "xlsxabstractsheet.h"
#include "xlsxdocument.h"
#include "xlsxworkbook.h"

#include "OpticMaterial.h"
#include "ParameterSystem.h"

template<FloatingList T>
QString OpticMaterial<T>::name() const {
    return mat_name;
}

template<FloatingList T>
T OpticMaterial<T>::wl() const {
    if (wavelengths.empty()) {
        qWarning() << "Material" << mat_name << "does not have wavelengths defined.";
        return {};
    }
    return wavelengths.back().second;
}

template<FloatingList T>
T OpticMaterial<T>::nData() const {
    if (n_data.empty()) {
        qWarning() << "Material" << mat_name << "does not have n data defined.";
        return {};
    }
    return n_data.back().second;
}

template<FloatingList T>
T OpticMaterial<T>::kData() const {
    if (k_data.empty()) {
        qWarning() << "Material" << mat_name << "does not have k data defined.";
        return {};
    }
    return k_data.back().second;
}

/*
 * Separately load n and k are suitable for Sopra, Solcore, etc. but not efficient for Df, etc.
 * The situation that n_data is empty while k_data is full or vice versa is seldom, and can be neglected.
 * Thus, to always load n and k at the same time seems reasonable, also to allow reloading is safer.
 */
template <FloatingList T>
void OpticMaterial<T>::load_nk() {
    QString line;
    QStringList ln_data;
    if (db_type == DbType::SOPRA) {
        // Load Sopra's n data
        QFile mat_file(path);
        if (not mat_file.open(QIODevice::ReadOnly)) {
            // Use site-packages db instead of source repo db to skip this high cost file I/O
            bool f_submat = false;
            QDirIterator dit(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (dit.hasNext()) {
                QString subDirPath = dit.next();
                QDir subDir(subDirPath);
                QStringList filters;
                filters << mat_name + ".MAT";
                subDir.setNameFilters(filters);

                if (QFileInfoList files = subDir.entryInfoList(QDir::Files); not files.isEmpty()) {
                    f_submat = true;
                    mat_file.setFileName(files.front().absolutePath());
                    break;
                }
            }
            if (not f_submat) {
                throw std::runtime_error("Cannot open file " + QFileInfo(mat_file).filePath().toStdString());
            }
        }
        QTextStream mat_stream(&mat_file);
        T frac_wl;
        T frac_n_data;
        T frac_k_data;
        mat_stream.readLine();  // VERSION
        mat_stream.readLine();  // FORMAT
        const QString points = mat_stream.readLine();  // POINTS
        const QStringList points_data = points.split('*', Qt::SkipEmptyParts);  // skip trailing asterisk
        if (points_data.length() not_eq 2) {  // if Qt::KeepEmptyParts, length should equal 3
            throw std::runtime_error("Error parsing file " + QFileInfo(mat_file).filePath().toStdString());
        }
        const std::size_t n_points = points_data.back().toULong();
        for (std::size_t i = 0; i < n_points; i++) {
            line = mat_stream.readLine();
            ln_data = line.split('*', Qt::SkipEmptyParts);
            if (ln_data.length() not_eq 5) {
                throw std::runtime_error("Error parsing file " + QFileInfo(mat_file).filePath().toStdString());
            }
            frac_wl.emplace_back(ln_data.at(3).toDouble());
            frac_n_data.emplace_back(ln_data.at(4).toDouble());
            frac_k_data.emplace_back(ln_data.back().toDouble());
        }
        mat_file.close();
        wavelengths.emplace_back(1, frac_wl);
        n_data.emplace_back(1, frac_n_data);
        k_data.emplace_back(1, frac_k_data);
    } else if (db_type == DbType::SOLCORE) {
        // Load Solcore's n data
        const QDir mat_dir(path);
        const ParameterSystem *par_sys = ParameterSystem::GetInstance();
        static const QRegularExpression ws_regexp("\\s+");
        // Note that same Solcore material has the same n_wl and k_wl even for different compositions, so there is
        // no need to store many n_wl and k_wl for one material.
        if (par_sys->isComposition(mat_name, "x")) {
            const QDir n_dir = mat_dir.filePath("n");
            const QDir k_dir = mat_dir.filePath("k");
            if (not n_dir.exists() or not k_dir.exists()) {
                throw std::runtime_error("Cannot find n and k folder for composition material " + mat_name.toStdString());
            }
            const QFileInfoList n_flist = n_dir.entryInfoList(QDir::Files);
            const QFileInfoList k_flist = k_dir.entryInfoList(QDir::Files);
            for (const QFileInfo& n_info : n_flist) {
                if (n_info.fileName() not_eq "critical_points.txt") {
                    // Warning: use completeBaseName() instead of baseName() to leave out all before the last dot!
                    const QString main_fraction_str = n_info.completeBaseName().split('_').front();
                    QFile n_file(n_info.filePath());  // use filePath() rather than fileName()!
                    if (not n_file.open(QIODevice::ReadOnly)) {
                        throw std::runtime_error("Cannot open file " + n_info.filePath().toStdString());
                    }
                    QTextStream n_stream(&n_file);
                    T frac_n_wl;
                    T frac_n_data;
                    while (not n_stream.atEnd()) {
                        line = n_stream.readLine();
                        // Clazy: Don't create temporary QRegularExpression objects.
                        // Use a static QRegularExpression object instead
                        ln_data = line.split(ws_regexp);
                        if (ln_data.length() not_eq 2) {
                            throw std::runtime_error("Error parsing file " + n_info.filePath().toStdString());
                        }
                        frac_n_wl.emplace_back(ln_data.front().toDouble());
                        frac_n_data.emplace_back(ln_data.back().toDouble());
                    }
                    n_file.close();
                    wavelengths.emplace_back(main_fraction_str.toDouble(), frac_n_wl);
                    n_data.emplace_back(main_fraction_str.toDouble(), frac_n_data);
                }
            }
            for (const QFileInfo& k_info : k_flist) {
                if (k_info.fileName() not_eq "critical_points.txt") {
                    const QString main_fraction_str = k_info.completeBaseName().split('_').front();
                    QFile k_file(k_info.filePath());
                    if (not k_file.open(QIODevice::ReadOnly)) {
                        throw std::runtime_error("Cannot open file " + k_info.filePath().toStdString());
                    }
                    QTextStream k_stream(&k_file);
                    T frac_k_wl;
                    T frac_k_data;
                    while (not k_stream.atEnd()) {
                        line = k_stream.readLine();
                        ln_data = line.split(ws_regexp);
                        if (ln_data.length() not_eq 2) {
                            throw std::runtime_error("Error parsing file " + k_info.filePath().toStdString());
                        }
                        frac_k_wl.emplace_back(ln_data.front().toDouble());
                        frac_k_data.emplace_back(ln_data.back().toDouble());
                    }
                    k_file.close();
                    if (wavelengths.empty()) {
                        wavelengths.emplace_back(main_fraction_str.toDouble(), frac_k_wl);
                    }
                    k_data.emplace_back(main_fraction_str.toDouble(), frac_k_data);
                }
            }
        } else {
            QFile n_file = mat_dir.filePath("n.txt");
            QFile k_file = mat_dir.filePath("k.txt");
            if (not n_file.open(QIODevice::ReadOnly)) {
                throw std::runtime_error("Cannot open file " + n_file.fileName().toStdString());
            }
            if (not k_file.open(QIODevice::ReadOnly)) {
                throw std::runtime_error("Cannot open file " + k_file.fileName().toStdString());
            }
            QTextStream n_stream(&n_file);
            QTextStream k_stream(&k_file);
            T frac_n_wl;
            T frac_n_data;
            T frac_k_wl;
            T frac_k_data;
            while (not n_stream.atEnd()) {
                line = n_stream.readLine();
                ln_data = line.split(ws_regexp);
                if (ln_data.length() not_eq 2) {
                    throw std::runtime_error("Error parsing file " + n_file.fileName().toStdString());
                }
                frac_n_wl.emplace_back(ln_data.front().toDouble());
                frac_n_data.emplace_back(ln_data.back().toDouble());
            }
            while (not k_stream.atEnd()) {
                line = k_stream.readLine();
                ln_data = line.split(ws_regexp);
                if (ln_data.length() not_eq 2) {
                    throw std::runtime_error("Error parsing file " + k_file.fileName().toStdString());
                }
                frac_k_wl.emplace_back(ln_data.front().toDouble());
                frac_k_data.emplace_back(ln_data.back().toDouble());
            }
            n_file.close();
            k_file.close();
            wavelengths.emplace_back(1, frac_n_wl);
            n_data.emplace_back(1, frac_n_data);
            k_data.emplace_back(1, frac_k_data);
        }
    } else if (db_type == DbType::DF) {
        // Load DriftFusion's n data
        QXlsx::Document doc(path);
        if (not doc.load()) {
            // Remember to toStdString()! Otherwise,
            // error C2039: "parse" is not a member of "std::formatter<
            //  std::__p2286::_Compile_time_parse_format_specs::_FormattedType,
            //  std::__p2286::_Compile_time_parse_format_specs::_CharT>"
            throw std::runtime_error(std::format("Cannot load DriftFusion's material data file {}", path.toStdString()));
        }
        doc.selectSheet("data");
        // QXlsx::AbstractSheet is not a derived class of QObject
        const QXlsx::AbstractSheet *data_sheet = doc.sheet("data");
        if (data_sheet == nullptr) {
            throw std::runtime_error(std::format("Data sheet in data file {} does not exist!", path.toStdString()));
        }
        data_sheet->workbook()->setActiveSheet(0);
        const auto *wsheet = dynamic_cast<QXlsx::Worksheet*>(data_sheet->workbook()->activeSheet());
        if (not wsheet) {
            throw std::runtime_error("Data sheet not found");
        }
        const int maxRow = wsheet->dimension().rowCount();
        const int maxCol = wsheet->dimension().columnCount();
        // QVector is an alias for QList.
        // QMapIterator<int, QMap<int, std::shared_ptr<Cell>>> iterates by rows
        // QList<QXlsx::CellLocation> clList = wsheet->getFullCells(&maxRow, &maxCol);
        // This approach costs more time, less space.
        QList<std::pair<double, T>> wls{{1, T(maxRow - 1)}};  // header by default
        for (int rc = 2; rc <= maxRow; rc++) {
            // QXlsx::Worksheet::cellAt() uses QMap find
            // Starting from QXlsx v1.5.0 cellAt(int row, int column) const returns std::shared_ptr<Cell> instead of
            // a raw pointer QXlsx::Worksheet::cellAt(). For compatibility, use auto.
            if (const auto cell = wsheet->cellAt(rc, 1); cell) {
                // QXlsx::Cell::readValue() will keep formula text!
                wls.front().second[rc - 2] = cell->value().toDouble() * 1e-9;
            }  // qDebug() << "Empty cell at Row " << rc << " Column " << 0;
        }
        // Table format has been checked in readDfDb()
        for (int cc = 2; cc < maxCol; cc += 2) {
            const QStringList mat_name_list = wsheet->cellAt(1, cc)->readValue().toString().split('_');
            const std::size_t mat_name_list_sz = mat_name_list.size();
            const QString& mat_name_header = mat_name_list.front();
            const double fraction = mat_name_list_sz == 2 ? 1 : mat_name_list.at(2).toDouble();
            if (mat_name == mat_name_header) {
                T n_list(maxRow - 1);
                T k_list(maxRow - 1);
                for (int rc = 2; rc <= maxRow; rc++) {
                    auto cell = wsheet->cellAt(rc, cc);
                    // std::shared_ptr<QXlsx::Cell> cell = clList.at(rc * maxCol + cc).cell;
                    if (cell) {
                        n_list[rc - 2] = cell->readValue().toDouble();
                    }  // qDebug() << "Empty cell at Row " << rc << " Column " << cc;
                    cell = wsheet->cellAt(rc, cc + 1);
                    if (cell) {
                        k_list[rc - 2] = cell->readValue().toDouble();
                    }
                }
                n_data.emplace_back(fraction, n_list);
                k_data.emplace_back(fraction, k_list);
            }
        }
    } else {
        throw std::runtime_error("Unknown database type.");
    }
}


template class OpticMaterial<QList<double>>;
