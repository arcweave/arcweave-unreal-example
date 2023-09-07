// ArcscriptTranspilerTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include <string>
#include <windows.h>
#include <any>

enum InputType {
    /// @brief A condition (results to bool) code type
    CONDITION,
    /// @brief A script code type
    SCRIPT
};
struct UVariableChange {
    char* varId;
    const char* type;
    int int_result;
    double double_result;
    const char* string_result;
    bool bool_result;
};

struct UTranspilerOutput {
    const char* output;
    InputType type;
    UVariableChange* changes;
    int changesLen = 0;
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

typedef UTranspilerOutput(__cdecl* MYPROC)(const char*, const char*, UVariable*, int, UVisit*, int);

int main()
{
    UVariable* initVars = new UVariable[2];
    initVars[0].id = "var1";
    initVars[0].name = "x";
    initVars[0].int_val = 2;
    initVars[0].type = "integer";

    initVars[1].id = "var4";
    initVars[1].name = "w";
    initVars[1].string_val = "Dummy text";
    initVars[1].type = "string";

    UVisit* visits = new UVisit[3];
    visits[0].elId = "el1";
    visits[0].visits = 2;
    visits[1].elId = "el2";
    visits[1].visits = 9;
    visits[2].elId = "el3";
    visits[2].visits = 0;

    std::string code = "<pre><code>x=5</code></pre>";
    std::string elId = "testElementId";

    UTranspilerOutput result;

    HINSTANCE hinstLib;
    MYPROC ProcAdd;
    BOOL fFreeResult, fRunTimeLinkSuccess = false;

    hinstLib = LoadLibrary(TEXT("ArcscriptTranspiler.dll"));
    if (hinstLib != NULL) {
        printf("Found hinstLib\n");
        ProcAdd = (MYPROC)GetProcAddress(hinstLib, "runScriptExport");

        if (NULL != ProcAdd) {
            printf("ProcAdd NOT NULL\n");
            fRunTimeLinkSuccess = TRUE;
            try {
                result = (ProcAdd)(code.c_str(), elId.c_str(), initVars, 2, visits, 3);
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
                std::cout << lines;
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
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