#if defined _WIN32 || defined _WIN64
    #include <Windows.h>

    #define ANTLR4LIBRARY_EXPORT __declspec(dllexport)
#else
    #include <stdio.h>
#endif

#ifndef ANTLR4LIBRARY_EXPORT
    #define ANTLR4LIBRARY_EXPORT
#endif

ANTLR4LIBRARY_EXPORT void Antlr4LibraryFunction()
{
#if defined _WIN32 || defined _WIN64
	MessageBox(NULL, TEXT("Loaded antlr4-runtime.dll from Third Party Plugin sample."), TEXT("antlr4 Plugin"), MB_OK);
#else
    printf("Loaded antlr4 from arcware Plugin");
#endif
}