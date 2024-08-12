//
// Created by Yihua Liu on 2024-8-12.
//

#include "ConnectionWidget.h"

#include <QAction>
#include <QHeaderView>
#include <QSqlDatabase>
#include <QTreeWidget>
#include <QVBoxLayout>

ConnectionWidget::ConnectionWidget(QWidget *parent)
        : QWidget(parent)
        , tree(new QTreeWidget(this)) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    tree->setHeaderLabels(QStringList(tr("Database")));
    tree->header()->setStretchLastSection(true);
    QAction *refreshAction = new QAction(tr("Refresh"), tree);
    metaDataAction = new QAction(tr("Show Schema"), tree);
    connect(refreshAction, &QAction::triggered, this, &ConnectionWidget::refresh);
    connect(metaDataAction, &QAction::triggered, this, &ConnectionWidget::showMetaData);
    tree->addAction(refreshAction);
    tree->addAction(metaDataAction);
    tree->setContextMenuPolicy(Qt::ActionsContextMenu);

    layout->addWidget(tree);

    connect(tree, &QTreeWidget::itemActivated,
            this, &ConnectionWidget::onItemActivated);
    connect(tree, &QTreeWidget::currentItemChanged,
            this, &ConnectionWidget::onCurrentItemChanged);
}

void ConnectionWidget::refresh() {
    const auto qDBCaption = [](const QSqlDatabase &db) -> QString{
        QString nm = db.driverName() + QLatin1Char(':');
        if (!db.userName().isEmpty())
            nm += db.userName() + QLatin1Char('@');
        nm += db.databaseName();
        return nm;
    };

    tree->clear();
    const QStringList connectionNames = QSqlDatabase::connectionNames();

    bool gotActiveDb = false;
    for (const QString &connectionName : connectionNames) {
        QTreeWidgetItem *root = new QTreeWidgetItem(tree);
        QSqlDatabase db = QSqlDatabase::database(connectionName, false);
        root->setText(0, qDBCaption(db));
        if (connectionName == activeDb) {
            gotActiveDb = true;
            setActive(root);
        }
        if (db.isOpen()) {
            QStringList tables = db.tables();
            for (int t = 0; t < tables.count(); ++t) {
                QTreeWidgetItem *table = new QTreeWidgetItem(root);
                table->setText(0, tables.at(t));
            }
        }
    }
    if (not gotActiveDb) {
        activeDb = connectionNames.value(0);
        setActive(tree->topLevelItem(0));
    }

    tree->doItemsLayout();  // HACK
}

QSqlDatabase ConnectionWidget::currentDatabase() const {
    return QSqlDatabase::database(activeDb);
}

void ConnectionWidget::setActive(QTreeWidgetItem *item) {
    const auto qSetBold = [](QTreeWidgetItem *item, bool bold) -> void {
        QFont font = item->font(0);
        font.setBold(bold);
        item->setFont(0, font);
    };

    for (int i = 0; i < tree->topLevelItemCount(); i++) {
        if (tree->topLevelItem(i)->font(0).bold()) {
            qSetBold(tree->topLevelItem(i), false);
        }
    }

    if (not item) {
        return;
    }

    qSetBold(item, true);
    activeDb = QSqlDatabase::connectionNames().value(tree->indexOfTopLevelItem(item));
}

void ConnectionWidget::onItemActivated(QTreeWidgetItem *item) {
    if (not item) {
        return;
    }

    if (not item->parent()) {
        setActive(item);
    } else {
        setActive(item->parent());
        emit tableActivated(item->text(0));
    }
}

void ConnectionWidget::showMetaData() {
    QTreeWidgetItem *cItem = tree->currentItem();
    if (!cItem || !cItem->parent())
        return;
    setActive(cItem->parent());
    emit metaDataRequested(cItem->text(0));
}

void ConnectionWidget::onCurrentItemChanged(QTreeWidgetItem *current) {
    metaDataAction->setEnabled(current && current->parent());
}
