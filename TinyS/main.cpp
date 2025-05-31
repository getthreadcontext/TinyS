#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <vector>
#include <string> // For std::string
#include <comdef.h> // For _bstr_t, _com_error
#include "resource.h"

#pragma comment(lib, "comctl32.lib")

#import "SeliwareWrapper.tlb" no_namespace rename_search_namespace("SeliwareWrapper")

// --- Structs and Global Variables ---
struct TabInfo {
    HWND hEdit;
    TCHAR fileName[MAX_PATH];
    bool modified;
};

HWND hTab, hTabAdd, hTabClose;
HWND hBtnOpen, hBtnSave, hBtnClear, hBtnRun; 
HWND hBtnInject;
HINSTANCE hInst;
std::vector<TabInfo> tabs;
int currentTab = -1;

ISeliwareWrapperPtr seliwareComPtr;
bool seliwareSuccessfullyInitialized = false; 

// --- Function Declarations ---
void SwitchToTab(int index);
void CreateNewTab(const TCHAR* name = TEXT("Untitled"));
void CloseTab(int index);
void OpenFile();
void SaveFile();
void ResizeCurrentEditor(HWND hwnd);

void SeliwareAutoInitialize(HWND hwndParent); 
void SeliwareInject(HWND hwndParent);         
void SeliwareExecute(HWND hwndParent);       

// --- Function Implementations ---

void ResizeCurrentEditor(HWND hwnd) {
    if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].hEdit) {
        RECT rect;
        GetClientRect(hwnd, &rect);
        MoveWindow(tabs[currentTab].hEdit, 5, 40, rect.right - 10, rect.bottom - 80, TRUE);
    }
}

void SwitchToTab(int index) {
    if (index < 0 || index >= tabs.size()) return;
    HWND hwnd = GetParent(hTab);

    if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].hEdit) {
        ShowWindow(tabs[currentTab].hEdit, SW_HIDE);
    }
    currentTab = index;

    if (!tabs[currentTab].hEdit) {
        RECT rect;
        GetClientRect(hwnd, &rect);
        tabs[currentTab].hEdit = CreateWindow(
            TEXT("EDIT"), TEXT(""),
            WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            5, 40, rect.right - 10, rect.bottom - 80,
            hwnd, NULL, hInst, NULL);
        SendMessage(tabs[currentTab].hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    }
    ShowWindow(tabs[currentTab].hEdit, SW_SHOW);
    ResizeCurrentEditor(hwnd);
    SetFocus(tabs[currentTab].hEdit);
}

void CreateNewTab(const TCHAR* name) {
    TabInfo newTab = { 0 };
    wcscpy_s(newTab.fileName, name);
    newTab.modified = false;

    TCITEM tie = { 0 };
    tie.mask = TCIF_TEXT;
    tie.pszText = (TCHAR*)name;

    int index = TabCtrl_InsertItem(hTab, tabs.size(), &tie);
    tabs.push_back(newTab);

    TabCtrl_SetCurSel(hTab, index);
    SwitchToTab(index);
}

void CloseTab(int index) {
    if (index < 0 || index >= tabs.size() || tabs.size() <= 1) return;

    if (tabs[index].hEdit) {
        DestroyWindow(tabs[index].hEdit);
    }
    TabCtrl_DeleteItem(hTab, index);
    tabs.erase(tabs.begin() + index);

    if (currentTab >= index) {
        currentTab--;
        if (currentTab < 0 && !tabs.empty()) currentTab = 0;
        else if (tabs.empty()) currentTab = -1;
    }
    if (currentTab != -1) {
        TabCtrl_SetCurSel(hTab, currentTab);
        SwitchToTab(currentTab);
    }
}

void OpenFile() {
    OPENFILENAME ofn = { 0 };
    TCHAR szFile[MAX_PATH] = TEXT("");
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetParent(hTab);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = TEXT("Lua Files\0*.lua\0Text Files\0*.txt\0All Files\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        HANDLE hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize != INVALID_FILE_SIZE) {
                char* buffer = new char[fileSize + 1];
                DWORD bytesRead;
                if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                    buffer[bytesRead] = '\0';
                    const TCHAR* tabName = wcsrchr(szFile, '\\');
                    CreateNewTab(tabName ? tabName + 1 : szFile);
                    SetWindowTextA(tabs[currentTab].hEdit, buffer);
                    wcscpy_s(tabs[currentTab].fileName, szFile);
                    tabs[currentTab].modified = false;
                }
                delete[] buffer;
            }
            CloseHandle(hFile);
        }
    }
}

