#include <iostream>
#include <QFile>
// #include "boost/exception/get_error_info.hpp"
// #include <boost/exception/error_info.hpp>
#include <boost/property_tree/ini_parser.hpp>  // Already includes <boost/property_tree/ptree.hpp>

#include "Config.h"

SuisConfig::SuisConfig() {}

SuisConfig::SuisConfig(std::filesystem::path default_config,
                                 std::filesystem::path user_config) : default_config(std::move(default_config)),
                                                                      user_config(std::move(user_config)),
                                                                      user_folder(user_config.parent_path()) {
    boost::property_tree::ini_parser::read_ini(default_config.generic_string(), default_data);
    QFile user_config_file;
    if (!user_config_file.exists()) {
        reset_defaults(true);
    } else {
        boost::property_tree::ini_parser::read_ini(user_config.generic_string(), user_data);
    }
    const std::string version_data = version();
    if (version_data.empty() or version_data not_eq default_data.get<std::string>("Configuration.version")) {
        reset_defaults();
    }
}

/*
 * Resets the default Suis configuration in the user home folder.
 */
void SuisConfig::reset_defaults(bool confirm) {
    if (!confirm) {
        std::cout << "This action will delete any custom Suis configuration. "
                     "Are you sure you want to continue (Y/n)?";
        char response;
        std::cin >> response;
        if (response == 'Y' or response == 'y') {
            confirm = true;
        }
    }
    if (confirm) {
        QFile default_config_file;
        default_config_file.copy(user_config);
        boost::property_tree::ini_parser::read_ini(user_config.generic_string(), user_data);
    }
}

/*
 * Saves the current user configuration
 */
void SuisConfig::_save_user_config() const {
    boost::property_tree::write_ini(user_config.generic_string(), user_data);
}

/*
 * Restores all the default values without touching user additions.
 */
void SuisConfig::restore_defaults() {
    for (const boost::property_tree::ptree::value_type &s : default_data) {
        for (const boost::property_tree::ptree::value_type &o : default_data.get_child(s.second.data())) {
            user_data.put(o.first.data(), o.second.data());
        }
    }
}

/*
 * Provides the Solcore version
 *
 * :return: The version number
 */
auto SuisConfig::version() -> std::string const {
    /* try {
        // std::string of the parameter is path_type, std::string of the return value is template class Type.
        return user_data.get<std::string>("Configuration.version");
    } catch (const boost::property_tree::ptree_bad_path &ptree_bad_path) {
        std::cerr << ptree_bad_path.what() << '\n';
        return "";
    } */
    boost::optional<std::string> version_data = user_data.get_optional<std::string>("Configuration.version");
    return version_data.value_or("");
}

SuisConfig::OptionProxy::OptionProxy(const boost::property_tree::ptree &option_tree) : option_tree(option_tree) {}

auto SuisConfig::OptionProxy::operator[](const std::string &option_key) const -> std::string {
    return option_tree.get<std::string>(option_key);
}

auto SuisConfig::operator[](const std::string &section_key) const -> SuisConfig::OptionProxy {
    const boost::property_tree::ptree &option_tree = user_data.get_child(section_key);
    return SuisConfig::OptionProxy(option_tree);
}

/*
 * Sets if the loading messages (besides the welcome message) must be shown or not
 *
 * :param show: True/False for showing/hiding the loading messages
 * :return: None
 */
auto SuisConfig::verbose_loading(std::optional<bool> show) -> bool const {
    if (show.has_value()) {
        // user_data.put("Configuration.verbose_loading", std::format("{}", show.value()));
        user_data.put("Configuration.verbose_loading", show.value());
    }
    return user_data.get<std::string>("Configuration.verbose_loading") == "1";
}
