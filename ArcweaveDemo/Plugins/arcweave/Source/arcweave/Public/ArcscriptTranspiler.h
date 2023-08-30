#pragma once

#include "antlr4/runtime/src/antlr4-runtime.h"
#include "ArcscriptLexer.h"
#include "ArcscriptParser.h"
#include "ArcscriptVisitor.h"

namespace Arcweave
{
  /**
   * @enum InputType
   * The arscript code types
  */
  enum InputType { 
    /// @brief A condition (results to bool) code type
    CONDITION,
    /// @brief A script code type
    SCRIPT
  };

  /**
   * @struct TranspilerOutput
   * @brief The structure contains info that the ArcscriptTranspiler returns
   * @var TranspilerOutput::output
   * Member 'output' contains the text output when a codeblock is run
   * @var TranspilerOutput::changes
   * Member 'changes' contains the variables that are going to be changed from the codeblock
   * @var TranspilerOutput::result
   * Member 'result' contains the result of the ArcscriptTranspiler code. Useful in condition blocks
   * @var TranspilerOutput::type
   * Member 'type' contains the type of the code block i.e. CONDITION or SCRIPT
  */
  struct TranspilerOutput {
    std::string output;
    std::map<std::string, std::any> changes;
    std::any result;
    InputType type;
  };
  
  /**
   * Implementation of the Arcscript Transpiler in C++. Uses the ANTLR4 runtime library
   * to run the code and it needs the initial variables and the current element ID it is
   * transpiling.
   * 
   * 
  */
  class ArcscriptTranspiler {
  public:

    ArcscriptState state;

    ArcscriptTranspiler(std::string elId, std::map<std::string, Variable> initVars, std::map<std::string, int> _visits) : state(elId, initVars, _visits) { };

    /**
     * Runs the arcscript code and returns it's results.
     * @param code The code block that we need to parse
     * @return The result of the ran script
    */
    TranspilerOutput runScript(std::string code);
    void  runScript2(std::string code);
  };
};