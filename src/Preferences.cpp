//
// Created by Yihua Liu on 2024/03/29.
//

#include <QString>

#include "Preferences.h"
#include "SettingsStorage.h"

using namespace Qt::Literals::StringLiterals;

namespace {
    template <typename T>
    T value(const QString &key, const T &defaultValue = {}) {
        return SettingsStorage::instance()->loadValue(key, defaultValue);
    }

    template <typename T>
    void setValue(const QString &key, const T &value) {
        SettingsStorage::instance()->storeValue(key, value);
    }
}

Preferences *Preferences::m_instance = nullptr;

Preferences::Preferences() = default;

Preferences *Preferences::instance() {
    return m_instance;
}

void Preferences::initInstance() {
    if (not m_instance) {
        m_instance = new Preferences;
    }
}

void Preferences::freeInstance() {
    delete m_instance;
    m_instance = nullptr;
}

// General Options
std::filesystem::path Preferences::getsUnitSystemPath() const {
    return value<std::filesystem::path>(u"Preferences/Downloads/UnitsSystemPath"_s);
}
