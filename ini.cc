#include "ini.h"

using namespace std;

enum {
    INI_CONFIG_SECTION,
    INI_CONFIG_KEY_VALUE,
    INI_CONFIG_EMPTY,
};

union ini_parse_block {
    char *section;
    struct {
        char *key;
        char *value;
    } kv;
};

static int ini_parse_line(char *const line, union ini_parse_block &ipb)
{
    int line_type;
    size_t i, j;

    for (i = 0; line[i] != '\0'; ++i)
        if (!isspace(line[i]))
            break;
    switch (line[i]) {
    case '\0':
    case ';':
        line_type = INI_CONFIG_EMPTY;
        break;
    case '[':
        line_type = INI_CONFIG_SECTION;
        for (j = i + 1; line[j] != '\0' && line[j] != '\r' && line[j] != '\n'; ++j) {
            if (line[j] == ']')
                break;
        }
        if (line[j] == ']') {
            line[j] = '\0';
            ipb.section = line + i + 1;
        } else {
            line_type = INI_CONFIG_EMPTY;
        }
        break;
    default:
        line_type = INI_CONFIG_KEY_VALUE;
        for (j = i + 1; line[j] != '\0' && line[j] != '\r' && line[j] != '\n'; ++j) {
            if (line[j] == '=')
                break;
        }
        if (line[j] == '=') {
            ipb.kv.key = line + i;
            ipb.kv.value = line + j + 1;
            while (j-- > i) {
                if (!isspace(line[j]))
                    break;
            }
            line[++j] = '\0';
            for (i = 0; ipb.kv.value[i] != '\0' && ipb.kv.value[i] != '\r' && ipb.kv.value[i] != '\n'; ++i) {
                if (!isspace(ipb.kv.value[i])) {
                    ipb.kv.value = ipb.kv.value + i;
                    break;
                }
            }
            for (i = 0; ipb.kv.value[i] != '\0' && ipb.kv.value[i] != '\r' && ipb.kv.value[i] != '\n'; ++i) {
                continue;
            }
            while (i-- > 0)
                if (!isspace(ipb.kv.value[i]))
                    break;
            ipb.kv.value[++i] = '\0';
        } else {
            line_type = INI_CONFIG_EMPTY;
        }
        break;
    }

    return line_type;
}

static map<string, map<string, string>>::iterator ini_create_section(
        map<string, map<string, string>> &ini_config, const string &section)
{
    map<string, map<string, string>>::iterator it;

    it = ini_config.find(section);
    if (it != ini_config.end())
        return it;

    ini_config.insert(pair<string, map<string, string>>(section,
            map<string, string>()));
    return ini_config.find(section);
}

static map<string, string>::iterator ini_create_key(
        map<string, string> &section, const string &key, const string &value)
{
    map<string, string>::iterator vit;

    vit = section.find(key);
    if (vit != section.end()) {
        vit->second = value;
        return vit;
    }

    section.insert(pair<string, string>(key, value));
    return section.find(key);
}

bool ini::open(void)
{
    FILE *fp;
    char buff[2048];
    string current_section = "default";
    map<string, map<string, string>>::iterator it;

    fp = fopen(ini_file.c_str(), "r");
    if (fp == NULL)
        return false;

    it = ini_create_section(ini_config, current_section);
    if (it == ini_config.end()) {
        fclose(fp);
        return false;
    }
    while (fgets(buff, sizeof(buff) - 1, fp) != NULL) {
        ini_parse_block ipb;
        switch (ini_parse_line(buff, ipb)) {
        case INI_CONFIG_SECTION:
            current_section = ipb.section;
            it = ini_create_section(ini_config, current_section);
            if (it == ini_config.end()) {
                fclose(fp);
                return false;
            }
            break;
        case INI_CONFIG_KEY_VALUE:
            it->second.insert(pair<string, string>(
                string(ipb.kv.key), string(ipb.kv.value)));
            break;
        default:
            break;
        }
    }
    fclose(fp);

    return true;
}

string ini::get(const string &section, const string &key,
        const string &default_value)
{
    map<string, map<string, string>>::iterator it;
    map<string, string>::iterator vit;
    
    it = ini_config.find(section);
    if (it == ini_config.end())
        return default_value;

    vit = it->second.find(key);
    if (vit == it->second.end())
        return default_value;
    return vit->second;
}

int ini::get(const string &section, const string &key,
        const int &default_value)
{
    string str = get(section, key, "default");
    
    if (str.compare("default") == 0)
        return default_value;

    return atoi(str.c_str());
}

bool ini::set(const string &section, const string &key,
        const string &value)
{
    map<string, map<string, string>>::iterator it;
    map<string, string>::iterator vit;

    it = ini_create_section(ini_config, section);
    if (it == ini_config.end())
        return false;

    vit = ini_create_key(it->second, key, value);
    return vit != it->second.end();
}

bool ini::set(const string &section, const string &key,
        const int &value)
{
    return set(section, key, to_string(value));
}

bool ini::erase(const string &section)
{
    map<string, map<string, string>>::iterator it;

    it = ini_config.find(section);
    if (it == ini_config.end())
        return false;

    it->second.clear();
    ini_config.erase(it);
    return true;
}

bool ini::erase(const string &section, const string &key)
{
    map<string, map<string, string>>::iterator it;
    map<string, string>::iterator vit;

    it = ini_config.find(section);
    if (it == ini_config.end())
        return false;

    vit = it->second.find(key);
    if (vit == it->second.end())
        return false;

    it->second.erase(vit);
    return true;
}

void ini::clear(void)
{
    map<string, map<string, string>>::iterator it;

    for (it = ini_config.begin(); it != ini_config.end(); ++it)
        it->second.clear();
    ini_config.clear();
}

bool ini::save(const std::string &file)
{
    FILE *fp;
    map<string, map<string, string>>::iterator it;
    map<string, string>::iterator vit;

    fp = fopen(file.c_str(), "w");
    if (fp == NULL)
        return false;

    for (it = ini_config.begin(); it != ini_config.end(); ++it) {
        fprintf(fp, "[%s]\n", it->first.c_str());
        for (vit = it->second.begin(); vit != it->second.end(); ++vit) {
            fprintf(fp, "%s=%s\n", vit->first.c_str(), vit->second.c_str());
        }
    }

    fflush(fp);
    fclose(fp);
    return true;
}

bool ini::save(void)
{
    return save(ini_file);
}

