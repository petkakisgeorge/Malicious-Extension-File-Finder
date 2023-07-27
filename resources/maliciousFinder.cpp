#include <windows.h>
#include <sstream>
#include <commdlg.h>
#include <ShlObj.h>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <CommCtrl.h>
#include <thread>
#include <future>
#include <mutex>
#include <cstdlib>
#include <algorithm>
#include <cctype>

#define IDR_ICON_SMALL 101
#define IDR_ICON_BIG 102

/*
*   Author : George Petkakis
*   Linkedin: https://www.linkedin.com/in/george-petkakis-85a1711b3/
*   Github: https://github.com/petkakisgeorge

***     All rights reserved. Unauthorized copying, distribution, or modification of this code is strictly prohibited.    ***
***     This code is licensed under George Petkakis. Copying or using this code without proper attribution is not allowed.    ***
*/

namespace fs = std::filesystem;

// Define a vector to store PROCESS_INFORMATION for multiple processes
std::vector<PROCESS_INFORMATION> processInfos;

// Global variables to store user selections
bool newLayerSelectedChoices[5] = { true, true, true, true, false }; // All checkboxes are checked by default
bool selectedSearchForMultipleExtensions = true;
bool searchForFileName = false;
HWND hSubmitButton2;
HWND hSubmitButton3;
// Global variable to store the entered text

std::string enteredText2;
std::string enteredText3;
COLORREF backgroundColor = RGB(203, 231, 205);



// Step 10: Create radio button controls for the new layer choices
const char* newLayerChoices[] = {
    "Batch files",
    "Office Macros",
    "VBScript",
    "PowerShell Script",
    "Other"
};

// Global variable to store the selected path
std::string selectedPath = "";
std::string selectedpath_for_scan = "";

std::string TCHARToString(const TCHAR* tcharString)
{
#ifdef UNICODE
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, tcharString, -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0) {
        // Failed to get the buffer size
        return "";
    }

    std::string buffer(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, tcharString, -1, &buffer[0], bufferSize, nullptr, nullptr);
    return buffer;
#else
    return tcharString; // No conversion needed for non-Unicode builds (ANSI)
#endif
}
// Function to convert std::string to std::wstring
std::wstring StringToWString(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
    std::vector<wchar_t> buffer(size_needed + 1); // +1 for null-terminator
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &buffer[0], size_needed);
    buffer[size_needed] = L'\0';
    return std::wstring(&buffer[0]);
}


// Load the icon from the resources using the resource IDs
HICON hSmallIcon = (HICON)LoadImage(
    GetModuleHandle(NULL),
    MAKEINTRESOURCE(IDR_ICON_SMALL),
    IMAGE_ICON,
    0, 0,
    LR_DEFAULTSIZE
);

HICON hBigIcon = (HICON)LoadImage(
    GetModuleHandle(NULL),
    MAKEINTRESOURCE(IDR_ICON_BIG),
    IMAGE_ICON,
    0, 0,
    LR_DEFAULTSIZE
);


// ************************************************************ Some important functions ************************************* //

// Function to clear the console using the Windows API
void clearConsole() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    GetConsoleScreenBufferInfo(hConsole, &csbi);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire buffer with spaces
    FillConsoleOutputCharacter(hConsole, TEXT(' '), dwConSize, coordScreen, &cCharsWritten);

    // Reset the cursor position to the top-left
    SetConsoleCursorPosition(hConsole, coordScreen);
}



void disableCloseButton() {
    HWND hwndConsole = GetConsoleWindow();
    if (hwndConsole != NULL) {
        HMENU hMenu = GetSystemMenu(hwndConsole, FALSE);
        if (hMenu != NULL) {
            // Disable the close button (SC_CLOSE) in the system menu
            EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
        }
    }
}

