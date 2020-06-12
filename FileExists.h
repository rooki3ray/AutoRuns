#ifndef FILEEXISTS_H
#define FILEEXISTS_H
#include "windows.h"
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>
#include <Qt>
#include <QString>
#include <QDebug>
//int fileExists(LPCWSTR filename)
//{
//      WIN32_FIND_DATA FindFileData;
//      HANDLE hFind;
//      printf ("Target file is %s. ", argv[1]);
//      hFind = FindFirstFile(L"c:\\Windows\\System32\\drivers\\ndis.sys", &FindFileData);
//      if (hFind == INVALID_HANDLE_VALUE)
//      {
//           printf ("Invalid File Handle. Get Last Error reports %d ", GetLastError ());
//            qDebug()<<"NOT";
//          return 0;
//      }
//      else
//      {
//          qDebug()<<"YES";

//           printf ("The first file found is %s ", FindFileData.cFileName);
//          FindClose(hFind);
//          return 1;
//      }
//}

int fileExists(char* filepath)
{
   /* Check for existence */
   if( !(_access(filepath, 0 )) )
   {
//      printf( "File exists " );
      return 1;
      /* Check for write permission */
//      if( (_access( "ACCESS.C", 2 )) != -1 )
//         printf( "File ACCESS.C has write permission " );
   }
//    printf( "File not exists " );
    return 0;
}
#endif // FILEEXISTS_H
