#include <QDir>
#include <QProcessEnvironment>
#include <QStandardPaths>

#include "backend_init.h"

void backend_init() {
    /*Let us expected P1031R2: Low level file i/o library (https://wg21.link/p1031r2)
     * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1031r2.pdf (https://ned14.github.io/llfio/)
#ifdef _MSC_VER
    char *user_path;
    std::size_t len_userdata;
    errno_t err_userdata = _dupenv_s(&user_path, &len_userdata, "SUIS_USER_DATA");
    if (err_userdata) {
        throw std::runtime_error("_dupenv_s fails to get user data environment variable, errno is " + std::to_string(err_userdata) + ".");
    }
#else
    char *user_path = std::getenv("SUIS_USER_DATA");
#endif
    if (!user_path) {
        throw std::runtime_error("getenv fails to get user data environment variable.");
    }
    free(user_path);
     */
    QProcessEnvironment sysenv = QProcessEnvironment::systemEnvironment();
    QString user_path_str = sysenv.value("SUIS_USER_DATA", "");
    std::filesystem::path user_path;
    if (user_path_str.isEmpty()) {
        QStandardPaths::StandardLocation home_loc = QStandardPaths::HomeLocation;  // the same as QDir::homePath()
        QString home_path = QStandardPaths::displayName(home_loc);
        user_path = home_path.toStdString();
        user_path_str = "~/.suis";
        user_path /= ".suis";
    } else {
        user_path = user_path_str.toStdString();
    }
    // std::filesystem::path::format has native_format, generic_format, and auto_format.
    // generic_format uses slashes, while native_format of Windows paths uses backslashes.
    // We can convert std::filesyttem::path to std::string by std::filesystem::path::generic_string() bu
    // QDir() constructor accepts both QString and std::filesystem::path
    QDir quser_pathdir(user_path);
    if (!quser_pathdir.exists()) {
        // QDir::mkdir() is enough. Alternatively use QDir::mkpath() to create all parent directories.
        quser_pathdir.mkdir(user_path_str);
    }
    std::filesystem::path user_config = user_path / "suis.ini";

}