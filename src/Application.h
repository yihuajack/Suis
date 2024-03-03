//
// Created by Yihua on 2024/2/23.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>

class Application final : QGuiApplication {
    Q_OBJECT

    Q_DISABLE_COPY_MOVE(Application)
public:
    QGuiApplication *m_app{nullptr};
};

#endif // APPLICATION_H
