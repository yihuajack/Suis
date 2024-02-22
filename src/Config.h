#include <filesystem>
#include <boost/property_tree/ptree.hpp>

class OpenSCSimConfig {
private:
    std::filesystem::path default_config;
    std::filesystem::path user_config;
    std::filesystem::path user_folder;
    boost::property_tree::ptree default_data;
    boost::property_tree::ptree user_data;
    void reset_defaults(bool confirm = true);
    void _save_user_config() const;
    void restore_defaults();
    auto version() -> std::string const;
public:
    // Singleton (design) pattern is not preferred here.
    OpenSCSimConfig();
    OpenSCSimConfig(std::filesystem::path default_config, std::filesystem::path user_config);

    class OptionProxy {
    private:
        const boost::property_tree::ptree &option_tree;
    public:
        explicit OptionProxy(const boost::property_tree::ptree &option_tree);
        auto operator[](const std::string &option_key) const -> std::string;
    };

    auto operator[](const std::string &section_key) const -> OptionProxy;

    auto verbose_loading(std::optional<bool> show = std::nullopt) -> bool const;
};
