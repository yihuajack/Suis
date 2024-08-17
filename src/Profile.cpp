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

#include "Profile.h"
#include "ProfilePrivate.h"

Profile *Profile::m_instance = nullptr;

Profile::Profile(const std::filesystem::path &rootProfilePath,
                 const std::u16string &configurationName) {
    if (rootProfilePath.empty()) {
        m_profileImpl = std::make_unique<Private::DefaultProfile>(configurationName);
    } else {
        m_profileImpl = std::make_unique<Private::CustomProfile>(rootProfilePath, configurationName);
    }

    ensureDirectoryExists(SpecialFolder::Cache);
    ensureDirectoryExists(SpecialFolder::Config);
    ensureDirectoryExists(SpecialFolder::Data);
}

void Profile::initInstance(const std::filesystem::path &rootProfilePath, const std::u16string &configurationName) {
    if (m_instance) {
        return;
    }
    m_instance = new Profile(rootProfilePath, configurationName);
}

void Profile::freeInstance() {
    delete m_instance;
    m_instance = nullptr;
}

const Profile *Profile::instance() {
    return m_instance;
}

std::filesystem::path Profile::location(const SpecialFolder folder) const {
    switch (folder) {
        case SpecialFolder::Cache:
            return m_profileImpl->cacheLocation();

        case SpecialFolder::Config:
            return m_profileImpl->configLocation();

        case SpecialFolder::Data:
            return m_profileImpl->dataLocation();

        case SpecialFolder::Downloads:
            return m_profileImpl->downloadLocation();

        default:
            Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown SpecialFolder value.");
            return {};  /* Unreachable */
    }
}

std::filesystem::path Profile::rootPath() const {
    return m_profileImpl->rootPath();
}

std::u16string Profile::configurationName() const {
    return m_profileImpl->configurationName();
}

std::u16string Profile::profileName() const {
    return m_profileImpl->profileName();
}

std::unique_ptr<QSettings> Profile::applicationSettings(const std::u16string &name) const {
    return m_profileImpl->applicationSettings(name);
}

void Profile::ensureDirectoryExists(const SpecialFolder folder) const {
    const std::filesystem::path locationPath = location(folder);
    std::error_code ec;
    if (not std::filesystem::exists(locationPath)) {
        std::filesystem::create_directories(locationPath, ec);
    }
    // Note that create_directories() will return false if the directory already exists, even though it does nothing!
    // If that is the case, error_code will be 0.
    if (not locationPath.empty() and ec) {
        qFatal("Could not create required directory '%s'. Error code is '%d'",
               qUtf8Printable(QString::fromStdString(locationPath.string())),  /* or c_str() */
               ec.value());
    }
}

std::filesystem::path specialFolderLocation(const SpecialFolder folder) {
    return Profile::instance()->location(folder);
}
