#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <iostream>
#include <iostream>
#include <random>
#include <fstream>
#include <deque>
#include <queue>
#include <set>
#include <iterator>
// #include <thread>
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

std::vector<std::string> loadFile(std::string fileName) {
    std::ifstream file(fileName);
    // std::assert(file);

    std::vector<std::string> fileContents;
    std::copy(std::istream_iterator<std::string>(file), 
              std::istream_iterator<std::string>(), 
              std::back_inserter(fileContents));

    return fileContents;
}
void OnPaint(HWND hWnd);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"!! win32 app",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (hwnd == NULL)
    {
        return 0;
    }
    ;
    // set a timer for every 16 millisecond
    SetTimer(hwnd, 100, 16, NULL);
    ShowWindow(hwnd, nCmdShow);
    DragAcceptFiles(hwnd, true);
  
    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
long long cnt = 0;
void OnPaint(HWND hWnd)
{
    RECT  rect;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    FillRect(hdc, &ps.rcPaint, (HBRUSH) CreateSolidBrush(RGB(0, 0, 0)));
    GetClientRect(hWnd, &rect);
    SetTextColor(hdc, RGB(0xF4, 0xAE, 0x72));
    SetBkMode(hdc, TRANSPARENT);
    rect.left = 40 ;//(cnt % 99);
    // std::cerr << rect.left << std::endl;
    rect.top = 100 + cnt;
    DrawText(hdc, L"Hello World!", -1, &rect, DT_SINGLELINE | DT_NOCLIP);
    // SelectObject(hdc, oldPen);
    // DeleteObject(hPen);
    EndPaint(hWnd, &ps);
    ++cnt;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TIMER:
    if (wParam == 100 )
    {
        InvalidateRect(hwnd, NULL, FALSE); // update whole window every 100ms
    }
    break;
    case WM_DROPFILES:
        // auto size = DragQueryFileA( (HDROP)wParam, 0, NULL, 30);
        // std::thread t(
        { 
            CHAR out[100];
            DragQueryFileA( (HDROP)wParam, 0, out, 100);
            std::cerr << out << std::endl;
            WCHAR soundfile[100];
            mbstowcs(soundfile, out, strlen(out)+1);
            PlaySound(soundfile, NULL, SND_ASYNC);
            // playsound = true;
            
        }
        return 0;
    case WM_KEYDOWN:
        if(wParam == VK_ESCAPE) {
            PostQuitMessage(0);
          
        } 
        return 0;
        
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            OnPaint(hwnd);
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}