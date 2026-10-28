#include "core.h"
#include <fstream>
#include <algorithm>
#include <cstdlib>

static int levenshtein(const std::string& a, const std::string& b) {
    size_t n = a.size(), m = b.size();
    std::vector<std::vector<int> > dp(n + 1, std::vector<int>(m + 1));
    for (size_t i = 0; i <= n; i++) dp[i][0] = (int)i;
    for (size_t j = 0; j <= m; j++) dp[0][j] = (int)j;
    for (size_t i = 1; i <= n; i++) {
        for (size_t j = 1; j <= m; j++) {
            int cost = (a[i-1] == b[j-1]) ? 0 : 1;
            int v1 = dp[i-1][j] + 1, v2 = dp[i][j-1] + 1, v3 = dp[i-1][j-1] + cost;
            int mn = v1; if (v2 < mn) mn = v2; if (v3 < mn) mn = v3;
            dp[i][j] = mn;
        }
    }
    return dp[n][m];
}

std::vector<std::string> load_popular_list(const std::string& path) {
    std::vector<std::string> list;
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line[0] != '#') list.push_back(line);
    }
    return list;
}

Policy load_policy(const std::string& path) {
    Policy p;
    std::ifstream f(path);
    std::string line, section;
    while (std::getline(f, line)) {
        size_t hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        line.erase(0, line.find_first_not_of(" \t\r\n\"-"));
        line.erase(line.find_last_not_of(" \t\r\n\"") + 1);
        if (line.empty()) continue;

        if (line == "allowed_packages:") { section = "allow"; continue; }
        if (line == "denied_packages:") { section = "deny"; continue; }
        
        // Читаем настройку дистанции
        if (line.find("max_typosquat_distance") != std::string::npos) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                try { p.max_distance = std::stoi(line.substr(colon + 1)); } catch(...) {}
            }
            continue;
        }

        if (section == "allow") p.allowed.push_back(line);
        if (section == "deny") p.denied.push_back(line);
    }
    return p;
}

void check_dependencies(const std::vector<Dependency>& deps, 
                        const std::vector<std::string>& popular, 
                        const Policy& policy) {
    int violations = 0;
    std::cout << "Dependencies found: " << deps.size() << "\n";

    for (size_t i = 0; i < deps.size(); ++i) {
        const Dependency& d = deps[i];
        std::string full_name = d.ecosystem + ":" + d.name;
        std::cout << "  - " << d.name << " [" << d.ecosystem << "]\n";

        if (std::find(policy.denied.begin(), policy.denied.end(), full_name) != policy.denied.end()) {
            std::cout << "    [ERROR] Blocked by policy: " << full_name << "\n";
            violations++;
            continue;
        }

        if (!policy.allowed.empty() && std::find(policy.allowed.begin(), policy.allowed.end(), full_name) == policy.allowed.end()) {
            std::cout << "    [ERROR] Not in allowlist: " << full_name << "\n";
            violations++;
        }

        for (size_t j = 0; j < popular.size(); ++j) {
            int dist = levenshtein(d.name, popular[j]);
            if (dist > 0 && dist <= policy.max_distance) {
                std::cout << "    [ERROR] Typosquatting: '" << d.name << "' looks like '" << popular[j] << "'\n";
                violations++;
                break;
            }
        }
    }

    std::cout << "\n=================================\n";
    if (violations == 0) {
        std::cout << "RESULT: OK. Build allowed.\n";
        exit(0);
    } else {
        std::cout << "RESULT: FAIL. Violations: " << violations << "\nBuild BLOCKED.\n";
        exit(1);
    }
}
