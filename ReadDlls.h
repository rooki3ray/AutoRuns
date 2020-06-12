#ifndef READDLLS_H
#define READDLLS_H
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <shlwapi.h>
//HRESULT SHLoadIndirectString(
//  _In_        PCWSTR pszSource,
//  _Out_       PWSTR pszOutBuf,
//  _In_        UINT cchOutBuf,
//  _Reserved_  void **ppvReserved
//);
int ReadDllDescription(TCHAR* szOut, PCWSTR PATH)
{
    if (SHLoadIndirectString(PATH,  szOut, 1024, NULL) == S_OK)
        return 1;
    return 0;
}
#endif // READDLLS_H
