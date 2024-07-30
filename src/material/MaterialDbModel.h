//
// Created by Yihua Liu on 2024/6/3.
//

#ifndef SUISAPP_MATERIALDBMODEL_H
#define SUISAPP_MATERIALDBMODEL_H

#include <QAbstractListModel>

#include "OpticMaterial.h"

class MaterialDbModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(double progress READ progress WRITE setProgress NOTIFY progressChanged)
    // Sources include QXlsx library headers so is hard to register by QML_ELEMENT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

public:
    enum ModelRoles {
        NameRole = Qt::UserRole + 1,
        NWlRole,
        NDataRole,
        KWlRole,
        KDataRole
    };

    explicit MaterialDbModel(QObject *parent = nullptr, QString name = "");

    // Default arguments on virtual or override methods are prohibited
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;  // parent = QModelIndex()
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;  // role = Qt::DisplayRole

    [[nodiscard]] double progress() const;
    void setProgress(double progress);
    [[nodiscard]] QString name() const;
    [[nodiscard]] bool checked() const;
    void setChecked(bool checked);
    [[nodiscard]] QString path() const;
    void setPath(const QString &path);

    Q_INVOKABLE int readSolcoreDb(const QString& db_path);
    Q_INVOKABLE int readDfDb(const QString& db_path);
    Q_INVOKABLE int readGCLDb(const QString& user_name, const QString& pw, const QString& db_path);

    [[nodiscard]] OpticMaterial<QList<double>> *getMatByName(const QString &mat_name) const;

signals:
    void progressChanged();
    void checkedChanged();
    void pathChanged();

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    // If using QObject, the values should be a pointer
    QMap<QString, OpticMaterial<QList<double>> *> m_list;

    double m_progress;
    QString m_name;
    bool m_checked{};
    QString m_path;
};

#endif  // SUISAPP_MATERIALDBMODEL_H
