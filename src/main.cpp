#include "core.h"

int main(int argc, char** argv) {
    std::string manifest = "vcpkg.json";
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--manifest" && i + 1 < argc) manifest = argv[++i];
    }

    std::cout << "=== Universal Supply Chain Checker ===\n";
    std::cout << "Analyzing: " << manifest << "\n\n";

    std::vector<Dependency> deps = parse_manifest(manifest);
    if (deps.empty()) {
        std::cout << "[!] No dependencies found or unsupported format.\n";
        return 1;
    }

    std::vector<std::string> popular = load_popular_list("config/popular_packages.txt");
    Policy policy = load_policy("policy.yml");

    check_dependencies(deps, popular, policy);
    return 0;
}
