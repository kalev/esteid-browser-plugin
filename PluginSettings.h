#ifndef H_ESTEID_PLUGINSETTINGS
#define H_ESTEID_PLUGINSETTINGS

#include <string>
#include <vector>

class PluginSettings {
public:
    std::vector<std::string> whitelist;
    std::vector<std::string> default_whitelist;
    bool allowLocal;
    bool allowDefaults;
    std::string fileName;
    PluginSettings();
    ~PluginSettings();

    bool InWhitelist(std::string s);
    void FindConfig();

    /** Loads config. Will fail silently if config is not found */
    void Load();
    /** Saves config. Will throw ios_base::failure on failure */
    void Save();
};
#endif
