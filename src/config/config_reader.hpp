#pragma once

#include <map>
#include <string>

class ConfigReader {
public:
    bool load(const std::string& filename);
    double get(const std::string& key, double default_value = 0.0) const;
    void print() const;

private:
    std::map<std::string, double> params_{};

    static std::string trim(const std::string& str);
};
