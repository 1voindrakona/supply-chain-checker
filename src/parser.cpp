#include "core.h"
#include <fstream>
#include <regex>
#include <sstream>
#include <algorithm>

static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::vector<Dependency> parse_manifest(const std::string& path) {
    std::vector<Dependency> deps;
    std::string content = read_file(path);
    if (content.empty()) return deps;

    std::string filename = path.substr(path.find_last_of("/\\") + 1);

    // 1. JavaScript (package.json)
    if (filename == "package.json") {
        std::regex block_re("\"(?:dev)?[dD]ependencies\"\\s*:\\s*\\{([^}]*)\\}");
        auto block_begin = std::sregex_iterator(content.begin(), content.end(), block_re);
        for (auto bit = block_begin; bit != std::sregex_iterator(); ++bit) {
            std::string dep_block = (*bit)[1].str();
            std::regex item_re("\"([^\"]+)\"\\s*:");
            auto begin = std::sregex_iterator(dep_block.begin(), dep_block.end(), item_re);
            for (auto it = begin; it != std::sregex_iterator(); ++it) {
                deps.push_back({(*it)[1].str(), "npm"});
            }
        }
    } 
    // 2. C++ (vcpkg.json)
    else if (filename == "vcpkg.json") {
        std::regex block_re("\"dependencies\"\\s*:\\s*\\[([^\\]]*)\\]");
        std::smatch match;
        if (std::regex_search(content, match, block_re)) {
            std::string dep_block = match[1].str();
            std::regex obj_re("\"name\"\\s*:\\s*\"([^\"]+)\"");
            auto obj_begin = std::sregex_iterator(dep_block.begin(), dep_block.end(), obj_re);
            for (auto it = obj_begin; it != std::sregex_iterator(); ++it) {
                deps.push_back({(*it)[1].str(), "cpp"});
            }
            if (deps.empty()) {
                std::regex str_re("\"([^\"]+)\"");
                auto str_begin = std::sregex_iterator(dep_block.begin(), dep_block.end(), str_re);
                for (auto it = str_begin; it != std::sregex_iterator(); ++it) {
                    std::string name = (*it)[1].str();
                    if (name != "name" && name != "version") deps.push_back({name, "cpp"});
                }
            }
        }
    }
    // 3. Python (requirements.txt)
    else if (filename == "requirements.txt") {
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            if (line.empty() || line[0] == '#') continue;
            size_t pos = line.find_first_of("=<>~!");
            std::string name = (pos != std::string::npos) ? line.substr(0, pos) : line;
            name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
            if (!name.empty()) deps.push_back({name, "pypi"});
        }
    }
    // 4. Rust (Cargo.toml)
    else if (filename == "Cargo.toml") {
        std::regex re("^([a-zA-Z0-9_-]+)\\s*=");
        std::istringstream stream(content);
        std::string line;
        bool in_deps = false;
        while (std::getline(stream, line)) {
            if (line.find("[dependencies]") != std::string::npos) in_deps = true;
            else if (line.find("[") != std::string::npos) in_deps = false;
            else if (in_deps) {
                std::smatch m;
                if (std::regex_search(line, m, re)) deps.push_back({m[1].str(), "cargo"});
            }
        }
    }
    return deps;
}
