// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "app_environment.h"
#include "import_qml_components_plugins.h"
#include "import_qml_plugins.h"

#include "Application.h"
#include "material/DbSysModel.h"
#include "sql/SqlTreeModel.h"

int main(int argc, char *argv[]) {
    set_qt_environment();

    // qt.qpa.wayland: qtvirtualkeyboard currently is not supported at client-side, use QT_IM_MODULE=qtvirtualkeyboard at compositor-side.
    // terminate called after throwing an instance of 'std::filesystem::__cxx11::filesystem_error'
    //     what():  filesystem error: cannot make absolute path: Invalid argument []
    // https://forum.qt.io/topic/148045/windows-to-linux-lost-virtualkeyboard
    // https://bugreports.qt.io/browse/QTBUG-94994
    // qputenv("QT_IM_MODULE", "qtvirtualkeyboard");

    // Application organization name would affect QSettings locations.
    // QGuiApplication::setOrganizationName(QStringLiteral("Yihua Liu"));
    QGuiApplication::setApplicationName(QStringLiteral("Suis"));
    QGuiApplication::setOrganizationDomain(QStringLiteral("yihuajack.github.io"));  // for macOS/iOS
    QGuiApplication::setApplicationDisplayName(QStringLiteral("Suis - Solar Cell Simulator"));
    QGuiApplication::setApplicationVersion("1.0");

    /*
     * https://codereview.qt-project.org/c/qt/qtdeclarative/+/536412
     * https://doc.qt.io/qt-6/qtquickcontrols-customize.html
     * Avoid using qmlRegisterType and friends to expose C++ to QML
     * Instead, promote the QML_ELEMENT macro and how to edit the build files accordingly.
     * https://doc.qt.io/qt-6/qtqml-cppintegration-overview.html
     * [Defining QML Types from C++](https://doc.qt.io/qt-6/qtqml-cppintegration-definetypes.html)
     * https://scythe-studio.com/en/blog/how-to-integrate-qml-and-c-expose-object-and-register-c-class-to-qml
     * (Not recommended) [Embedding C++ Objects into QML with Context Properties]
     * (https://doc.qt.io/qt-6/qtqml-cppintegration-contextproperties.html)
     * qmlRegisterSingletonType vs qmlRegisterSingletonInstance:
     * It seems that unless separating singleton instances for multiple engines is needed,
     * just use qmlRegisterSingletonInstance. Note: cppObject must outlive the QML engine in which it is used.
     * QScopedPointer<DbSysModel> db_system(new DbSysModel);
     * qmlRegisterSingletonInstance("com.github.yihuajack.DbSysModel", 1, 0, "DbSysModel", db_system.get());
     * DbSysModel needs this approach because it includes QXlsx headers
     */
    qmlRegisterSingletonType<DbSysModel>("com.github.yihuajack.DbSysModel", 1, 0,
                                         "DbSysModel",
                                         [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)  // qobject_singletontype_provider
        Q_UNUSED(scriptEngine)

        return DbSysModel::instance();  // auto *db_system = new DbSysModel();
    });

    // qApp and qGuiApp are predefined macros in qguiapplication.h!
    std::unique_ptr<Application> app;
    app = std::make_unique<Application>(argc, argv);

    QQmlApplicationEngine engine;

    // [cmake] CMake Warning (dev) at D:/Qt/6.6.2/msvc2019_64/lib/cmake/Qt6Core/Qt6CoreMacros.cmake:2829 (message):
    // [cmake]   Qt policy QTP0001 is not set: ':/qt/qml/' is the default resource prefix
    // [cmake]   for QML modules.  Check https://doc.qt.io/qt-6/qt-cmake-policy-qtp0001.html
    // [cmake]   for policy details.  Use the qt_policy command to set the policy and
    // [cmake]   suppress this warning.
    // Use this QUrl instead of "qrc:Main/main.qml"
    using namespace Qt::Literals::StringLiterals;
    // QString operator""_qs introduced since 6.2 deprecated since 6.8, use QString operator""_s instead.
    // See https://github.com/qbittorrent/qBittorrent/issues/19184.
    const QUrl url(u"qrc:/qt/qml/com/github/yihuajack/main.qml"_s);

    // Consider using KDSingleApplication
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, app.get(),
        [url](QObject *obj, const QUrl &objUrl) {
            if (not obj and url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return QGuiApplication::exec();
}
