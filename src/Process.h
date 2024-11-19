//
// Created by Yihua Liu on 2024/03/31.
//

#ifndef PROCESS_H
#define PROCESS_H

#include <QProcess>
#include <QIODevice>
#include <QObject>
#include <QQmlEngine>

class Process : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    enum OpenModeFlag {
        NotOpen = QIODevice::NotOpen,
        ReadOnly = QIODevice::ReadOnly,
        WriteOnly = QIODevice::WriteOnly,
        ReadWrite = QIODevice::ReadWrite,
        Append = QIODevice::Append,
        Truncate = QIODevice::Truncate,
        Text = QIODevice::Text,
        Unbuffered = QIODevice::Unbuffered,
        NewOnly = QIODevice::NewOnly,
        ExistingOnly = QIODevice::ExistingOnly
    };
    Q_DECLARE_FLAGS(OpenMode, OpenModeFlag)

signals:
    void readyReadStandardError();
    void readyReadStandardOutput();

public:
    explicit Process(QObject* parent = nullptr);
    ~Process() override;
    // Clazy: Invokable arguments need to be fully-qualified (Process::OpenMode instead of OpenMode)
    Q_INVOKABLE void start(const QString& program, const QStringList& arguments = QStringList(),
                           Process::OpenMode mode = ReadWrite);
    Q_INVOKABLE QByteArray readAllStandardError();
    Q_INVOKABLE QString readAllStandardOutput();
    Q_INVOKABLE qint64 write(const QString& data);

protected:
    QProcess *m_Process;
    void newProcess();
    void deleteProcess();
};

#endif  // PROCESS_H
