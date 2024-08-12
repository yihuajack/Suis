//
// Created by Yihua Liu on 2024-8-12.
//

#ifndef SUISAPP_CONNECTIONWIDGET_H
#define SUISAPP_CONNECTIONWIDGET_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTreeWidget)
QT_FORWARD_DECLARE_CLASS(QTreeWidgetItem)
QT_FORWARD_DECLARE_CLASS(QSqlDatabase)

class ConnectionWidget : public QWidget {
    Q_OBJECT

public:
    ConnectionWidget(QWidget *parent = nullptr);

    QSqlDatabase currentDatabase() const;

signals:
    void tableActivated(const QString &table);
    void metaDataRequested(const QString &tableName);

public slots:
    void refresh();
    void showMetaData();
    void onItemActivated(QTreeWidgetItem *item);
    void onCurrentItemChanged(QTreeWidgetItem *current);

private:
    void setActive(QTreeWidgetItem *);

    QTreeWidget *tree;
    QAction *metaDataAction;
    QString activeDb;
};

#endif  // SUISAPP_CONNECTIONWIDGET_H
