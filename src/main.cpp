// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "app_environment.h"
#include "import_qml_components_plugins.h"
#include "import_qml_plugins.h"

#include "backend_init.h"

int main(int argc, char *argv[])
{
    set_qt_environment();

    QGuiApplication::setOrganizationName(QStringLiteral("Yihua Liu"));
    QGuiApplication::setApplicationName(QStringLiteral("Suis"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("Suis - Solar Cell Simulator"));

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // [cmake] CMake Warning (dev) at D:/Qt/6.6.2/msvc2019_64/lib/cmake/Qt6Core/Qt6CoreMacros.cmake:2829 (message):
    // [cmake]   Qt policy QTP0001 is not set: ':/qt/qml/' is the default resource prefix
    // [cmake]   for QML modules.  Check https://doc.qt.io/qt-6/qt-cmake-policy-qtp0001.html
    // [cmake]   for policy details.  Use the qt_policy command to set the policy and
    // [cmake]   suppress this warning.
    // Use this Qurl instead of "qrc:Main/main.qml"
    const QUrl url(u"qrc:/qt/qml/com/github/yihuajack/main.qml"_qs);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
