//
// Created by Yihua Liu on 2024-7-1.
//

#ifndef SUISAPP_DEVICEMODEL_H
#define SUISAPP_DEVICEMODEL_H

#include <QAbstractTableModel>
#include <QQmlEngine>

class DeviceModel : public QAbstractTableModel {
    Q_OBJECT
    Q_PROPERTY(double progress READ getProgress WRITE setProgress NOTIFY progressChanged)
    QML_ELEMENT

public:
    enum ModelRoles {
        NameRole = Qt::UserRole + 1
    };

    explicit DeviceModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

    [[nodiscard]] double getProgress() const;
    void setProgress(double progress);

    Q_INVOKABLE int readDfDb(const QString &db_path);

signals:
    void progressChanged(double progress);

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    QMap<QString, QList<QVariant>> m_data;
    double import_progress;
};

#endif  // SUISAPP_DEVICEMODEL_H
