// 

#include <windows.h>
#include <cstdio>
#include "tjd_ftp.h"
#include "gl.h"
#include "sacw_api.h"

// Forward declarations just so I can order these however.

HWND InitWindow(HINSTANCE);
LRESULT CALLBACK WinMessageCallback(HWND, UINT, WPARAM, LPARAM);
void LogMessage(const char* message);
//int DownloadFile(const char* url, const char* dest);
void PanMap(f32 x, f32 y);
static LRESULT Win32InitOpenGL(HDC hdc);


static bool gIsRunning;

static bool  Panning = false;
static v2f32 ClickLast = {};

static s32 KEY_PANNING = VK_LBUTTON;


static char* CmdLineArgs;


int CALLBACK WinMain(HINSTANCE instance,
                     HINSTANCE prevInstance,
                     LPSTR     cmdLine,
                     int       cmdShow)
{   
    CmdLineArgs = strtok(cmdLine, " ");    
    CmdLineArgs = strtok(NULL, " ");

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


        // 
        sacw_MainLoop();


        SwapBuffers(hdc);
    }   

    // if this turns out to be needed, it may need to be moved elsewhere @todo
    sacw_Cleanup();

    return (int)message.wParam;
}


void PanMap(f32 x, f32 y)
{
    f32 dx, dy;

    dx = (ClickLast.x - x) * -0.001f;
    dy = (ClickLast.y - y) *  0.001f;

    sacw_PanMap(dx, dy);

    ClickLast.x = x;
    ClickLast.y = y;
}


/*
 * I am compiling and building this as a console application so I can write out to the command
 * prompt without having to allocate a new window and stuff. This means normal main can just 
 * wrap WinMain and no one is the wiser.
*/
int main(int argc, char** argv)
{
    int result = 0;

    // if (argc > 1) 
    // {
    //     printf("Test...\n");
    //     int errCode = DownloadFile();
    //     printf("Error code: %d", errCode);
    // } 
    // else 
    // {
        
    // }

    result = WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);

    return result;
}


void HandleKeyDown(unsigned int keyCode)
{
    printf("Key code: %d\n", keyCode);
    if (keyCode == KEY_PANNING) Panning = true;
}


void HandleKeyUp(unsigned int keyCode)
{
    if (keyCode == KEY_PANNING) Panning = false;
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

            // initialize opengl for windows
            result = Win32InitOpenGL(hdc);
            if (result == 0)
            {
                sacw_Init(CmdLineArgs);
            }

            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            sacw_UpdateViewport(cs->cx, cs->cy);

            gIsRunning = true;
        }
        break;

        case WM_SIZE:
        {
            f32 width = (f32)LOWORD(lParam);
            f32 height = (f32)HIWORD(lParam);
            sacw_UpdateViewport(width, height);
        }
        break;

        case WM_MOUSEWHEEL:
        {
            f32 delta = GET_WHEEL_DELTA_WPARAM(wParam) * 0.01f; // @todo
            sacw_ZoomMap(delta);
        }
        break;

        case WM_LBUTTONDOWN:
        {
            ClickLast.x = (lParam & 0xffff);
            ClickLast.y = (lParam & 0xffff0000) >> 16;
    
            HandleKeyDown(VK_LBUTTON);
        }
        break;

        case WM_LBUTTONUP:
        {
            HandleKeyUp(VK_LBUTTON);
        }
        break;
        
        case WM_KEYDOWN:
        {
            HandleKeyDown(wParam);
        }
        break;

        case WM_KEYUP:
        {
            HandleKeyUp(wParam);
        }
        break;
        
        case WM_MOUSEMOVE:
        {
            if (Panning)
            {
                PanMap(lParam & 0xffff, (lParam & 0xffff0000) >> 16);
            }
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
            //1000, 1000,                         // w, h
            CW_USEDEFAULT, CW_USEDEFAULT, 
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


/*int DownloadFile(const char* url, const char* dest)
{
    // @todo
    // Either implement the callback or make this the fallback in the case that the
    // eventual FTP or HTTPs implementation doesn't work or whatever
    int result = URLDownloadToFile(NULL, url, dest, 0, NULL);
    return result;
}*/



static LRESULT Win32InitOpenGL(HDC hdc) {
    bool success = false;
    PIXELFORMATDESCRIPTOR reqPfd = {};
    reqPfd.nSize = sizeof(reqPfd);
    reqPfd.nVersion = 1;
    reqPfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    reqPfd.iPixelType = PFD_TYPE_RGBA;
    reqPfd.cColorBits = 32;
    reqPfd.cAlphaBits = 8;
    reqPfd.cDepthBits = 24;

    int reqPfdId = ChoosePixelFormat(hdc, &reqPfd);
    if (!reqPfdId) 
    {
        LogMessage("No suitable pixel format found.");
        return -1;
    }

    PIXELFORMATDESCRIPTOR pfd = {};
    DescribePixelFormat(hdc, reqPfdId, sizeof(pfd), &pfd);

    success = SetPixelFormat(hdc, reqPfdId, &pfd);
    if (!success) 
    {
        LogMessage("Failed to set pixel format.");
        return -2;
    }

    HGLRC glRC = wglCreateContext(hdc);
    if (!glRC) 
    {
        LogMessage("Failed to create OpenGL context.");
        return -3;
    }

    // todo: this may be redundant at the moment since we are requesting a sepcific
    //       openGL feature set right after this. This might be useful as a fall back?
    success = wglMakeCurrent(hdc, glRC);
    if (!success) 
    {
        LogMessage("Failed to wglMakeCurrent.");
        return -4;
    }


    // request context attributes so we can get a specific OpenGL version/feature set
    GLint reqAttributes[] = 
    {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 1,
            WGL_CONTEXT_PROFILE_MASK_ARB,
            WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
    };

    // todo: verify this somehow
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
            wglGetProcAddress("wglCreateContextAttribsARB");

    HGLRC newGLRC = wglCreateContextAttribsARB(hdc, 0, reqAttributes);

    if (!newGLRC) 
    {
        // todo: then in here we could fall back to a lower version or even
        //       fallback to software rendering..
        LogMessage("Failed to set new OpenGL context.");
        return -5;
    }

    wglMakeCurrent(NULL, NULL);
    wglMakeCurrent(hdc, newGLRC);

    return 0;
}
