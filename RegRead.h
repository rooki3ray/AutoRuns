#ifndef REG_H
#define REG_H
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <string>
#include "StringConvert.h"
#include <Qt>
#include <QString>
#include <QDebug>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define HKLM HKEY_LOCAL_MACHINE
#define HKCU HKEY_CURRENT_USER
struct KEY_VALUE
{
    TCHAR key[MAX_KEY_LENGTH];
    BYTE value[MAX_VALUE_NAME];
    BYTE type[1];
    int keylen;
    int valuelen;
};
bool QueryKey(HKEY hKey, KEY_VALUE* list, int* length, int mode);
void GetKeyValue(HKEY hKey, LPCSTR lpSubKey, KEY_VALUE* list, int* length);
void GetGroupKeyValue(HKEY hKey, LPCSTR lpSubKey, KEY_VALUE* list, int* length);

bool QueryKey(HKEY hKey, KEY_VALUE* list, int* length, int mode)
{
//    TCHAR achKey[MAX_KEY_LENGTH];        // buffer for subkey name
    DWORD cbName;                        // size of name string
    TCHAR achClass[MAX_PATH] = TEXT(""); // buffer for class name
    DWORD cchClassName = MAX_PATH;       // size of class string
    DWORD cSubKeys = 0;                  // number of subkeys
    DWORD cbMaxSubKey;                   // longest subkey size
    DWORD cchMaxClass;                   // longest class string
    DWORD cValues;                       // number of values for key
    DWORD cchMaxValue;                   // longest value name
    DWORD cbMaxValueData;                // longest value data
    DWORD cbSecurityDescriptor;          // size of security descriptor
    FILETIME ftLastWriteTime;            // last write time

    DWORD i, retCode;

//    TCHAR achValue[MAX_VALUE_NAME];
//    BYTE achData[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD cchData = MAX_VALUE_NAME;
    DWORD Type;

    // Get the class name and the value count.
    retCode = RegQueryInfoKey(
        hKey,                  // key handle
        achClass,              // buffer for class name
        &cchClassName,         // size of class string
        NULL,                  // reserved
        &cSubKeys,             // number of subkeys
        &cbMaxSubKey,          // longest subkey size
        &cchMaxClass,          // longest class string
        &cValues,              // number of values for this key
        &cchMaxValue,          // longest value name
        &cbMaxValueData,       // longest value data
        &cbSecurityDescriptor, // security descriptor
        &ftLastWriteTime);     // last write time
    // Enumerate the key values.
    if (mode == 0)
    {
        *length = 0;
        if (cValues)
        {
            for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
            {
                cchData = MAX_VALUE_NAME;
                cchValue = MAX_VALUE_NAME;
                list[i].key[0] = '\0';
                list[i].value[0] = '\0';
                retCode = RegEnumValue(hKey, i, list[i].key, &cchValue, NULL, &Type, list[i].value, &cchData);
                if (retCode == ERROR_SUCCESS)
                {
                    list[i].keylen = cchValue;
                    list[i].valuelen = cchData;
                    *length += 1;
                }
            }
        }
        return 1;
    }
    // Enumerate the subkeys, until RegEnumKeyEx fails.
    else if (mode == 1)
    {
        if (cSubKeys)
        {
            for (i = 0; i < cSubKeys; i++)
            {

                cbName = MAX_KEY_LENGTH;
                retCode = RegEnumKeyEx(hKey, i,
                                       list[i].key,
                                       &cbName,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &ftLastWriteTime);
                if (retCode == ERROR_SUCCESS)
                {
                    list[i].keylen = cbName;
                    *length += 1;
                }
            }
        }
        return 1;
    }
    else if (mode == 2)
    {
//        KEY_VALUE* list_tmp = new KEY_VALUE[16];
        bool autorunsFlag = 0;
        if (cValues)
        {
            KEY_VALUE *tmp_kv = new KEY_VALUE [cValues];
            int index_description = -1,
                index_imagepath = -1,
                index_type = -1;
            for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
            {
                cchData = MAX_VALUE_NAME;
                cchValue = MAX_VALUE_NAME;
                retCode = RegEnumValue(hKey, i, tmp_kv[i].key, &cchValue, NULL, &Type, tmp_kv[i].value, &cchData);
                if (retCode == ERROR_SUCCESS)
                {
                    tmp_kv[i].keylen = cchValue;
                    tmp_kv[i].valuelen = cchData;
                }
                std::string tmp = TCHAR_STRING(tmp_kv[i].key);
//                QString key1 = "";
//                QString value1 = "";
//                for(int j = 0; j < tmp_kv[i].keylen; j++)
//                {
//                    key1 += (char)tmp_kv[i].key[j];
//                }
//                for(int j = 0; j < tmp_kv[i].valuelen; j=j+2)
//                {
//                    value1 += ((char)tmp_kv[i].value[j]);
//                }
//                qDebug()<<key1<<value1;
                std::string start = "Start";
                std::string imagepath = "ImagePath";
                std::string description = "Description";
                std::string type = "Type";
                std::string::size_type idx = tmp.find(start);
                std::string::size_type idx2 = tmp.find(imagepath);
                std::string::size_type idx3 = tmp.find(description);
                std::string::size_type idx4 = tmp.find(type);

                if (idx != std::string::npos && tmp_kv[i].value[0] <= 2)
                {
                    autorunsFlag = 1;
                }
                if (idx2 != std::string::npos)
                {
                    index_imagepath = i;
                }
                if (idx3 != std::string::npos)
                {
                    index_description = i;
                }
                if (idx4 != std::string::npos)
                {
                    index_type = i;
                }
            }
            if(autorunsFlag && index_imagepath > 0)
            {
                if (index_type > 0)
                {
                    list[*length].type[0] = tmp_kv[index_type].value[0];
                }
                if (index_description > 0 && ((char)tmp_kv[index_description].value[0]) == '@')      // description的优先级
                {
                    list[*length].valuelen = tmp_kv[index_description].valuelen;
                    for(int j = 0; j < tmp_kv[index_description].valuelen; j++)
                    {
                        list[*length].value[j] = tmp_kv[index_description].value[j];
                    }
                }
                else
                {
                    list[*length].valuelen = tmp_kv[index_imagepath].valuelen;
                    for(int j = 0; j < tmp_kv[index_imagepath].valuelen; j++)
                    {
                        list[*length].value[j] = tmp_kv[index_imagepath].value[j];
                    }
                }

                *length += 1;
                delete [] tmp_kv;
                return 1;
            }
            return 0;
        }
        else
        return 0;
    }
}

