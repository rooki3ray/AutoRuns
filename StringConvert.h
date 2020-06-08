#ifndef STRINGCONVERT_H
#define STRINGCONVERT_H
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <string>
#include <windows.h>

std::string TCHAR_STRING(TCHAR *STR)
{
    int l = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, NULL);
    char*  ch = new char[sizeof(char) * l];
    WideCharToMultiByte(CP_ACP, 0, STR, -1, ch, l, NULL, NULL);
    std::string str(ch);
    delete [] ch;
    return str;
}

#endif // STRINGCONVERT_H
