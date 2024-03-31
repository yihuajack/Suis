//
// Created by Yihua Liu on 2024/2/23.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>
#include <QSettings>

#include "CommandLineParseResult.h"

class Application final : public QGuiApplication {
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
