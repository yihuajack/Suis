//
// Created by Yihua Liu on 2024/03/29.
//

#include <QFile>

#include "Fs.h"

/**
 * Removes the file with the given filePath.
 *
 * This function will try to fix the file permissions before removing it.
 */
bool Utils::Fs::removeFile(const std::filesystem::path &path) {
    if (QFile::remove(path)) {
        return true;
    }

    QFile file {path};
    if (not file.exists()) {
        return true;
    }

    // Make sure we have read/write permissions
    file.setPermissions(file.permissions() | QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser);
    return file.remove();
}