void showConsoleWindow() {
    // Create a new console window
    AllocConsole();

    // Redirect standard input/output to the new console window
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    // Set the console window title
    SetConsoleTitleW(L"Searching Console/Window");

    // Move the console window to the center of the screen (optional)
    HWND hwndConsole = GetConsoleWindow();
    if (hwndConsole != NULL) {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int consoleWidth = 800; // Adjust this to your desired width
        int consoleHeight = 600; // Adjust this to your desired height
        int consoleX = (screenWidth - consoleWidth) / 2;
        int consoleY = (screenHeight - consoleHeight) / 2;
        SetWindowPos(hwndConsole, NULL, consoleX, consoleY, consoleWidth, consoleHeight, SWP_SHOWWINDOW);
    }

      // Disable the close button on the console window
    disableCloseButton();
}



// Check if the string is empty or contains only spaces
bool isStringEmptyOrSpaces(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char c) {
        return std::isspace(c);
    });
}

void TerminateProcessByPID(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess != NULL) {
        // Terminate the process
        if (TerminateProcess(hProcess, 0)) {
            std::cout << "Process with PID " << pid << " terminated successfully." << std::endl;
        } else {
            std::cout << "Failed to terminate process with PID " << pid << ". Error code: " << GetLastError() << std::endl;
        }

        CloseHandle(hProcess);
    } else {
        std::cout << "Failed to open process with PID " << pid << ". Error code: " << GetLastError() << std::endl;
    }
}

PROCESS_INFORMATION executeProcess(const std::wstring& wCommand) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Call CreateProcess to execute the executable with arguments
    if (CreateProcessW(NULL, const_cast<wchar_t*>(wCommand.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        // The process was successfully created
        
        return pi;
    } else {
        // Error calling the executable
        std::wcout << L"Error calling the executable: " << GetLastError() << std::endl;
        // You can return a default-initialized PROCESS_INFORMATION or throw an exception here
        // depending on how you want to handle the error.
        return PROCESS_INFORMATION{};
    }
}

bool isProcessRunning(const PROCESS_INFORMATION& pi) {
    DWORD exitCode;
    if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
        //std::cout << exitCode << std::endl;
        return exitCode == STILL_ACTIVE;
    }
    // Failed to get the exit code, treat as not running    
    return false;
}

void StopSearching(){
// Terminate all processes in processInfos vector
    for (const auto& pi : processInfos) {
        TerminateProcessByPID(pi.dwProcessId);
    }    
    // Process has finished, proceed with the rest of your code
    EnableWindow(hSubmitButton2, TRUE); 
    // Clear the cmd window
    clearConsole();
    MessageBox(NULL, "All Searches were Stopped!", "STOP SUCCEED", MB_OK);
}

void checkProcessStatus(const PROCESS_INFORMATION& pi) {
  //  std::cout << "entered" << std::endl;
    while (isProcessRunning(pi)) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep for 1 second
    }

    // Process has finished, proceed with the rest of your code
    EnableWindow(hSubmitButton2, TRUE); 
    // Clear the cmd window
    clearConsole();
}


void starting_the_Search(){

    showConsoleWindow();
    // Call your findFiles functions and update the progress bar accordingly
    // For example, you can update the progress bar in findFiles function like this:
    int totalSearches = 2; // Update this with the total number of searches you have
    int currentSearch = 0;    
    std::string rootPath = ""; 

     // Variables to store user-selected data (replace these with your actual user input)
    std::string selectedPath_Final = selectedPath;    
    std::string selectedPathForScan_Final = selectedpath_for_scan;
    bool category1 = newLayerSelectedChoices[0];
    bool category2 = newLayerSelectedChoices[1];
    bool category3 = newLayerSelectedChoices[2];
    bool category4 = newLayerSelectedChoices[3];
    bool category5 = newLayerSelectedChoices[4];
    bool search_Custom = searchForFileName;
    bool search_Multiple = selectedSearchForMultipleExtensions;
    std::string extraExtension = enteredText2;
    std::string extraFileName = enteredText3;


// Construct the command with arguments
    std::string command = "resources/search console.exe";
    command += " " + selectedPath_Final ;
    command += " " + selectedPathForScan_Final ;
    command += " " + std::to_string(static_cast<int>(category1)) ;
    command += " " + std::to_string(static_cast<int>(category2)) ;
    command += " " + std::to_string(static_cast<int>(category3)) ;
    command += " " + std::to_string(static_cast<int>(category4)) ;
    command += " " + std::to_string(static_cast<int>(category5)) ;
    command += " " + std::to_string(static_cast<int>(search_Custom)) ;
    command += " " + std::to_string(static_cast<int>(search_Multiple)) ;
    if (isStringEmptyOrSpaces( extraExtension)) {
        command += " empty";
    }
    else{
        command += " " + extraExtension ;
    }
    if (isStringEmptyOrSpaces( extraFileName)) {
        command += " empty";
    }
    else{
        command += " " + extraFileName ;
    }
      
    // Convert the command to wide string (required by CreateProcess)
    std::wstring wCommand = std::wstring(command.begin(), command.end());
    // Enable the button
    EnableWindow(hSubmitButton2, FALSE); 
    // Asynchronously execute the process and store the returned PROCESS_INFORMATION
    std::future<PROCESS_INFORMATION> future = std::async(std::launch::async, executeProcess, wCommand);

    // Wait for the process to finish and get the PROCESS_INFORMATION
    PROCESS_INFORMATION pi = future.get();
    processInfos.push_back(pi);
    
    // Start the process checking loop in a new thread
    std::thread thread(checkProcessStatus, pi);

    // Detach the thread, so it runs independently from the main thread
    thread.detach();
}


