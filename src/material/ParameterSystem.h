//
// Created by Yihua Liu on 2024/6/11.
//

#ifndef SUISAPP_PARAMETERSYSTEM_H
#define SUISAPP_PARAMETERSYSTEM_H

#include <mutex>
#include <QSettings>

class ParameterSystem {
public:
    ParameterSystem(ParameterSystem&) = delete;
    void operator=(const ParameterSystem&) = delete;
    static ParameterSystem *GetInstance() {
        std::lock_guard lock(mutex_);
        return pinstance_;
    }
    static void SetInstance(const QMap<QString, QString>& par_map, const QString& root_path) {
        std::lock_guard lock(mutex_);
        if (pinstance_ == nullptr) {
            pinstance_ = new ParameterSystem(par_map, root_path);
        }
    }
    [[nodiscard]] bool isComposition(const QString& mat_name, const QString& key) const;

protected:
    ParameterSystem(const QMap<QString, QString>& par_map, const QString& root_path);
    ~ParameterSystem() = default;
    QSettings par_settings;

private:
    static ParameterSystem *pinstance_;
    static std::mutex mutex_;
};

#endif  // SUISAPP_PARAMETERSYSTEM_H
