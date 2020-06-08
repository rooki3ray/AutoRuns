#ifndef TASKS_H
#define TASKS_H
/********************************************************************
 This sample enumerates through the tasks on the local computer and
 displays their name and state.
********************************************************************/


/********************************************************************
 This sample enumerates through the tasks on the local computer and
 displays their name and state.
********************************************************************/

#define _WIN32_DCOM

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <comdef.h>
#include <tchar.h>

//  Include the task header file.
#include <taskschd.h>
//#pragma comment(lib, "taskschd.lib")
//#pragma comment(lib, "comsupp.lib")

using namespace std;
struct tasks
{
    char* folderName;
    char* taskName;
    char* imagePath;
};

bool walkFolders(ITaskFolder* rootFolder, HRESULT hr, BSTR folderName, tasks* list, int* length);
void getTasks(ITaskFolder* rootFolder, HRESULT hr, BSTR folderName, tasks* list, int* length);

int allTasks(tasks* list, int* length)
{
    //  ------------------------------------------------------
    //  Initialize COM.
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
//        printf("\nCoInitializeEx failed: %x", hr);
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);

    if (FAILED(hr))
    {
//        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create an instance of the Task Service.
    ITaskService* pService = NULL;
    hr = CoCreateInstance(CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void**)&pService);
    if (FAILED(hr))
    {
//        printf("\nFailed to CoCreate an instance of the TaskService class: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(),
        _variant_t(), _variant_t());
    if (FAILED(hr))
    {
//        printf("\nITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the pointer to the root task folder.
    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
//        printf("\nCannot get Root Folder pointer: %x", hr);
        return 1;
    }
    BSTR name = NULL;
    hr = pRootFolder->get_Name(&name);
    if (FAILED(hr))
    {
//        printf("\nCannot get Folder name: %x", hr);
        return false;
    }

//    printf("\n\nFolder Name: %S", name);
    walkFolders(pRootFolder, hr, name, list, length);
    SysFreeString(name);
    //getTasks(pRootFolder, hr);
    pRootFolder->Release();
//    printf("\n---------------------------------------------------%d", length);
//    for (int i = 0; i < *length; i++)
//    {
//        printf("%s\t%s\t%s\n", list[i].folderName, list[i].taskName, list[i].imagePath);
//    }
    pService->Release();
    CoUninitialize();
    return 0;
}

// Recursive search sub-folders
bool walkFolders(ITaskFolder* rootFolder, HRESULT hr, BSTR folderName, tasks* list, int* length) {
    ITaskFolderCollection* pFolders = NULL;
    hr = rootFolder->GetFolders(0, &pFolders);
    getTasks(rootFolder, hr, folderName, list, length);
    if (FAILED(hr))
    {
//        printf("\nCannot get Folders: %x", hr);
        return false;
    }

    LONG numFolders = 0;
    hr = pFolders->get_Count(&numFolders);
    //printf("Number of Folders:%d", numFolders);

    if (numFolders != 0) {
        for (LONG i = 0; i < numFolders; i++) {
            ITaskFolder* pRootFolder = NULL;
            hr = pFolders->get_Item(_variant_t(i + 1), &pRootFolder);
            if (SUCCEEDED(hr)) {
                BSTR name = NULL;
                hr = pRootFolder->get_Name(&name);
                if (FAILED(hr))
                {
//                    printf("\nCannot get Folder name: %x", hr);
                    return false;
                }
//                printf("\n\nFolder Name: %S", name);
                char* name_ch = _com_util::ConvertBSTRToString(name);
                char* folderName_ch = _com_util::ConvertBSTRToString(folderName);
                std::string const &cc = std::string(name_ch) + "\\" + std::string(folderName_ch);
                char const* c = cc.c_str();
                BSTR folderName_BSTR = _com_util::ConvertStringToBSTR(c);
                walkFolders(pRootFolder, hr, folderName_BSTR, list, length);
                SysFreeString(name);
            }
            else
            {
//                printf("\n\tCannot get the folder name: %x", hr);
                return false;
            }
        }
        pFolders->Release();
    }
    return true;
}

