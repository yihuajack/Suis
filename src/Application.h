//
// Created by Yihua on 2024/2/23.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>

class Application final : public QObject {
    Q_OBJECT
    // Q_DISABLE_COPY_MOVE(Application)

public:
    static Application *instance();

    // Parameters of the constructor must match that of the base class
    // QGuiApplication is a derived class from QCoreApplication
    // QGuiApplication has an additional default parameter int = ApplicationFlags
    // If derive Application from QGuiApplication,
    // the constructor should be declared as Application(int &argc, char **argv)
private:
    explicit Application();
    ~Application() = default;

    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;
    Application(Application &&) = delete;
    Application &operator=(Application &&) = delete;
};

#endif // APPLICATION_H
