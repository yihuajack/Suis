//
// Created by Yihua Liu on 2024/03/31.
//

// https://stackoverflow.com/questions/74364607/qt-application-hangs-on-process-start-function-happens-only-when-function-is
// https://forum.qt.io/topic/117436/run-a-sh-shell-while-clicking-button-in-qml-with-qt5-15-0/21
#include "Process.h"

Process::Process(QObject* parent) : QObject(parent), m_Process(nullptr) {}

Process::~Process() {
    deleteProcess();
}

void Process::newProcess() {
    if (m_Process) {
        return;
    }
    m_Process = new QProcess();
    connect(m_Process, &QProcess::readyReadStandardError, this, &Process::readyReadStandardError);
    connect(m_Process, &QProcess::readyReadStandardOutput, this, &Process::readyReadStandardOutput);
}

void Process::deleteProcess() {
    if (not m_Process) {
        return;
    }
    disconnect(m_Process, &QProcess::readyReadStandardError, this, &Process::readyReadStandardError);
    disconnect(m_Process, &QProcess::readyReadStandardOutput, this, &Process::readyReadStandardOutput);
    delete m_Process;
    m_Process = nullptr;
}

void Process::start(const QString& program, const QStringList& arguments, OpenMode mode) {
    if (not m_Process) {
        newProcess();
    }
    m_Process->start(program, arguments, static_cast<QIODevice::OpenMode>((static_cast<int>(mode))));
}

QByteArray Process::readAllStandardError() {
    return m_Process ? m_Process->readAllStandardError() : QByteArray();
}

QByteArray Process::readAllStandardOutput() {
    return m_Process ? m_Process->readAllStandardOutput() : QByteArray();
}

qint64 Process::write(const QString& data) {
    return m_Process ? m_Process->write(qUtf8Printable(data)) : 0;
}
