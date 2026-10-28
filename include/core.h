#pragma once
#include <string>
#include <vector>
#include <iostream>

struct Dependency {
    std::string name;
    std::string ecosystem;
};

struct Policy {
    std::vector<std::string> allowed;
    std::vector<std::string> denied;
    int max_distance = 1;
};

// ??????? (????????????? ?????????? ???? ?? ????? ?????)
std::vector<Dependency> parse_manifest(const std::string& filepath);
std::vector<std::string> load_popular_list(const std::string& filepath);
Policy load_policy(const std::string& filepath);

// ???????? ????????????
void check_dependencies(const std::vector<Dependency>& deps, 
                        const std::vector<std::string>& popular, 
                        const Policy& policy);
