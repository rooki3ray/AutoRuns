#ifndef STRINGCONVERT_H
#define STRINGCONVERT_H
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <string>
#include <windows.h>
#include <QString>

// 将TCHAR*转换为std::string
std::string TCHAR_STRING(TCHAR *STR)
{
    int l = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, NULL);
    char*  ch = new char[sizeof(char) * l];
    WideCharToMultiByte(CP_ACP, 0, STR, -1, ch, l, NULL, NULL);
    std::string str(ch);
    delete [] ch;
    return str;
};

// 将QString转换为LPCTSTR(const wchar_t *)
const wchar_t * QSTRING_LPCTSTR(QString qstring)
{
    const wchar_t * str = reinterpret_cast<const wchar_t *>(qstring.utf16());
    return str;
}
#endif // STRINGCONVERT_H
