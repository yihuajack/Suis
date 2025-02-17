//
// Created by Yihua Liu on 2024/3/31.
//

#include <QDirIterator>
#include <QFile>
#include "xlsxabstractsheet.h"
#include "xlsxdocument.h"
#include "xlsxworkbook.h"

#include "OpticMaterial.h"

template<FloatingList T>
QString OpticMaterial<T>::name() const {
    return mat_name;
}

template<FloatingList T>
T OpticMaterial<T>::wl() const {
    if (wl.empty()) {
        try {
            load_n();
        } catch (std::runtime_error& e) {
            qWarning() << "Material" << mat_name << "does not have wl defined." << e.what();
            return {};
        }
    }
    return wavelengths.back().second;
}

template<FloatingList T>
T OpticMaterial<T>::nData() const {
    if (n_data.empty()) {
        try {
            load_n();
        } catch (std::runtime_error& e) {
            qWarning() << "Material" << mat_name << "does not have n-data defined." << e.what();
            return {};
        }
    }
    return n_data.back().second;
}

template<FloatingList T>
T OpticMaterial<T>::kData() const {
    if (k_data.empty()) {
        try {
            load_k();
        } catch (std::runtime_error& e) {
            qWarning() << "Material" << mat_name << "does not have k-data defined." << e.what();
            return {};
        }
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
        QList<double> frac_wl;
        QList<double> frac_n_data;
        QList<double> frac_k_data;
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
    } else if (db_type == DbType::DF) {
        // Load DriftFusion's n data
        QXlsx::Document doc(path);
        if (not doc.load()) {
            throw std::runtime_error(std::format("Cannot load DriftFusion's material data file {}", path));
        }
        doc.selectSheet("data");
        // QXlsx::AbstractSheet is not a derived class of QObject
        const QXlsx::AbstractSheet *data_sheet = doc.sheet("data");
        if (data_sheet == nullptr) {
            throw std::runtime_error(std::format("Data sheet in data file %s does not exist! {}", path));
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
        QList<std::pair<double, QList<double>>> wls{{1, QList<double>(maxRow - 1)}};  // header by default
        for (int rc = 2; rc <= maxRow; rc++) {
            // QXlsx::Worksheet::cellAt() uses QMap find
            if (const QXlsx::Cell *cell = wsheet->cellAt(rc, 1); cell) {
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
                QList<double> n_list(maxRow - 1);
                QList<double> k_list(maxRow - 1);
                for (int rc = 2; rc <= maxRow; rc++) {
                    const QXlsx::Cell *cell = wsheet->cellAt(rc, cc);
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
