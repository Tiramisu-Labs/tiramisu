#include <commands/Build.hpp>

#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <stdlib.h>
#include <Project.hpp>
#include <optional>
#include <fstream>

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
        std::cerr << "[ERROR] Project directory or file path parameter is missing.\n";
        getHelp();
        return;
    }

    const std::filesystem::path input_path(input_arg);
    std::error_code ec;
    
    std::filesystem::path project_root = project->getConfigPath();
    std::vector<std::filesystem::path> files_to_compile;

    if (std::filesystem::is_directory(input_path, ec)) {
        std::cout << "dir file\n";
        
        if (std::filesystem::exists(project_root / "Makefile") || std::filesystem::exists(project_root / "makefile")) {
            std::cout << "[BUILD] Custom Makefile discovered. Bypassing fallback routines.\n";
            std::string make_cmd = "make -C " + project_root.string() + " CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++";
            if (std::system(make_cmd.c_str()) != 0) {
                std::cerr << "[ERROR] Makefile pipeline execution terminated with faults.\n";
            }
            return;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(project_root)) {
            if (entry.is_regular_file()) {
                files_to_compile.push_back(entry.path());
            }
        }
    } else if (std::filesystem::is_regular_file(input_path, ec)) {
        std::cout << "regular file\n";
        files_to_compile.push_back(input_path);

        std::filesystem::path current = input_path.parent_path();
        while (!current.empty() && current != current.root_path()) {
            if (std::filesystem::exists(current / "tiramisu.yaml") || std::filesystem::exists(current / "tiramisu.yml")) {
                project_root = current;
                break;
            }
            current = current.parent_path();
        }

        if (project_root.empty()) {
            std::cout << "[WARN] tiramisu.yaml not found in parent path hierarchy. Falling back to file directory context.\n";
            project_root = input_path.parent_path();
        }
    } else {
        std::cerr << "[ERROR] Provided path is neither a valid directory nor a regular file: " << input_arg << "\n";
        return;
    }
    
    std::string target_arch = env ? env->arch : "native";

    std::string c_compiler = "gcc";
    std::string cpp_compiler = "c++";
    std::string python_includes = "-I/usr/include/python3.10"; 
    std::string python_libs = "-lpython3.10";

    if (target_arch == "arm64" || target_arch == "aarch64") {
        c_compiler = "aarch64-linux-gnu-gcc";
        cpp_compiler = "aarch64-linux-gnu-g++";
        python_includes = "-I/usr/include/aarch64-linux-gnu/python3.10";
        python_libs = "-lpython3.10";
    } else if (target_arch == "x86_64") {
        c_compiler = "x86_64-linux-gnu-gcc";
        cpp_compiler = "x86_64-linux-gnu-g++";
    }

    for (const std::filesystem::path& file_path : files_to_compile) {
        std::cout << "file to compile: " << file_path.c_str() << std::endl;
        std::string ext = file_path.extension().string();
        std::string stem = file_path.stem().string();
        std::string out_so = (file_path.parent_path() / (stem + ".so")).string();
        std::cout << "extension: " << ext << std::endl;
        auto it = extensionsMap_.find(ext);

        switch (it != extensionsMap_.end() ? it->second : Extensions::INVALID) {
            
            case Extensions::C: {
                std::cout << "Extensions::C\n";
                if (!compilerExists(c_compiler)) {
                    std::cerr << "[ERROR] Target cross-compiler '" << c_compiler << "' is missing from system utilities paths.\n"
                              << "Please install the missing toolchain package to proceed.\n";
                    return;
                }
                std::string cmd = c_compiler + " -Wall -Wextra -Werror -Wno-unused-parameter -shared -fPIC -O3 -o " + out_so + " " + file_path.string();
                std::cout << "[BUILD] Compiling C [" << target_arch << "] -> " << file_path.filename() << "\n";
                
                if (std::system(cmd.c_str()) == 0) {
                    if (!verifyHandlerSymbol(out_so, target_arch)) {
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
                if (!compilerExists(cpp_compiler)) {
                    std::cerr << "[ERROR] Target cross-compiler '" << cpp_compiler << "' is missing from system utilities paths.\n";
                    return;
                }

                std::string cmd = cpp_compiler + " -Wall -Wextra -Werror -Wno-unused-parameter -shared -fPIC -O3 -o " + out_so + " " + file_path.string();
                std::cout << "[BUILD] Compiling C++ [" << target_arch << "] -> " << file_path.filename() << "\n";
                
                if (std::system(cmd.c_str()) == 0) {
                    if (!verifyHandlerSymbol(out_so, target_arch)) {
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

                std::string cmd = c_compiler + " -Wall -Wextra -shared -fPIC -O3 " + python_includes + 
                                  " -o " + out_so + " " + shim_path.string() + " " + python_libs;
                
                int s_call = std::system(cmd.c_str());
                std::filesystem::remove(shim_path);

                if (s_call == 0) {
                    if (!verifyHandlerSymbol(out_so, target_arch)) {
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