// Step 1: Declare the Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{    
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED)
        {
            if (LOWORD(wParam) == 278) // Check if the submit button for radio buttons is clicked
            {                             
                int numSelected = 0;            
                for (int i = 0; i < 5; i++)
                {
                    if (newLayerSelectedChoices[i])
                    {
                        numSelected++;                       
                    }
                }
                                   
            // ************************ INPUT - FILE EXTENSIONS *********************************** //    
                 // Get the length of the entered text
                int length2 = GetWindowTextLength(GetDlgItem(hwnd, 597));
                
                // Allocate buffer to store the entered text
                char* buffer2 = new char[length2 + 1];
                
                // Get the text from the input box and store it in the buffer
                GetDlgItemText(hwnd, 597, buffer2, length2 + 1);

                // Store the entered text in the global variable
                enteredText2 = buffer2;

                // Clean up the buffer
                delete[] buffer2;

                // Display the entered text in a message box
                          

             // ************************ INPUT - FILE EXTENSIONS *********************************** //    
                 // Get the length of the entered text
                int length3 = GetWindowTextLength(GetDlgItem(hwnd, 607));
                
                // Allocate buffer to store the entered text
                char* buffer3 = new char[length3 + 1];
                
                // Get the text from the input box and store it in the buffer
                GetDlgItemText(hwnd, 607, buffer3, length3 + 1);

                // Store the entered text in the global variable
                enteredText3 = buffer3;

                // Clean up the buffer
                delete[] buffer3;


            // ************************ Check if at least on option is selected *************************************** //                                         
                if (numSelected == 0 && !selectedSearchForMultipleExtensions && !searchForFileName)
                {
                    MessageBox(hwnd, "Please select at least one option!", "Error", MB_OK | MB_ICONERROR);
                }else{     
                    starting_the_Search();
                }
            }
            else if (LOWORD(wParam) == 213){
            BROWSEINFO browseInfo = { 0 };
            browseInfo.hwndOwner = hwnd;
            browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
            LPITEMIDLIST itemIdList = SHBrowseForFolder(&browseInfo);

            if (itemIdList != NULL)
            {
                TCHAR folderPath[MAX_PATH];
                if (SHGetPathFromIDList(itemIdList, folderPath))
                {
                    // Set the selected folder path in the input box
                    SetWindowText(GetDlgItem(hwnd, 121), folderPath);
                    selectedPath = TCHARToString(folderPath);
                           
                }

                // Free the memory allocated by SHBrowseForFolder
                CoTaskMemFree(itemIdList);
            }
            }
            else if (LOWORD(wParam) == 233){
            BROWSEINFO browseInfo = { 0 };
            browseInfo.hwndOwner = hwnd;
            browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
            LPITEMIDLIST itemIdList = SHBrowseForFolder(&browseInfo);

            if (itemIdList != NULL)
            {
                TCHAR folderPath[MAX_PATH];
                if (SHGetPathFromIDList(itemIdList, folderPath))
                {
                    // Set the selected folder path in the input box
                    SetWindowText(GetDlgItem(hwnd, 129), folderPath);
                    selectedpath_for_scan = TCHARToString(folderPath);
                           
                }

                // Free the memory allocated by SHBrowseForFolder
                CoTaskMemFree(itemIdList);
            }
            }
            else if (LOWORD(wParam) == 60){
                if (selectedSearchForMultipleExtensions){
                    selectedSearchForMultipleExtensions = false;
                }
                else{
                    selectedSearchForMultipleExtensions= true;
                }
              
            }
            else if (LOWORD(wParam) == 80){
                if (searchForFileName){
                    searchForFileName = false;
                }
                else{
                    searchForFileName = true;
                }
              
            }
            else if (LOWORD(wParam) == 787){
                StopSearching();
            }
            else // Check if a radio button is clicked
            {               
                // Check if a checkbox is clicked
                for (int i = 0; i < 5; i++)
                {
                    newLayerSelectedChoices[i] = (SendMessage(GetDlgItem(hwnd, i + 6), BM_GETCHECK, 0, 0) == BST_CHECKED);
                }
            }
        }
    
    case WM_PAINT:
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc;
    GetClientRect(hwnd, &rc);

    // Fill the client area with the background color
    HBRUSH hBrush = CreateSolidBrush(backgroundColor);
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    // Draw the line using the GDI function
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(93, 135, 185)); // Red color, 2-pixel width
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    POINT lineStart = { 15, 20 }; // Modify these coordinates as needed
    POINT lineEnd = { 785, 20 }; // Modify these coordinates as needed
    MoveToEx(hdc, lineStart.x, lineStart.y, NULL);
    LineTo(hdc, lineEnd.x, lineEnd.y);

    POINT lineStart2 =  { 15, 20 }; 
    POINT lineEnd2 =  { 15, 335 };
    MoveToEx(hdc, lineStart2.x, lineStart2.y, NULL);
    LineTo(hdc, lineEnd2.x, lineEnd2.y);

    POINT lineStart3 = { 785, 20 };
    POINT lineEnd3 = { 785, 335 };
    MoveToEx(hdc, lineStart3.x, lineStart3.y, NULL);
    LineTo(hdc, lineEnd3.x, lineEnd3.y);

    POINT lineStart4 = { 15, 335 };
    POINT lineEnd4 = { 785, 335 };
    MoveToEx(hdc, lineStart4.x, lineStart4.y, NULL);
    LineTo(hdc, lineEnd4.x, lineEnd4.y);

    POINT lineStart5 = { 400, 20 };
    POINT lineEnd5 = { 400, 335 };
    MoveToEx(hdc, lineStart5.x, lineStart5.y, NULL);
    LineTo(hdc, lineEnd5.x, lineEnd5.y);

    POINT lineStart6 = { 15, 170 };
    POINT lineEnd6 = { 785, 170 };
    MoveToEx(hdc, lineStart6.x, lineStart6.y, NULL);
    LineTo(hdc, lineEnd6.x, lineEnd6.y);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    EndPaint(hwnd, &ps);
    return 0;
}

    case WM_DRAWITEM:
{
    DRAWITEMSTRUCT* pDrawItem = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
    int controlId = GetDlgCtrlID(pDrawItem->hwndItem);

    if (controlId == 787)
    {
        HDC hdc = pDrawItem->hDC;
        RECT rc = pDrawItem->rcItem;

        // Draw the button based on the button state
        if (pDrawItem->itemState & ODS_SELECTED)
        {
            // Draw the button background in a different color when hovered
            HBRUSH hHoverBrush = CreateSolidBrush(RGB(234, 81, 81));
            FillRect(hdc, &rc, hHoverBrush);
            DeleteObject(hHoverBrush);

            // Draw the button text in white
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, "STOP", -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }        
        else
        {
            // Draw the button background in red when not pressed or hovered
            HBRUSH hRedBrush = CreateSolidBrush(RGB(166, 14, 14));
            FillRect(hdc, &rc, hRedBrush);
            DeleteObject(hRedBrush);

            // Draw the button text in white
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, "STOP", -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }

        return TRUE;
    }else if (controlId == 278)
    {
        HDC hdc = pDrawItem->hDC;
        RECT rc = pDrawItem->rcItem;

        // Draw the button based on the button state
        if (pDrawItem->itemState & ODS_SELECTED)
        {
            // Draw the button background in a different color when hovered
            HBRUSH hHoverBrush = CreateSolidBrush(RGB(34, 165, 82));
            FillRect(hdc, &rc, hHoverBrush);
            DeleteObject(hHoverBrush);

            // Draw the button text in white
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, "START SEARCHING", -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        } else if (pDrawItem->itemState & ODS_DISABLED)
        {
            // Draw the button background in a different color when hovered
            HBRUSH hHoverBrush = CreateSolidBrush(RGB(112, 225, 153));
            FillRect(hdc, &rc, hHoverBrush);
            DeleteObject(hHoverBrush);

            // Draw the button text in white
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, "START SEARCHING", -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }        
        else
        {
            // Draw the button background in red when not pressed or hovered
            HBRUSH hRedBrush = CreateSolidBrush(RGB(13, 131, 56));
            FillRect(hdc, &rc, hRedBrush);
            DeleteObject(hRedBrush);

            // Draw the button text in white
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, "START SEARCHING", -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }

        return TRUE;
    }
    
    break;
}

