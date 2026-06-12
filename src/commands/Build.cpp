#include <commands/Build.hpp>

#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <stdlib.h>
#include <Project.hpp>
#include <optional>
#include <fstream>
#include <set>

std::string Build::getName() const { return "build"; }

std::string_view Build::getHelp() const {
    return BUILD_HELP;
}

bool Build::compilerExists(const std::string& compiler_name) {
    std::string check_cmd = "which " + compiler_name + " > /dev/null 2>&1";
    return (std::system(check_cmd.c_str()) == 0);
}

bool Build::verifyHandlerSymbol(const std::string& so_path, const std::string& target_arch) {
    std::string nm_tool = "nm";
    if (target_arch == "arm64" || target_arch == "aarch64") {
        nm_tool = "aarch64-linux-gnu-nm";
    } else if (target_arch == "x86_64") {
        nm_tool = "x86_64-linux-gnu-nm";
    }
    std::string check_cmd = nm_tool + " -D " + so_path + " | grep -q ' T handler'";
    int res = std::system(check_cmd.c_str());
    return (res == 0);
}

std::string Build::getCompiler(const std::string& arch, const std::string& ext)
{
    std::string canonical_arch = arch;
    if (arch == "arm64") canonical_arch = "aarch64";

    static const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> compiler_registry = {
        {"aarch64", {
            {".c",       "aarch64-linux-gnu-gcc"},
            {".cpp",     "aarch64-linux-gnu-g++"},
            {".cc",      "aarch64-linux-gnu-g++"},
            {".rs",      "rustc --target aarch64-unknown-linux-gnu"},
            {".zig",     "zig build-lib -target aarch64-linux"},
            {".go",      "GOARCH=arm64 go build -buildmode=c-shared"},
            {".py",      "aarch64-linux-gnu-gcc"}
        }},
        {"x86_64", {
            {".c",       "x86_64-linux-gnu-gcc"},
            {".cpp",     "x86_64-linux-gnu-g++"},
            {".cc",      "x86_64-linux-gnu-g++"},
            {".rs",      "rustc --target x86_64-unknown-linux-gnu"},
            {".zig",     "zig build-lib -target x86_64-linux"},
            {".go",      "GOARCH=amd64 go build -buildmode=c-shared"},
            {".py",      "x86_64-linux-gnu-gcc"}
        }}
    };

    auto arch_it = compiler_registry.find(canonical_arch);
    if (arch_it != compiler_registry.end()) {
        auto ext_it = arch_it->second.find(ext);
        if (ext_it != arch_it->second.end()) {
            return ext_it->second;
        }
    }

    return "";
}

std::vector<std::filesystem::path>  Build::collectFiles(const std::filesystem::path& path, const std::string& arch)
{
    std::vector<std::filesystem::path> files_to_compile;
    std::set<std::filesystem::path> processed_makefiles;
    std::error_code ec;

    if (std::filesystem::is_directory(path, ec)) {
        for (auto it = std::filesystem::recursive_directory_iterator(path); it != std::filesystem::end(it); ++it) {
            std::filesystem::path file_dir = it->path().parent_path();

            bool has_makefile = std::filesystem::exists(file_dir / "Makefile") || 
                                std::filesystem::exists(file_dir / "makefile");

            if (has_makefile) {
                if (processed_makefiles.insert(file_dir).second) {
                    std::cout << "[BUILD] Custom Makefile discovered in " << file_dir.string() << ". Executing...\n";
                    
                    std::string make_cmd = "make -C " + file_dir.string();

                    static const std::vector<std::pair<std::string, std::string>> toolchain_matrix = {
                        {".c",    "CC"},
                        {".cpp",  "CXX"},
                        {".cc",   "CXX"},
                        {".rs",   "RUSTC"},
                        {".zig",  "ZIG"},
                        {".go",   "GO"}
                    };

                    for (const auto& [ext, make_var] : toolchain_matrix) {
                        std::string compiler_cmd = getCompiler(arch, ext);
                        
                        if (!compiler_cmd.empty()) {
                            make_cmd += " " + make_var + "=\"" + compiler_cmd + "\"";
                        }
                    }

                    if (std::system(make_cmd.c_str()) != 0) {
                        std::cerr << "[ERROR] Makefile execution terminated with faults in: " << file_dir.string() << "\n";
                    }
                }
                continue; 
            }

            if (it->is_regular_file()) {
                files_to_compile.push_back(it->path());
            }
        }
    } else if (std::filesystem::is_regular_file(path, ec)) {
        files_to_compile.push_back(path);
    } 
    return files_to_compile;
}

