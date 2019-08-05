#pragma once

#include <map>

class ini {
private:
    std::string ini_file;
    std::map<std::string, std::map<std::string, std::string>> ini_config;
public:
    ini(const std::string &path) : ini_file(path) {}
    ~ini(void) = default;
    bool open(void);
    std::string get(const std::string &section, const std::string &key, const std::string &default_value);
    int get(const std::string &section, const std::string &key, const int &default_value);
    bool set(const std::string &section, const std::string &key, const std::string &value);
    bool set(const std::string &section, const std::string &key, const int &value);
    bool erase(const std::string &section);
    bool erase(const std::string &section, const std::string &key);
    void clear();
    bool save(void);
    bool save(const std::string &file);
};