// Handle the WM_CLOSE message
    case WM_CLOSE:{
                 // Terminate all processes in processInfos vector
                 for (const auto& pi : processInfos) {
                      TerminateProcessByPID(pi.dwProcessId);
                 }

                // Close the main window
                DestroyWindow(hwnd);
                 return 0;
    }
        return 0;
        
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main()
{


// ****************************************************** Creation of the Window ********************************************//    
        // Step 2: Register the Window Class
    const char CLASS_NAME[] = "MyWindowClass";
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

     // Step 3: Create the Window
    int windowWidth = 800;
    int windowHeight = 500;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int windowX = (screenWidth - windowWidth) / 2;
    int windowY = (screenHeight - windowHeight) / 2;

    HWND hwnd = CreateWindowEx(
        0,                  // Optional window styles.
        CLASS_NAME,         // Window class
        "Malicious File/Extension Finder",    // Window text
        WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX), // Window style
        windowX, windowY,  // Window position
        windowWidth, windowHeight, // Size
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );


// Step 2: Set the icons for the window
SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hSmallIcon);   // Set small icon (taskbar icon)
SendMessage(hwnd, WM_SETICON, ICON_SMALL2, (LPARAM)hSmallIcon); // Set small icon (taskbar icon)
SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hBigIcon);      // Set big icon (alt-tab icon)


    // After the window is created (inside the main function), set the brush as the background
   // SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBackgroundBrush);

    if (hwnd == NULL)
    {
        return 0;
    }

    HWND hwndConsole = GetConsoleWindow();
    if (hwndConsole != NULL) {
        // Get the dimensions of the screen
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // Get the dimensions of the console window
        RECT consoleRect;
        GetWindowRect(hwndConsole, &consoleRect);
        int consoleWidth = consoleRect.right - consoleRect.left;
        int consoleHeight = consoleRect.bottom - consoleRect.top;

        // Calculate the position to center the console window
        int consoleX = (screenWidth - consoleWidth) / 2;
        int consoleY = (screenHeight - consoleHeight) / 2;

        // Move the console window to the center of the screen
        SetWindowPos(hwndConsole, NULL, consoleX, consoleY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

// ******************************************** File Path - For Scan-Search ***************************************************** //

    // Step 4: Create static text control for the question
    HWND hQuestionText = CreateWindow(
        "STATIC",                   // Predefined class; Unicode assumed
        "Select the path to be searched:",      // Question text
        WS_VISIBLE | WS_CHILD,      // Styles
        30, 40, 350, 30,            // x, y, width, height
        hwnd,                       // Parent window
        NULL,                       // No menu
        hInstance,                 // Instance handle
        NULL                        // Additional application data
    );
     // Step 6: Create the "Browse" button
    HWND hBrowseButton2 = CreateWindow(
        "BUTTON",
        "Browse",
         WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        145, 125, 100, 25, 
        hwnd, 
        (HMENU)233, 
        hInstance, 
        NULL
    );// 680 -60 620
    // Step 7: Create the input method (edit control)
    HWND hInputMethod6 = CreateWindow(  
        "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY,
        30, 80, 350, 25, hwnd, (HMENU)129, hInstance, NULL
    );
    std::string defaultPath = "C:\\";
    SetWindowTextW(GetDlgItem(hwnd, 129), StringToWString(defaultPath).c_str());
    selectedpath_for_scan = defaultPath;



// ******************************************* Check Boxes - File Extensions  *****************************************************//
    // Step 4: Create static text control for the question
    HWND hQuestionText2 = CreateWindow(
        "STATIC",                   // Predefined class; Unicode assumed
        "Choose the file extensions:",      // Question text
        WS_VISIBLE | WS_CHILD,      // Styles
        30, 180, 350, 30,            // x, y, width, height
        hwnd,                       // Parent window
        NULL,                       // No menu
        hInstance,                  // Instance handle
        NULL                        // Additional application data
    );
   
    // Step 11: Create checkboxes for the new layer choices
    for (int i = 0; i < 4; i++) {
         HWND hCheckBox = CreateWindow(
            "BUTTON",                   // Predefined class; Unicode assumed
            newLayerChoices[i],         // Choice text
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, // Use BS_AUTOCHECKBOX style for checkboxes
            30, 220 + i * 25, 150, 20,  // x, y, width, height
            hwnd,                       // Parent window
            reinterpret_cast<HMENU>(i + 6), // Use reinterpret_cast for HMENU cast
            GetModuleHandle(NULL),      // Instance handle (instead of hInstance)
            NULL                        // Additional application data
        );
        // Check all checkboxes by default
        SendMessage(hCheckBox, BM_SETCHECK, BST_CHECKED, 0);
    }
     HWND hCheckBox2 = CreateWindow(
            "BUTTON",                   // Predefined class; Unicode assumed
            newLayerChoices[4],         // Choice text
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, // Use BS_AUTOCHECKBOX style for checkboxes
            200, 220 , 180, 20,  // x, y, width, height
            hwnd,                       // Parent window
            reinterpret_cast<HMENU>(4 + 6), // Use reinterpret_cast for HMENU cast
            GetModuleHandle(NULL),      // Instance handle (instead of hInstance)
            NULL                        // Additional application data
        );


// **************************************** Checkbox - File with multiple extensions ******************************************** //
// Step 4: Create static text control for the question
    HWND hQuestionText3 = CreateWindow(
        "STATIC",                   // Predefined class; Unicode assumed
        "Files with multiple extensions:",      // Question text
        WS_VISIBLE | WS_CHILD,      // Styles
        450, 40, 320, 30,            // x, y, width, height
        hwnd,                       // Parent window
        NULL,                       // No menu
        hInstance,                 // Instance handle
        NULL                        // Additional application data
    );
    HWND hCheckBox3 = CreateWindow(
            "BUTTON",                   // Predefined class; Unicode assumed
            "Search for malicious double extensions",         // Choice text
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, // Use BS_AUTOCHECKBOX style for checkboxes
            450, 80, 300, 20,  // x, y, width, height
            hwnd,                      // Parent window
            reinterpret_cast<HMENU>(60), // Use reinterpret_cast for HMENU cast
            GetModuleHandle(NULL),      // Instance handle (instead of hInstance)
            NULL                        // Additional application data
        );
        // To check the checkbox
    SendMessage(hCheckBox3, BM_SETCHECK, BST_CHECKED, 0);

// *********************************** Checkbox - Check for a specific file name ******************************************** //

    HWND hQuestionText4 = CreateWindow(
        "STATIC",                   // Predefined class; Unicode assumed
        "User's Custom Search:",      // Question text
        WS_VISIBLE | WS_CHILD,      // Styles
        450, 180, 320, 30,            // x, y, width, height
        hwnd,                       // Parent window
        NULL,                       // No menu
        hInstance,                 // Instance handle
        NULL                        // Additional application data
    );
      HWND hCheckBox4 = CreateWindow(
            "BUTTON",                   // Predefined class; Unicode assumed
            "Search for a specific file name",         // Choice text
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, // Use BS_AUTOCHECKBOX style for checkboxes
            450, 220, 300, 20,  // x, y, width, height
            hwnd,                       // Parent window
            reinterpret_cast<HMENU>(80), // Use reinterpret_cast for HMENU cast
            GetModuleHandle(NULL),      // Instance handle (instead of hInstance)
            NULL                        // Additional application data
        ); 
//  ****************************************** INPUT BOX ************************************** // 

    HWND hInputBox2 = CreateWindow(  // INPUT - FILE EXTENSIONS
        "EDIT",                         // Predefined class; Unicode assumed
        "",                             // No initial text in the input box
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, // Styles
        230, 250, 110, 35,               // x, y, width, height
        hwnd,                           // Parent window
        (HMENU)597,                       // Control ID (used in WM_COMMAND message)
        hInstance,                      // Instance handle
        NULL                            // Additional application data
    );
     HWND hInputBox3 = CreateWindow(  // INPUT - CUSTOM SEARCH
        "EDIT",                         // Predefined class; Unicode assumed
        "",                             // No initial text in the input box
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, // Styles
        480, 250, 240, 35,               // x, y, width, height
        hwnd,                           // Parent window
        (HMENU)607,                       // Control ID (used in WM_COMMAND message)
        hInstance,                      // Instance handle
        NULL                            // Additional application data
    );
    

// ***************************************************** Final Actions **************************************** //

    HWND hQuestionText5 = CreateWindow(
        "STATIC",                   // Predefined class; Unicode assumed
        "Select a path to store the Results:",      // Question text
        WS_VISIBLE | WS_CHILD,      // Styles
        250, 340, 410, 25, //460           // x, y, width, height
        hwnd,                       // Parent window
        NULL,                       // No menu
        hInstance,                 // Instance handle
        NULL                        // Additional application data
    );

     // Step 12: Create the submit button for checkboxes
    hSubmitButton2 = CreateWindow(
        "BUTTON",                   // Predefined class; Unicode assumed
        "START SEARCHING",      // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,  // Styles
        60, 410, 560, 50,           // x, y, width, height
        hwnd,                       // Parent window
        (HMENU)278,                   // Button ID (used in WM_COMMAND message)
        hInstance,                  // Instance handle
        NULL                        // Additional application data
    );


HWND hSubmitButton3 = CreateWindow(
    "BUTTON",                   // Predefined class; Unicode assumed
    "STOP",                     // Button text
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, // Styles with BS_OWNERDRAW
     640, 410, 100, 50,                // Position and size
    hwnd,                       // Parent window
    (HMENU)787,                 // Button ID (used in WM_COMMAND message)
    hInstance,                  // Instance handle
    NULL                                         // Additional application data
);




//SendMessage(hSubmitButton3, WM_CTLCOLORBTN, BS_PUSHBUTTON, TRUE);

    // Step 6: Create the "Browse" button
    HWND hBrowseButton = CreateWindow(
        "BUTTON",
        "Browse",
         WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        110, 370, 100, 25, 
        hwnd, 
        (HMENU)213, 
        hInstance, 
        NULL
    );// 680 -60 620
    
   
    // Step 7: Create the input method (edit control)
HWND hInputMethod5 = CreateWindow(  
    "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY,
    230, 370, 460, 25, hwnd, (HMENU)121, hInstance, NULL
);

    std::string desktopPath = std::getenv("USERPROFILE");
    desktopPath += "\\Desktop\\";

    // Convert desktopPath to wide string

    // Pre-set the initial folder path
  
     SetWindowTextW(GetDlgItem(hwnd, 121), StringToWString(desktopPath).c_str());
    selectedPath = desktopPath;  

    // Step 7: Show the Window
    ShowWindow(hwnd, SW_SHOWNORMAL);

    // Step 8: Run the Message Loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}