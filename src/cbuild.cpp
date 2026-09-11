#include <json.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

void create_project(const std::string& name) {
    if (fs::exists(name)) {
        std::cerr << "Error: Directory \"" << name << "\" already exists!\n";
        return;
    }

    fs::create_directories(name + "/src");
    fs::create_directories(name + "/include");

    std::ofstream main_file(name + "/src/main.cpp");
    main_file << "#include <iostream>\n\n"
              << "int main() {\n"
              << "    std::cout << \"Hello from " << name << "!\" << std::endl;\n"
              << "    return 0;\n"
              << "}\n";
    main_file.close();

    json config;
    config["project"] = name;
    config["compiler"] = "g++";
    config["flags"] = json::array({"-std=c++20", "-Wall", "-O2"});
    config["include_dirs"] = json::array({"include", "src"});
    config["src"] = json::array({"src/main.cpp"});
    #ifdef _WIN32
    	config["output"] = name + ".exe";
	#else
    	config["output"] = name;
	#endif

    std::ofstream config_file(name + "/cbuild_config.json");
    config_file << config.dump(4);
    config_file.close();

    std::cout << "Project \"" << name << "\" created successfully!\n";
    std::cout << "  cd " << name << "\n";
    std::cout << "  cbuild\n";
}

std::string get_object_name(const std::string& src) {
    fs::path p(src);
    return "cbuild_cache/" + p.stem().string() + ".o";
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && std::string(argv[1]) == "create") {
        create_project(argv[2]);
        return 0;
    }

    std::ifstream config_file("cbuild_config.json");
    if (!config_file.is_open()) {
        std::cerr << "Error: cbuild_config.json not found\n";
        std::cerr << "Usage: cbuild create <project_name>\n";
        return 1;
    }

    json data = json::parse(config_file);

    std::string compiler = data["compiler"];
    std::string output = data["output"];

    std::vector<std::string> sources;
    for (const auto& s : data["src"]) {
        sources.push_back(s.get<std::string>());
    }

    std::vector<std::string> includes;
    if (data.contains("include_dirs")) {
        for (const auto& d : data["include_dirs"]) {
            includes.push_back(d.get<std::string>());
        }
    }

    std::vector<std::string> flags;
    if (data.contains("flags")) {
        for (const auto& f : data["flags"]) {
            flags.push_back(f.get<std::string>());
        }
    }

    fs::create_directories("cbuild_cache");

    std::vector<std::string> objects;

    for (const auto& src : sources) {
        std::string obj = get_object_name(src);
        objects.push_back(obj);

        std::string cmd = compiler + " -c";

        for (const auto& f : flags) {
            cmd += " " + f;
        }

        for (const auto& dir : includes) {
            cmd += " -I" + dir;
        }

        cmd += " " + src + " -o " + obj;

        std::cout << "Compiling: " << src << "\n";
        int result = system(cmd.c_str());
        if (result != 0) {
            std::cerr << "Failed to compile " << src << "\n";
            return 1;
        }
    }

    std::string link_cmd = compiler;

    for (const auto& f : flags) {
        link_cmd += " " + f;
    }

    for (const auto& obj : objects) {
        link_cmd += " " + obj;
    }

    link_cmd += " -o " + output;

    std::cout << "Linking: " << output << "\n";
    int result = system(link_cmd.c_str());

    if (result != 0) {
        std::cerr << "Link failed\n";
        return 1;
    }

    std::cout << "Build successful: " << output << "\n";
    return 0;
}