// 

#include <windows.h>
#include <cstdio>

// Forward declarations just so I can order these however.

HWND InitWindow(HINSTANCE);

LRESULT CALLBACK WinMessageCallback(HWND, UINT, WPARAM, LPARAM);

void LogMessage(const char* message);

static bool gIsRunning;



int CALLBACK WinMain(HINSTANCE instance,
                     HINSTANCE prevInstance,
                     LPSTR     cmdLine,
                     int       cmdShow)
{   
    HWND hwnd = InitWindow(instance);

    if (!hwnd) 
    {
        LogMessage("Failed to create window.");
        return -1;
    }   

    ShowWindow(hwnd, cmdShow);
    UpdateWindow(hwnd);

    HDC hdc = GetDC(hwnd);
    MSG message;
    while (gIsRunning)
    {
        // This is a blocking call, consider exploring PeekMessage.
        GetMessage(&message, NULL, 0, 0);

        TranslateMessage(&message);
        DispatchMessage(&message);

        SwapBuffers(hdc);
    }   

    return (int)message.wParam;
}

/*
 * I am compiling and building this as a console application so I can write out to the command
 * prompt without having to allocate a new window and stuff. This means normal main can just 
 * wrap WinMain and no one is the wiser.
*/
int main(int argc, char** argv)
{
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}


LRESULT CALLBACK WinMessageCallback(HWND hwnd, 
                                    UINT message, 
                                    WPARAM wParam, 
                                    LPARAM lParam)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_CREATE:
        {
            HDC hdc = GetDC(hwnd);
            LogMessage("Window created.");

            gIsRunning = true;
        }
        break;

        case WM_CLOSE:
        {
            gIsRunning = false;
            PostQuitMessage(0);         
        }
        break;

        case WM_DESTROY:
        {
            gIsRunning = false;
            PostQuitMessage(0);
        }
        break;

        default:
            result = DefWindowProc(hwnd, message, wParam, lParam);
        break;
    }

    return result;
}


HWND InitWindow(HINSTANCE instance)
{
    WNDCLASS wndClass = {};
    wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WinMessageCallback;
    wndClass.hInstance = instance;
    wndClass.lpszClassName = "SACWeather";
    wndClass.hIcon = 0;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);


    if (!RegisterClass(&wndClass)) 
    {
        LogMessage("Failed to register window class.");
        return 0;
    }    

    HWND hwnd = CreateWindowEx(
            0,
            wndClass.lpszClassName,
            "SAC Weather",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,       // x, y
            1000, 1000,                         // w, h
            //CW_USEDEFAULT, CW_USEDEFAULT, 
            0,
            0,
            instance,
            0
    );

    if (!hwnd) 
    {
        LogMessage("Error creating window");
        return 0;
    }

    return hwnd;
}

void ShowError(const char* message)
{
    printf("%s\n", message);
}


void LogMessage(const char* message)
{
    printf("%s\n", message);
}