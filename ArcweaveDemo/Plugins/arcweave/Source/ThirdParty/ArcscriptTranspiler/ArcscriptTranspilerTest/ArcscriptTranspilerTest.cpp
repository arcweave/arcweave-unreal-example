// ArcscriptTranspilerTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <windows.h>
#include <any>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum InputType {
    /// @brief A condition (results to bool) code type
    CONDITION,
    /// @brief A script code type
    SCRIPT
};
struct UVariableChange {
    const char* varId;
    const char* type;
    int int_result;
    double double_result;
    const char* string_result;
    bool bool_result;

    UVariableChange() {
        varId = nullptr;
        type = nullptr;
        int_result = 0;
        double_result = 0.0;
        string_result = nullptr;
        bool_result = false;
    }
};

struct UTranspilerOutput {
    const char* output;
    InputType type;
    UVariableChange* changes;
    size_t changesLen = 0;
    bool conditionResult = false;
};


struct UVariable {
    const char* id;
    const char* name;
    const char* type;
    int int_val;
    double double_val;
    const char* string_val;
    bool bool_val;
};

struct UVisit {
    const char* elId;
    int visits;

    UVisit() {
        elId = nullptr;
        visits = 0;
    }
};

typedef UTranspilerOutput(__cdecl* MYPROC)(const char*, const char*, UVariable*, size_t, UVisit*, size_t);

UVariable* getInitialVars(json initialVarsJson) {
    UVariable* initVars = new UVariable[initialVarsJson.size()];
    int i = 0;
    for (json::iterator it = initialVarsJson.begin(); it != initialVarsJson.end(); ++it) {
        std::string id = it.value()["id"].template get<std::string>();
        std::string name = it.value()["name"].template get<std::string>();
        std::string type = it.value()["type"].template get<std::string>();

        initVars[i].id = _strdup(id.c_str());
        initVars[i].name = _strdup(name.c_str());
        initVars[i].type = _strdup(type.c_str());

        if (strcmp(initVars[i].type, "string") == 0) {
            initVars[i].string_val = _strdup(it.value()["value"].template get<std::string>().c_str());
        }
        else if (strcmp(initVars[i].type, "integer") == 0) {
            initVars[i].int_val = it.value()["value"].template get<int>();
        }
        else if (strcmp(initVars[i].type, "double") == 0) {
            initVars[i].double_val= it.value()["value"].template get<double>();
        }
        else if (strcmp(initVars[i].type, "boolean") == 0) {
            initVars[i].bool_val = it.value()["value"].template get<bool>();
        }
        i += 1;
    }

    return initVars;
}

UVisit* getVisits(json initVisits) {
    if (initVisits.size() == 0) return nullptr;
    UVisit* visits = new UVisit[initVisits.size()];
    int i = 0; 
    for (json::iterator it = initVisits.begin(); it != initVisits.end(); ++it) {
        visits[i].elId = _strdup(it.key().c_str());
        visits[i].visits = it.value().template get<int>();
        i += 1;
    }
    return visits;
}

HINSTANCE hinstLib;
MYPROC ProcAdd;
BOOL fFreeResult, fRunTimeLinkSuccess = false;

int testFile(std::filesystem::path path) {
    std::ifstream f(path);
    json data = json::parse(f);
    std::cout << data << std::endl;
    json initVarsJson = data["initialVars"];
    UVariable* initVars = getInitialVars(initVarsJson);
    size_t initVarLen = initVarsJson.size();
    for (json::iterator it = data["cases"].begin(); it != data["cases"].end(); ++it) {
        const char* code = _strdup((*it)["code"].template get<std::string>().c_str());
        UVisit* visits = nullptr;
        size_t visitsLen = 0;
        const char* currentElement = nullptr;
        if ((*it).contains("elementId")) {
            currentElement = _strdup((*it)["elementId"].template get<std::string>().c_str());
        }
        else {
            currentElement = _strdup("TestElement");
        }
        if ((*it).contains("visits")) {
            visits = getVisits((*it)["visits"]);
            visitsLen = (*it)["visits"].size();
        }
        bool hasError = false;
        if ((*it).contains("error")) {
            hasError = true;
        }
        
        UTranspilerOutput result = (ProcAdd)(code, currentElement, initVars, initVarLen, visits, visitsLen);
    }
    return 0;
}



int main()
{

    UTranspilerOutput result;


    hinstLib = LoadLibrary(TEXT("ArcscriptTranspiler.dll"));
    if (hinstLib != NULL) {
        printf("Found hinstLib\n");
        ProcAdd = (MYPROC)GetProcAddress(hinstLib, "runScriptExport");

        if (NULL != ProcAdd) {
            printf("ProcAdd NOT NULL\n");
            fRunTimeLinkSuccess = TRUE;
            //try {
                const std::filesystem::path path{ "D:\\arcweave\\unreal\\arcweave-unreal-example\\ArcweaveDemo\\Plugins\\arcweave\\Source\\arcweave\\test\\valid.json" };
                testFile(path);
                /*result = (ProcAdd)(code.c_str(), elId.c_str(), initVars, 2, visits, 3);
                std::cout << "CHANGES: " << std::endl;
                std::string lines = "";
                for (int i = 0; i < result.changesLen; i++) {
                    std::string line = result.changes[i].varId + std::string(": ");
                    if (strcmp(result.changes[i].type, "string") == 0) {
                        line += result.changes[i].string_result;
                    }
                    else if (strcmp(result.changes[i].type, "integer") == 0) {
                        line += std::to_string(result.changes[i].int_result);
                    }
                    else if (strcmp(result.changes[i].type, "double") == 0) {
                        line += std::to_string(result.changes[i].double_result);
                    }
                    else if (strcmp(result.changes[i].type, "bool") == 0) {
                        line += result.changes[i].bool_result ? "true" : "false";
                    }
                    line += "\n";

                    lines += line;
                }
                std::cout << lines;*/
            /*}
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                throw e;
            }*/
            printf("ProcAdd finished\n");
        }
        else {
            std::cerr << "Get Last Error: " << GetLastError() << std::endl;
        }

        fFreeResult = FreeLibrary(hinstLib);
    }
    else {
        std::cerr << "Get Last Error: " << GetLastError() << std::endl;
    }

    if (!fRunTimeLinkSuccess) {
        printf("Message printed from executable\n");
    }

    //ArcscriptTranspiler transpiler(elId, initVars, visits);
    //TranspilerOutput output = transpiler.runScript("<pre><code>x=5</code></pre>");
    system("pause");
}