void GetKeyValue(HKEY hKey, LPCSTR lpSubKey, KEY_VALUE* list, int* length)
{
    HKEY* hTestKey = new HKEY;
    if (RegOpenKeyExA(hKey, lpSubKey, 0, KEY_READ, hTestKey) == ERROR_SUCCESS)
    {
        QueryKey(*hTestKey, list, length, 0);
    }
    RegCloseKey(*hTestKey);
    delete hTestKey;
}

void GetGroupKeyValue(HKEY hKey, LPCSTR lpSubKey, KEY_VALUE* list, int* length)
{
    QStringList result_tmp[1];
    HKEY hTestKey;
    int len=0;
    KEY_VALUE* list_grp = new KEY_VALUE[1024];
    if (RegOpenKeyExA(hKey, lpSubKey, 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
    {
        QueryKey(hTestKey, list_grp, &len, 1);
    }
    RegCloseKey(hTestKey);
    *length = 0;
    for (int i = 0; i < len; i++)
    {
        std::string str = TCHAR_STRING(list_grp[i].key);
//        qDebug()<<QString::fromStdString((std::string)lpSubKey + "\\" + str);
        std::string tmp = (std::string)lpSubKey + "\\" + str;
        LPCSTR new_lpSubKey = tmp.c_str();
//                printf("%s\n", new_lpSubKey);

        HKEY* hTestKey_ = new HKEY;
        if (RegOpenKeyExA(hKey, new_lpSubKey, 0, KEY_READ, hTestKey_) == ERROR_SUCCESS)
        {
            if (QueryKey(*hTestKey_, list, length, 2))
            {
                list[(*length)-1].keylen = list_grp[i].keylen;
                for(int j = 0; j < list_grp[i].keylen; j++)
                {
                    list[(*length)-1].key[j] = (char)list_grp[i].key[j];
                }
            }
        }
        RegCloseKey(*hTestKey_);
        delete hTestKey_;
    }
    delete[] list_grp;
}
#endif // REG_H

