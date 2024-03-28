/*
 * Copyright (C) 2024  Yihua Liu <yihuajack@live.cn>
 * Copyright (C) 2016  Eugene Shalygin <eugene.shalygin@gmail.com>
 * Copyright (C) 2012  Christophe Dumez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#include "ProfilePrivate.h"

#include <QCoreApplication>

using namespace std::string_literals;

Private::Profile::Profile(std::u16string configurationName) : m_configurationName{std::move(configurationName)} {
}

std::u16string Private::Profile::configurationName() const {
    return m_configurationName;
}

std::u16string Private::Profile::configurationSuffix() const {
    return (m_configurationName.empty() ? std::u16string() : (u'_' + m_configurationName));
}

std::u16string Private::Profile::profileName() const {
    return QCoreApplication::applicationName().toStdU16String() + configurationSuffix();
}

Private::DefaultProfile::DefaultProfile(const std::u16string &configurationName) : Profile {configurationName} {
}

std::filesystem::path Private::DefaultProfile::rootPath() const {
    return {};
}

std::filesystem::path Private::DefaultProfile::basePath() const {
    return std::filesystem::path{QStandardPaths::displayName(QStandardPaths::HomeLocation).toStdString()};
}

std::filesystem::path Private::DefaultProfile::cacheLocation() const {
    return locationWithConfigurationName(QStandardPaths::CacheLocation);
}

std::filesystem::path Private::DefaultProfile::configLocation() const {
    return locationWithConfigurationName(QStandardPaths::AppConfigLocation);
}

std::filesystem::path Private::DefaultProfile::dataLocation() const {
    return locationWithConfigurationName(QStandardPaths::AppLocalDataLocation);
}

std::filesystem::path Private::DefaultProfile::downloadLocation() const {
    return std::filesystem::path{QStandardPaths::writableLocation(QStandardPaths::DownloadLocation).toStdString()};
}

std::unique_ptr<QSettings> Private::DefaultProfile::applicationSettings(const std::u16string &name) const {
    // QSettings::NativeFormat, QSettings::UserScope
    return std::make_unique<QSettings>( QString::fromStdU16String(profileName()), QString::fromStdU16String(name));
}

std::filesystem::path Private::DefaultProfile::locationWithConfigurationName(const QStandardPaths::StandardLocation location) const {
    return std::filesystem::path{QStandardPaths::writableLocation(location).toStdU16String() + configurationSuffix()};
}

Private::CustomProfile::CustomProfile(std::filesystem::path rootPath, const std::u16string &configurationName)
    : Profile {configurationName}
    , m_rootPath {std::move(rootPath)}
    , m_basePath {m_rootPath / std::filesystem::path(profileName())}
    , m_cacheLocation {m_basePath / std::filesystem::path(u"cache"s)}
    , m_configLocation {m_basePath / std::filesystem::path(u"config"s)}
    , m_dataLocation {m_basePath / std::filesystem::path(u"data"s)}
    , m_downloadLocation {m_basePath / std::filesystem::path(u"downloads"s)} {
}

std::filesystem::path Private::CustomProfile::rootPath() const {
    return m_rootPath;
}

std::filesystem::path Private::CustomProfile::basePath() const {
    return m_basePath;
}

std::filesystem::path Private::CustomProfile::cacheLocation() const {
    return m_cacheLocation;
}

std::filesystem::path Private::CustomProfile::configLocation() const {
    return m_configLocation;
}

std::filesystem::path Private::CustomProfile::dataLocation() const {
    return m_dataLocation;
}

std::filesystem::path Private::CustomProfile::downloadLocation() const {
    return m_downloadLocation;
}

std::unique_ptr<QSettings> Private::CustomProfile::applicationSettings(const std::u16string &name) const {
    // here we force QSettings::IniFormat format always because we need it to be portable across platforms
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    const std::u16string CONF_FILE_EXTENSION = u".ini"s;
#else
    const auto CONF_FILE_EXTENSION = u".conf"_s;
#endif
    const std::filesystem::path settingsFilePath = configLocation() / std::filesystem::path(name + CONF_FILE_EXTENSION);
    return std::make_unique<QSettings>(QString::fromStdString(settingsFilePath.string()), QSettings::IniFormat);
}
