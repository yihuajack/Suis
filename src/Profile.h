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

#pragma once

#include <filesystem>
// Just use QSettings instead of boost::program_options. Boost libraries lack maintenance and documentation.
#include <QSettings>

namespace Private {
    class Profile;
}

enum class SpecialFolder {
    Cache,
    Config,
    Data,
    Downloads
};

class Profile {
public:
    static void initInstance(const std::filesystem::path &rootProfilePath, const std::u16string &configurationName);
    static void freeInstance();
    static const Profile *instance();

    [[nodiscard]] std::filesystem::path location(SpecialFolder folder) const;
    [[nodiscard]] std::unique_ptr<QSettings> applicationSettings(const std::u16string &name) const;

    [[nodiscard]] std::filesystem::path rootPath() const;
    [[nodiscard]] std::u16string configurationName() const;

    /// Returns either default name for configuration file (QCoreApplication::applicationName())
    /// or the value, supplied via parameters
    [[nodiscard]] std::u16string profileName() const;

private:
    Profile(const std::filesystem::path &rootProfilePath, const std::u16string &configurationName);
    ~Profile() = default;  // to generate correct call to ProfilePrivate::~ProfileImpl()

    void ensureDirectoryExists(SpecialFolder folder) const;

    std::unique_ptr<Private::Profile> m_profileImpl;
    static Profile *m_instance;
};

std::filesystem::path specialFolderLocation(SpecialFolder folder);