void Build::compile(const std::vector<std::filesystem::path>&& files, const std::string& arch)
{
    std::error_code ec;

    for (const std::filesystem::path& file_path : files) {
        std::string ext = file_path.extension().string();
        std::string stem = file_path.stem().string();
        std::string compiler = getCompiler(arch, ext);
        std::filesystem::path arch_dir = file_path.parent_path() / arch;

        if (compiler.empty()) continue;

        std::filesystem::create_directories(arch_dir, ec);
        if (ec) {
            std::cerr << "[ERROR] Could not create architecture directory: " << arch_dir << " - " << ec.message() << "\n";
            continue;
        }

        std::string out_so = (arch_dir / (stem + ".so")).string();
        auto it = extensionsMap_.find(ext);

        switch (it != extensionsMap_.end() ? it->second : Extensions::INVALID) {
            
            case Extensions::C: {
                if (!compilerExists(compiler)) {
                    std::cerr << "[ERROR] Target cross-compiler '" << compiler << "' is missing from system utilities paths.\n"
                              << "Please install the missing toolchain package to proceed.\n";
                    return;
                }
                std::string cmd = compiler + " -Wall -Wextra -Werror -Wno-unused-parameter -shared -fPIC -O3 -o " + out_so + " " + file_path.string();
                std::cout << "[BUILD] Compiling C [" << arch << "] -> " << file_path.filename() << "\n";
                
                if (std::system(cmd.c_str()) == 0) {
                    if (!verifyHandlerSymbol(out_so, arch)) {
                        std::cerr << "[VALIDATION ERROR] File '" << file_path.filename() << "' compiled successfully but fails to export a dynamic 'handler' function.\n";
                        std::filesystem::remove(out_so); 
                    } else {
                        std::cout << "[SUCCESS] Generated binary: " << stem << ".so\n";
                    }
                }
                break;
            }

            case Extensions::CPP: {
                std::cout << "Extensions::CPP\n";
                if (!compilerExists(compiler)) {
                    std::cerr << "[ERROR] Target cross-compiler '" << compiler << "' is missing from system utilities paths.\n";
                    return;
                }

                std::string cmd = compiler + " -Wall -Wextra -Werror -Wno-unused-parameter -shared -fPIC -O3 -o " + out_so + " " + file_path.string();
                std::cout << "[BUILD] Compiling C++ [" << arch << "] -> " << file_path.filename() << "\n";
                
                if (std::system(cmd.c_str()) == 0) {
                    if (!verifyHandlerSymbol(out_so, arch)) {
                        std::cerr << "[VALIDATION ERROR] File '" << file_path.filename() << "' compiled successfully but is missing required 'extern \"C\" void handler(...)' linkage definitions.\n";
                        std::filesystem::remove(out_so);
                    } else {
                        std::cout << "[SUCCESS] Generated binary: " << stem << ".so\n";
                    }
                }
                break;
            }

            case Extensions::PY: {
                std::cout << "[BUILD] Python script detected. Generating native embedder shim architecture...\n";
                std::filesystem::path shim_path = file_path.parent_path() / ("_" + stem + "_shim.c");
                
                std::ofstream shim_file(shim_path);
                shim_file << "#include <Python.h>\n"
                          << "#include <stdio.h>\n"
                          << "void handler(const char* req, char* res) {\n"
                          << "    Py_Initialize();\n"
                          << "    PyRun_SimpleString(\"import sys\\nsys.path.append('.')\");\n"
                          << "    PyObject *pName = PyUnicode_DecodeFSDefault(\"" << stem << "\");\n"
                          << "    PyObject *pModule = PyImport_Import(pName);\n"
                          << "    Py_DECREF(pName);\n"
                          << "    if(pModule != NULL) {\n"
                          << "        PyObject *pFunc = PyObject_GetAttrString(pModule, \"handler\");\n"
                          << "        if(pFunc && PyCallable_Check(pFunc)) {\n"
                          << "            PyObject *pArgs = PyTuple_Pack(1, PyUnicode_FromString(req));\n"
                          << "            PyObject *pValue = PyObject_CallObject(pFunc, pArgs);\n"
                          << "            Py_DECREF(pArgs);\n"
                          << "            if(pValue != NULL) {\n"
                          << "                snprintf(res, 4096, \"%s\", PyUnicode_AsUTF8(pValue));\n"
                          << "                Py_DECREF(pValue);\n"
                          << "            } else { snprintf(res, 4096, \"Runtime Error\"); }\n"
                          << "        } else { snprintf(res, 4096, \"Missing handler function\"); }\n"
                          << "        Py_XDECREF(pFunc); Py_DECREF(pModule);\n"
                          << "    } else { snprintf(res, 4096, \"Module Import Error\"); }\n"
                          << "    Py_Finalize();\n"
                          << "}\n";
                shim_file.close();
                std::string python_includes = "-I/usr/include/python3.10"; 
                std::string python_libs = "-lpython3.10";
                if (arch == "arm64" || arch == "aarch64") {
                    python_includes = "-I/usr/include/aarch64-linux-gnu/python3.10";
                    python_libs = "-lpython3.10";
                }
                std::string cmd = compiler + " -Wall -Wextra -shared -fPIC -O3 " + python_includes + 
                                  " -o " + out_so + " " + shim_path.string() + " " + python_libs;
                
                int s_call = std::system(cmd.c_str());
                std::filesystem::remove(shim_path);

                if (s_call == 0) {
                    if (!verifyHandlerSymbol(out_so, arch)) {
                        std::cerr << "[ERROR] Critical Shim Failure: Generated Python container wrapper failed interface validation.\n";
                        std::filesystem::remove(out_so);
                    } else {
                        std::cout << "[SUCCESS] Python script encapsulated natively into: " << stem << ".so\n";
                    }
                }
                break;
            }

            case Extensions::WASM:
            case Extensions::INVALID:
            default:
                break;
        }
    }
}

// this really needs to be refractor :(
void Build::execute(const Command& command)
{
    const auto env_it = command.options.find("--env");

    std::string env_name = env_it != command.options.end() ? env_it->second : "";
    auto project = Project::getProject(command);
    auto env = project ? project->getEnv(env_name) : std::nullopt;

    std::cout << "[BUILD] Executing compilation pipeline...\n";
    std::string input_arg = "";
    try {
        input_arg = command.arguments.at(0);
    } catch (const std::out_of_range& e) {
        input_arg = std::filesystem::current_path();
    }

    const std::filesystem::path input_path(input_arg);

    if (!env) {
        // should compile the .so for all envs arch
        for (auto [key, value] : project->getEnvs()) {
            std::vector<std::filesystem::path> files = collectFiles(input_path, value.arch);
            compile(std::move(files), value.arch);
        }
    } else {
        std::vector<std::filesystem::path> files = collectFiles(input_path, env->arch);
        compile(std::move(files), env->arch);
    }
}