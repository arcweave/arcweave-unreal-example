#if defined _WIN64
#pragma once
#include "ArcscriptTranspiler.h"
#include "ArcscriptErrorListener.h"
#include <sstream>
#include <iterator>
#include <windows.h>
#endif

#define _CRT_SECURE_NO_WARNINGS

using namespace Arcweave;
using namespace antlr4;

TranspilerOutput ArcscriptTranspiler::runScript(std::string code) {
  ANTLRInputStream input(code);
  ArcscriptLexer lexer(&input);

  TranspilerOutput result;

  ErrorListener lexerErrorListener;
  lexer.removeErrorListeners();
  lexer.addErrorListener(&lexerErrorListener);
  
  CommonTokenStream tokens(&lexer);

  // Run the lexer
  tokens.fill();

  ArcscriptParser parser(&tokens);
  parser.setArcscriptState(&state);
  ErrorListener parserErrorListener;
  parser.removeErrorListeners();
  parser.addErrorListener(&parserErrorListener);

  ArcscriptParser::InputContext *tree;

  // Run the parser
  tree = parser.input();
  
  ArcscriptVisitor visitor(&state);
  
  // Run the visitor
  std::any res(visitor.visitInput(tree));

  result.changes = visitor.state->variableChanges;
  std::vector<std::string> outputs = visitor.state->outputs;

  // concatenate the outputs
  const char* const delim = "";
  std::ostringstream imploded;
  std::copy(outputs.begin(), outputs.end(), std::ostream_iterator<std::string>(imploded, delim));
  result.output = imploded.str();
  result.result = res;
  
  if (tree->script() != NULL) {
    result.type = SCRIPT;
  } else {
    result.type = CONDITION;
  }

  return result;
}

ARCSCRIPTTRANSPILER_API UTranspilerOutput runScriptExport(const char* code, const char* elId, UVariable* variables, int varLength, UVisit* visits, int visitsLength)
{
    Arcweave::TranspilerOutput transpilerOutput;
    transpilerOutput.output = "THIS IS THE RESULT";
    transpilerOutput.type = InputType::CONDITION;

    std::string sCode(code);
    std::string sElId(elId);

    std::map<std::string, Variable> initVars;
    for (int i = 0; i < varLength; i++) {
        Variable var;
        var.id = std::string(variables[i].id);
        var.name = std::string(variables[i].name);
        var.type = std::string(variables[i].type);
        
        if (strcmp(variables[i].type, "string") == 0) {
            var.value = std::string(variables[i].string_val);
        }
        else if (strcmp(variables[i].type, "integer") == 0) {
            var.value = variables[i].int_val;
        }
        else if (strcmp(variables[i].type, "double") == 0) {
            var.value = variables[i].double_val;
        }
        else if (strcmp(variables[i].type, "bool") == 0) {
            var.value = variables[i].bool_val;
        }
        initVars[variables[i].id] = var;
    }

    std::map<std::string, int> initVisits;
    for (int i = 0; i < visitsLength; i++) {
        initVisits[std::string(visits[i].elId)] = visits[i].visits;
    }

    Arcweave::ArcscriptTranspiler transpiler(sElId, initVars, initVisits);
    transpilerOutput = transpiler.runScript(sCode);

    UTranspilerOutput uTranspilerOutput;
    uTranspilerOutput.output = new char[transpilerOutput.output.size() + 1];
    strcpy_s(uTranspilerOutput.output, transpilerOutput.output.size() + 1, transpilerOutput.output.c_str());
    uTranspilerOutput.type = transpilerOutput.type;
    
    if (transpilerOutput.type == InputType::CONDITION) {
        uTranspilerOutput.conditionResult = std::any_cast<bool>(transpilerOutput.result);
    }

    int changesLen = transpilerOutput.changes.size();
    std::cout << "Changes Len: " << changesLen << std::endl;
    UVariableChange* variableChanges = new UVariableChange[changesLen];
    int i = 0;
    for (auto change : transpilerOutput.changes) {
        UVariableChange uChange;
        uChange.varId = _strdup(change.first.c_str());

        if (change.second.type() == typeid(std::string)) {
            uChange.type = "string";
            std::string string_result = std::any_cast<std::string>(change.second);
            uChange.string_result = _strdup(string_result.c_str());
        }
        else if (change.second.type() == typeid(int)) {
            uChange.type = "integer";
            uChange.int_result = std::any_cast<int>(change.second);
        }
        else if (change.second.type() == typeid(double)) {
            uChange.type = "double";
            uChange.double_result = std::any_cast<double>(change.second);
        }
        else if (change.second.type() == typeid(bool)) {
            uChange.type = "bool";
            uChange.bool_result = std::any_cast<bool>(change.second);
        }
        variableChanges[i] = uChange;
        i++;
    }
    uTranspilerOutput.changes = variableChanges;
    uTranspilerOutput.changesLen = changesLen;
    return uTranspilerOutput;
    //std::cout << code << std::endl;
    //std::cout << elId << std::endl;
    //std::cout << _visits["test"] << std::endl;
    /*try {
        Arcweave::ArcscriptTranspiler transpiler(elId, initVars, _visits);
        
        try {
            transpilerOutput = transpiler.runScript(code);
        }
        catch (std::exception& e) {
            std::cerr << "Arcscript Transpiler failed during runScript: " << std::endl;
            std::cerr << e.what() << std::endl;
            return false;
        }
    }
    catch (std::exception &e) {
        std::cerr << "Arcscript Transpiler failed during init: " << std::endl;
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;*/
}
