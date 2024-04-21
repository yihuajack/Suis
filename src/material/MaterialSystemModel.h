//
// Created by Yihua Liu on 2024/4/8.
//

#ifndef SUISAPP_MATERIALSYSTEMMODEL_H
#define SUISAPP_MATERIALSYSTEMMODEL_H

#include <list>
#include <QAbstractListModel>

#include "CustomDbModel.h"
#include "DbModel.h"

class MaterialSystemModel : public QAbstractListModel {
    Q_OBJECT

private:
    DbModel SolcoreMatList;
    DbModel SopraMatList;
    DbModel SuisMatList;

    QMap<QString, DbModel*> m_list;
    // {"Df",      DfMatList},
    // {"Solcore", SolcoreMatList},
    // {"Sopra",   SopraMatList},
    // {"Suis",    SuisMatList}

public:
    enum Roles {
        DbNameRole = Qt::UserRole,
        DbImportedPathRole
    };

    MaterialSystemModel();
    [[nodiscard]] int rowCount(const QModelIndex &) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Q_INVOKABLE void addSpecies(const QString &species);
    Q_INVOKABLE void deleteSpecies(const QString &species, const int &rowIndex);
};


#endif  // SUISAPP_MATERIALSYSTEMMODEL_H
