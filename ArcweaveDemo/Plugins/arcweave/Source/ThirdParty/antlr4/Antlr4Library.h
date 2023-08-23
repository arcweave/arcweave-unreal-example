#if defined _WIN32 || defined _WIN64
#define ANTLR4LIBRARY_IMPORT __declspec(dllimport)
#elif defined __linux__
#define ANTLR4LIBRARY_IMPORT __attribute__((visibility("default")))
#else
#define ANTLR4LIBRARY_IMPORT
#endif

ANTLR4LIBRARY_IMPORT void Antlr4LibraryFunction();