void SaveFile() {
    if (currentTab < 0 || currentTab >= tabs.size()) return;

    TCHAR* fileNameToSave = tabs[currentTab].fileName;
    bool promptForName = (wcslen(fileNameToSave) == 0 || wcscmp(fileNameToSave, TEXT("Untitled")) == 0);

    if (promptForName) {
        OPENFILENAME ofn = { 0 };
        TCHAR szFile[MAX_PATH] = TEXT("");
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetParent(hTab);
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = TEXT("Lua Files\0*.lua\0Text Files\0*.txt\0All Files\0*.*\0");
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_OVERWRITEPROMPT;

        if (!GetSaveFileName(&ofn)) return;
        wcscpy_s(tabs[currentTab].fileName, MAX_PATH, szFile);
        fileNameToSave = tabs[currentTab].fileName;

        TCITEM tie = { 0 };
        tie.mask = TCIF_TEXT;
        const TCHAR* tabName = wcsrchr(fileNameToSave, '\\');
        tie.pszText = (TCHAR*)(tabName ? tabName + 1 : fileNameToSave);
        TabCtrl_SetItem(hTab, currentTab, &tie);
    }

    HANDLE hFile = CreateFile(fileNameToSave, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        int len = GetWindowTextLength(tabs[currentTab].hEdit);
        char* buffer = new char[len + 1];
        GetWindowTextA(tabs[currentTab].hEdit, buffer, len + 1);
        DWORD bytesWritten;
        WriteFile(hFile, buffer, len, &bytesWritten, NULL);
        delete[] buffer;
        CloseHandle(hFile);
        tabs[currentTab].modified = false;
    }
    else {
        MessageBox(GetParent(hTab), TEXT("Failed to save file."), TEXT("Error"), MB_ICONERROR);
    }
}

// --- Seliware C++ Helper Functions ---
void SeliwareAutoInitialize(HWND hwndParent) { 
    if (!seliwareComPtr) {
        MessageBox(hwndParent, TEXT("Seliware COM object not available."), TEXT("Initialization Error"), MB_ICONERROR);
        seliwareSuccessfullyInitialized = false;
        return;
    }
    try {
        seliwareComPtr->Initialize();
        seliwareSuccessfullyInitialized = true;
        
    }
    catch (_com_error& e) {
        TCHAR errorMsg[256];
        swprintf_s(errorMsg, _countof(errorMsg), TEXT("Seliware Auto-Initialize failed: %s"), e.ErrorMessage());
        MessageBox(hwndParent, errorMsg, TEXT("Seliware Error"), MB_ICONERROR);
        seliwareSuccessfullyInitialized = false;
    }
}

void SeliwareInject(HWND hwndParent) { 
    if (!seliwareComPtr) {
        MessageBox(hwndParent, TEXT("Seliware COM object not available."), TEXT("Error"), MB_ICONERROR);
        return;
    }
    if (!seliwareSuccessfullyInitialized) {
        MessageBox(hwndParent, TEXT("Seliware not initialized. Cannot inject."), TEXT("Seliware Error"), MB_ICONWARNING);
        return;
    }
    try {
        _bstr_t result = seliwareComPtr->Inject();
        std::string resultStr = (char*)result;

        if (resultStr != "Success") {
            TCHAR msg[256];
            swprintf_s(msg, _countof(msg), TEXT("Seliware Inject result: %S"), resultStr.c_str());
            MessageBox(hwndParent, msg, TEXT("Seliware Info"), MB_ICONINFORMATION); 
        }
  
    }
    catch (_com_error& e) {
        TCHAR errorMsg[256];
        swprintf_s(errorMsg, _countof(errorMsg), TEXT("Seliware Inject failed: %s"), e.ErrorMessage());
        MessageBox(hwndParent, errorMsg, TEXT("Seliware Error"), MB_ICONERROR);
    }
}

void SeliwareExecute(HWND hwndParent) { 
    if (!seliwareComPtr) {
        MessageBox(hwndParent, TEXT("Seliware COM object not available."), TEXT("Error"), MB_ICONERROR);
        return;
    }
    if (!seliwareSuccessfullyInitialized) {
        MessageBox(hwndParent, TEXT("Seliware not initialized. Cannot execute."), TEXT("Seliware Error"), MB_ICONWARNING);
        return;
    }
    if (currentTab < 0 || currentTab >= tabs.size() || !tabs[currentTab].hEdit) {

        return;
    }

    try {
        if (seliwareComPtr->IsInjected() == VARIANT_FALSE) {
            MessageBox(hwndParent, TEXT("Seliware is not injected. Please inject first."), TEXT("Seliware"), MB_ICONWARNING);
            return;
        }

        int len = GetWindowTextLength(tabs[currentTab].hEdit);
        if (len == 0) {
      
            return;
        }
        char* scriptAnsi = new char[len + 1];
        GetWindowTextA(tabs[currentTab].hEdit, scriptAnsi, len + 1);

        _bstr_t scriptBstr(scriptAnsi);
        seliwareComPtr->Execute(scriptBstr);
        delete[] scriptAnsi;
      
    }
    catch (_com_error& e) {
        TCHAR errorMsg[256];
        swprintf_s(errorMsg, _countof(errorMsg), TEXT("Seliware Execute failed: %s"), e.ErrorMessage());
        MessageBox(hwndParent, errorMsg, TEXT("Seliware Error"), MB_ICONERROR);
    }
}

