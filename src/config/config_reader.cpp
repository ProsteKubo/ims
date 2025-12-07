#include "config_reader.hpp"

#include <fstream>
#include <iostream>
#include <string>

using std::cerr;
using std::cout;
using std::ifstream;
using std::string;

string ConfigReader::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

bool ConfigReader::load(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open config file: " << filename << "\n";
        return false;
    }

    string line;
    while (getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#' || line[0] == '[') continue;

        size_t eq_pos = line.find('=');
        if (eq_pos != string::npos) {
            string key = trim(line.substr(0, eq_pos));
            string value_str = trim(line.substr(eq_pos + 1));

            size_t comment_pos = value_str.find('#');
            if (comment_pos != string::npos) {
                value_str = trim(value_str.substr(0, comment_pos));
            }

            try {
                double value = std::stod(value_str);
                params_[key] = value;
            } catch (...) {
                cerr << "Warning: Invalid value for " << key << ": " << value_str << "\n";
            }
        }
    }

    file.close();
    return true;
}

double ConfigReader::get(const string& key, double default_value) const {
    auto it = params_.find(key);
    if (it != params_.end()) return it->second;
    return default_value;
}

void ConfigReader::print() const {
    cout << "Loaded Configuration Parameters:" << '\n';
    for (const auto& p : params_) {
        cout << "  " << p.first << " = " << p.second << '\n';
    }
}