// Get the registered tasks in the folder
void getTasks(ITaskFolder* rootFolder, HRESULT hr, BSTR folderName, tasks* list, int* length) {
    IRegisteredTaskCollection* pTaskCollection = NULL;
    hr = rootFolder->GetTasks(NULL, &pTaskCollection);

    if (FAILED(hr))
    {
//        printf("\n\tCannot get the registered tasks.: %x", hr);
        return;
    }

    LONG numTasks = 0;
    hr = pTaskCollection->get_Count(&numTasks);

    if (numTasks == 0)
    {
//        printf("\n\tNo Tasks are currently running");
        pTaskCollection->Release();
        return;
    }

    //printf("\nNumber of Tasks : %d", numTasks);

    TASK_STATE taskState;

    for (LONG i = 0; i < numTasks; i++)
    {
        IRegisteredTask* pRegisteredTask = NULL;
        hr = pTaskCollection->get_Item(_variant_t(i + 1), &pRegisteredTask);

        if (SUCCEEDED(hr))
        {
            BSTR taskName = NULL;
            hr = pRegisteredTask->get_Name(&taskName);
            if (SUCCEEDED(hr))
            {
//                printf("\n\tTaskName: %S", taskName);

                list[*length].folderName = _com_util::ConvertBSTRToString(folderName);
                list[*length].taskName = _com_util::ConvertBSTRToString(taskName);
                SysFreeString(taskName);
                hr = pRegisteredTask->get_State(&taskState);
                if (SUCCEEDED(hr))
                {
//                    printf("\n\tState: %d", taskState);
                    ;
                }
                else
                {
                    printf("\n\tCannot get the registered task state: %x", hr);
                    ;
                }


                ITaskDefinition* taskDefination = NULL;
                hr = pRegisteredTask->get_Definition(&taskDefination);
                if (FAILED(hr))
                {
//                    printf("\n\tCannot get the task defination: %x", hr);
                    return;
                }

                IActionCollection* taskActions = NULL;
                hr = taskDefination->get_Actions(&taskActions);
                if (FAILED(hr))
                {
//                    printf("\n\tCannot get the task actions: %x", hr);
                    return;
                }
                taskDefination->Release();

                /*LONG numActions = 0;
                hr = taskActions->get_Count(&numActions);
                if (SUCCEEDED(hr))
                    printf("\n\tCount of Actions: %d", numActions);
                else
                    printf("\n\tCannot get the number of actions: %x", hr);*/

                IAction* action = NULL;
                hr = taskActions->get_Item(1, &action);
                if (FAILED(hr))
                {
//                    printf("\n\tCannot get the action: %x", hr);
                    return;
                }
                taskActions->Release();

                IExecAction* execAction = NULL;
                hr = action->QueryInterface(IID_IExecAction, (void**)&execAction);
                if (FAILED(hr))
                {
//                    printf("\n\tQueryInterface call failed for IExecAction: %x", hr);
                    return;
                }
                action->Release();

                BSTR imagePath = NULL;
                hr = execAction->get_Path(&imagePath);
                if (SUCCEEDED(hr))
                {
//                    printf("\n\tImage Path: %S", imagePath);

                    list[*length].imagePath = _com_util::ConvertBSTRToString(imagePath);
                    *length += 1;
                }
                else
                {
//                    printf("\n\tCannot get the image path: %x", hr);
                    ;
                }
                execAction->Release();
            }
            else
            {
//                printf("\n\tCannot get the registered task name: %x", hr);
                ;
            }
            pRegisteredTask->Release();
        }
        else
        {
//            printf("\n\tCannot get the registered task item at index=%d: %x", i + 1, hr);
            ;
        }
    }
    pTaskCollection->Release();
}

#endif // TASKS_H