// --- Window Procedure ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
    {
        hInst = ((LPCREATESTRUCT)lParam)->hInstance;

        if (FAILED(CoInitialize(NULL))) {
            MessageBox(hwnd, TEXT("COM Initialization Failed! This application cannot run."), TEXT("Critical Error"), MB_ICONERROR | MB_OK);
            return -1;
        }

        try {
            seliwareComPtr.CreateInstance(__uuidof(SeliwareWrapper));
            if (!seliwareComPtr) {
                MessageBox(hwnd, TEXT("Failed to create Seliware COM object. Seliware features will be unavailable."), TEXT("COM Error"), MB_ICONERROR);
            }
            else {
                SeliwareAutoInitialize(hwnd); 
            }
        }
        catch (_com_error& e) {
            TCHAR errorMsg[256];
            swprintf_s(errorMsg, _countof(errorMsg), TEXT("Seliware CreateInstance failed: %s. Seliware features will be unavailable."), e.ErrorMessage());
            MessageBox(hwnd, errorMsg, TEXT("COM Error"), MB_ICONERROR);
        }

        InitCommonControls();

        hTab = CreateWindow(WC_TABCONTROL, TEXT(""),
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            5, 5, 500, 30, hwnd, (HMENU)ID_TAB_CONTROL, hInst, NULL);

        hTabAdd = CreateWindow(TEXT("BUTTON"), TEXT("+"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            510, 5, 25, 30, hwnd, (HMENU)ID_BTN_TAB_ADD, hInst, NULL);
        hTabClose = CreateWindow(TEXT("BUTTON"), TEXT("×"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            540, 5, 25, 30, hwnd, (HMENU)ID_BTN_TAB_CLOSE, hInst, NULL);

        int bottomButtonY = 500;
        hBtnOpen = CreateWindow(TEXT("BUTTON"), TEXT("Open"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            5, bottomButtonY, 60, 25, hwnd, (HMENU)ID_BTN_OPEN, hInst, NULL); 
        hBtnSave = CreateWindow(TEXT("BUTTON"), TEXT("Save"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            70, bottomButtonY, 60, 25, hwnd, (HMENU)ID_BTN_SAVE, hInst, NULL); 
        hBtnClear = CreateWindow(TEXT("BUTTON"), TEXT("Clear"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            135, bottomButtonY, 60, 25, hwnd, (HMENU)ID_BTN_CLEAR, hInst, NULL); 


        hBtnInject = CreateWindow(TEXT("BUTTON"), TEXT("Inject SW"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            200, bottomButtonY, 75, 25, hwnd, (HMENU)ID_BTN_INJECT, hInst, NULL); 
        hBtnRun = CreateWindow(TEXT("BUTTON"), TEXT("Exec SW"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            280, bottomButtonY, 75, 25, hwnd, (HMENU)ID_BTN_RUN, hInst, NULL); 

        CreateNewTab(TEXT("Untitled"));
        SwitchToTab(0);
    }
    break;

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        MoveWindow(hTab, 5, 5, width - 75, 30, TRUE);
        MoveWindow(hTabAdd, width - 65, 5, 25, 30, TRUE);
        MoveWindow(hTabClose, width - 35, 5, 25, 30, TRUE);

        int bottomButtonY = height - 35;
        int currentX = 5;
        MoveWindow(hBtnOpen, currentX, bottomButtonY, 60, 25, TRUE); currentX += 65;
        MoveWindow(hBtnSave, currentX, bottomButtonY, 60, 25, TRUE); currentX += 65;
        MoveWindow(hBtnClear, currentX, bottomButtonY, 60, 25, TRUE); currentX += 65;
        MoveWindow(hBtnInject, currentX, bottomButtonY, 75, 25, TRUE); currentX += 80;
        MoveWindow(hBtnRun, currentX, bottomButtonY, 75, 25, TRUE);

        ResizeCurrentEditor(hwnd);
    }
    break;

    case WM_NOTIFY:
    {
        LPNMHDR lpnm = (LPNMHDR)lParam;
        if (lpnm->hwndFrom == hTab && lpnm->code == TCN_SELCHANGE) {
            int iPage = TabCtrl_GetCurSel(hTab);
            if (iPage != -1) {
                SwitchToTab(iPage);
            }
        }
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_TAB_ADD:
            CreateNewTab(TEXT("Untitled"));
            break;
        case ID_BTN_TAB_CLOSE:
            if (!tabs.empty()) CloseTab(currentTab);
            break;

        case ID_BTN_OPEN:
            OpenFile();
            break;
        case ID_BTN_SAVE:
            SaveFile();
            break;
        case ID_BTN_CLEAR:
            if (currentTab != -1 && tabs[currentTab].hEdit) SetWindowText(tabs[currentTab].hEdit, TEXT(""));
            break;

        case ID_BTN_INJECT:
            SeliwareInject(hwnd);
            break;
        case ID_BTN_RUN:
            SeliwareExecute(hwnd); 
            break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        if (seliwareComPtr) {
            seliwareComPtr.Release();
            seliwareComPtr = nullptr;
        }
        CoUninitialize();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// --- WinMain ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = TEXT("SeliwareTextEditor");

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error"), MB_ICONERROR | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindow(
        wc.lpszClassName, TEXT("TinyS"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 600, 
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error"), MB_ICONERROR | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}