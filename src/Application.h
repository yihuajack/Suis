//
// Created by Yihua Liu on 2024/2/23.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QSettings>

#include "CommandLineParseResult.h"

// Application has to be a QApplication rather than QGuiApplication because we need to use QtCharts in QML ChartView
// Otherwise, ASSERT: "!" No style available without QApplication!"" in file
// C:\Users\qt\work\qt\qtbase\src\widgets\qtkernel\qapplication.cpp, line 907
class Application final : public QApplication {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Application)

public:
    // Parameters of the constructor must match that of the base class
    // QGuiApplication is a derived class from QCoreApplication
    // QGuiApplication has an additional default parameter int = ApplicationFlags
    explicit Application(int &argc, char **argv);
    ~Application() override = default;
private:
    CommandLineParseResult m_commandLineArgs;
};

#endif // APPLICATION_H
