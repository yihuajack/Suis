//
// Created by Yihua Liu on 2024/03/29.
//

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <filesystem>
#include <QObject>

class Preferences final : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Preferences)

    Preferences();
public:
    static void initInstance();
    static void freeInstance();
    static Preferences *instance();

    // General options
    [[nodiscard]] std::filesystem::path getsUnitSystemPath() const;

private:
    static Preferences *m_instance;
};

#endif  // PREFERENCES_H
