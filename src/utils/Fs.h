//
// Created by Yihua on 2024/03/29.
//

#ifndef UTILS_FS_H
#define UTILS_FS_H

#include <filesystem>

namespace Utils::Fs {
    bool removeFile(const std::filesystem::path &path);
}

#endif  // UTILS_FS_H
