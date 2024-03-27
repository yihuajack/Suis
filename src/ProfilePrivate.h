/*
 * Bittorrent Client using Qt and libtorrent.
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

#include <QStandardPaths>

#include "Profile.h"

namespace Private {
    class Profile {
    public:
        virtual ~Profile() = default;

        [[nodiscard]] virtual std::filesystem::path rootPath() const = 0;

        /**
         * @brief The base path against to which portable (relative) paths are resolved
         */
        [[nodiscard]] virtual std::filesystem::path basePath() const = 0;

        [[nodiscard]] virtual std::filesystem::path cacheLocation() const = 0;
        [[nodiscard]] virtual std::filesystem::path configLocation() const = 0;
        [[nodiscard]] virtual std::filesystem::path dataLocation() const = 0;
        [[nodiscard]] virtual std::filesystem::path downloadLocation() const = 0;

        [[nodiscard]] virtual std::unique_ptr<QSettings> applicationSettings(const std::u16string &name) const = 0;

        [[nodiscard]] std::u16string configurationName() const;

        /**
         * @brief QCoreApplication::applicationName() with optional configuration name appended
         */
        [[nodiscard]] std::u16string profileName() const;

    protected:
        explicit Profile(std::u16string configurationName);

        [[nodiscard]] std::u16string configurationSuffix() const;

    private:
        std::u16string m_configurationName;
    };

    /// Default implementation. Takes paths from system
    class DefaultProfile final : public Profile     {
    public:
        explicit DefaultProfile(const std::u16string &configurationName);

        [[nodiscard]] std::filesystem::path rootPath() const override;
        [[nodiscard]] std::filesystem::path basePath() const override;
        [[nodiscard]] std::filesystem::path cacheLocation() const override;
        [[nodiscard]] std::filesystem::path configLocation() const override;
        [[nodiscard]] std::filesystem::path dataLocation() const override;
        [[nodiscard]] std::filesystem::path downloadLocation() const override;
        [[nodiscard]] std::unique_ptr<QSettings> applicationSettings(const std::u16string &name) const override;

    private:
        /**
         * @brief Standard path writable location for profile files
         *
         * @param location location kind
         * @return QStandardPaths::writableLocation(location) / configurationName()
         */
        [[nodiscard]] std::filesystem::path locationWithConfigurationName(QStandardPaths::StandardLocation location) const;
    };

    /// Custom tree: creates directories under the specified root directory
    class CustomProfile final : public Profile     {
    public:
        CustomProfile(std::filesystem::path rootPath, const std::u16string &configurationName);

        [[nodiscard]] std::filesystem::path rootPath() const override;
        [[nodiscard]] std::filesystem::path basePath() const override;
        [[nodiscard]] std::filesystem::path cacheLocation() const override;
        [[nodiscard]] std::filesystem::path configLocation() const override;
        [[nodiscard]] std::filesystem::path dataLocation() const override;
        [[nodiscard]] std::filesystem::path downloadLocation() const override;
        [[nodiscard]] std::unique_ptr<QSettings> applicationSettings(const std::u16string &name) const override;

    private:
        const std::filesystem::path m_rootPath;
        const std::filesystem::path m_basePath;
        const std::filesystem::path m_cacheLocation;
        const std::filesystem::path m_configLocation;
        const std::filesystem::path m_dataLocation;
        const std::filesystem::path m_downloadLocation;
    };
}
