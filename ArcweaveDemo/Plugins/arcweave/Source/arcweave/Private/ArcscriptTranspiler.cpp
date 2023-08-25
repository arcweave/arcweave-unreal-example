#include "ArcscriptTranspiler.h"
#include "ArcscriptErrorListener.h"
#include <sstream>
#include <iterator>

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