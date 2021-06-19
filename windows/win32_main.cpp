// 
#define _WIN32_WINNT 0x06000000

#include <windows.h>
#include <wininet.h>
#include <cstdio>
#include <dwmapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <windowsx.h>
#include "tjd_gl.h"
// #include "tjd_ftp.h"
#include "tjd_ui.h"
#include "sacw_api.h"
#include "win32_menu.h"

#include "imgui/imgui_impl_win32.h"


#define LEFTEXTENDWIDTH     8
#define RIGHTEXTENDWIDTH    8
#define BOTTOMEXTENDWIDTH   20
#define TOPEXTENDWIDTH      27

// @todo
// probably move this somewhere else
class WinBindStatusCallback : public IBindStatusCallback
{
public:
    STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown* punk)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(OnStartBinding)(
        /* [in] */ DWORD dwReserved,
        /* [in] */ IBinding __RPC_FAR* pib
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(GetPriority)(
        /* [out] */ LONG __RPC_FAR* pnPriority
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(OnLowResource)(
        /* [in] */ DWORD reserved
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(OnProgress)(
        /* [in] */ ULONG ulProgress,
        /* [in] */ ULONG ulProgressMax,
        /* [in] */ ULONG ulStatusCode,
        /* [in] */ LPCWSTR wszStatusText
    );

    STDMETHOD(OnStopBinding)(
        /* [in] */ HRESULT hresult,
        /* [unique][in] */ LPCWSTR szError
    );

    STDMETHOD(GetBindInfo)(
        /* [out] */ DWORD __RPC_FAR* grfBINDF,
        /* [unique][out][in] */ BINDINFO __RPC_FAR* pbindinfo
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(OnDataAvailable)(
        /* [in] */ DWORD grfBSCF,
        /* [in] */ DWORD dwSize,
        /* [in] */ FORMATETC __RPC_FAR* pformatetc,
        /* [in] */ STGMEDIUM __RPC_FAR* pstgmed
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD_(ULONG, AddRef)()
    {
        return 0;
    }

    STDMETHOD_(ULONG, Release)()
    {
        return 0;
    }

    STDMETHOD(QueryInterface)(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR* __RPC_FAR* ppvObject
    )
    {
        return E_NOTIMPL;
    }
};

HRESULT WinBindStatusCallback::OnStopBinding(HRESULT hresult, LPCWSTR szError)
{
    const char* filename = "C:\\tmp\\testing_radar.nx3";
    sacw_RadarInit(filename, 94);
    return 0;
}

HRESULT WinBindStatusCallback::OnProgress(
    /* [in] */ ULONG ulProgress,
    /* [in] */ ULONG ulProgressMax,
    /* [in] */ ULONG ulStatusCode,
    /* [in] */ LPCWSTR wszStatusText
)
{
    printf("Progress: %d/%d\n", ulProgress, ulProgressMax);
    return 0;
}

// Forward declarations just so I can order these however.
HWND InitWindow(HINSTANCE);
LRESULT CALLBACK WinMessageCallback(HWND, UINT, WPARAM, LPARAM);
void LogMessage(const char* message);
//int DownloadFile(const char* url, const char* dest);
void PanMap(f32 x, f32 y);
static LRESULT Win32InitOpenGL(HDC hdc);

static bool gIsRunning;

static bool Panning = false;
static v2f32 ClickLast = {};

static s32 KEY_PANNING = VK_LBUTTON;

static char* CmdLineArgs;

int CALLBACK WinMain(
    HINSTANCE instance,
    HINSTANCE prevInstance,
    LPSTR cmdLine,
    int cmdShow
)
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

    const char* file_url = "https://tgftp.nws.noaa.gov/SL.us008001/DF.of/DC.radar/DS.p94r0/SI.kind/sn.last";
    const char* filename = "C:\\tmp\\testing_radar.nx3";

    bool testing_level2 = false;
    if (!testing_level2)
    {
        DeleteUrlCacheEntry(file_url);
        WinBindStatusCallback wbcb;
        URLDownloadToFile(nullptr, file_url, filename, 0, &wbcb);
    }
    else
    {
        const char* filename = "C:\\shapes\\KIND_20210526_1423";
        sacw_RadarInit(filename, 94);
    }

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

    dx = (ClickLast.x - x);
    dy = (ClickLast.y - y);

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
    // printf("Key code: %d\n", keyCode);
    if (keyCode == KEY_PANNING)
    {
        Panning = true;
        f32 points[2];
        sacw_GetPolarFromScreen(ClickLast.x, ClickLast.y, points);
    }
}

void HandleKeyUp(unsigned int keyCode)
{
    if (keyCode == KEY_PANNING) Panning = false;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void PaintCaption(HWND hwnd, HDC hdc)
{
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    HTHEME hTheme = OpenThemeData(nullptr, L"CompositedWindow::Window");
    if (hTheme)
    {
        HDC hdcPaint = CreateCompatibleDC(hdc);
        if (hdcPaint)
        {
            //int cx = RECTWIDTH(rcClient);
            //int cy = RECTHEIGHT(rcClient);

            int cx = rcClient.right - rcClient.left;
            int cy = rcClient.bottom - rcClient.top;

            // Define the BITMAPINFO structure used to draw text.
            // Note that biHeight is negative. This is done because
            // DrawThemeTextEx() needs the bitmap to be in top-to-bottom
            // order.
            BITMAPINFO dib = { 0 };
            dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            dib.bmiHeader.biWidth = cx;
            dib.bmiHeader.biHeight = -cy;
            dib.bmiHeader.biPlanes = 1;
            dib.bmiHeader.biBitCount = 24;
            dib.bmiHeader.biCompression = BI_RGB;

            HBITMAP hbm = CreateDIBSection(hdc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
            if (hbm)
            {
                HBITMAP hbmOld = (HBITMAP)SelectObject(hdcPaint, hbm);

                // Setup the theme drawing options.
                DTTOPTS DttOpts = { sizeof(DTTOPTS) };
                DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
                DttOpts.iGlowSize = 15;

                // Select a font.
                LOGFONT lgFont;
                HFONT hFontOld = NULL;
                 // if (SUCCEEDED(GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont)))
                 // {
                 //     HFONT hFont = CreateFontIndirect(&lgFont);
                 //     hFontOld = (HFONT)SelectObject(hdcPaint, hFont);
                 // }

                // Draw the title.
                RECT rcPaint = rcClient;
                rcPaint.top += 8;
                rcPaint.right -= 125;
                rcPaint.left += 8;
                rcPaint.bottom = 50;
                DrawThemeTextEx(
                    hTheme,
                    hdcPaint,
                    0, 0,
                    L"SacWeather",
                    -1,
                    DT_LEFT | DT_WORD_ELLIPSIS,
                    &rcPaint,
                    &DttOpts
                );

                // Blit text to the frame.
                BitBlt(hdc, 0, 0, cx, cy, hdcPaint, 0, 0, SRCCOPY);

                SelectObject(hdcPaint, hbmOld);
                if (hFontOld)
                {
                    SelectObject(hdcPaint, hFontOld);
                }
                DeleteObject(hbm);
            }
            DeleteDC(hdcPaint);
        }
        CloseThemeData(hTheme);
    }
}

HRESULT HitTestNCA(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
// Get the point coordinates for the hit test.
    POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // Get the window rectangle.
    RECT rcWindow;
    GetWindowRect(hwnd, &rcWindow);

    // Get the frame rectangle, adjusted for the style without a caption.
    RECT rcFrame = { 0 };
    AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

    // Determine if the hit test is for resizing. Default middle (1,1).
    USHORT uRow = 1;
    USHORT uCol = 1;
    bool fOnResizeBorder = false;

    // Determine if the point is at the top or bottom of the window.
    if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + TOPEXTENDWIDTH)
    {
        fOnResizeBorder = (ptMouse.y < (rcWindow.top - rcFrame.top));
        uRow = 0;
    }
    else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - BOTTOMEXTENDWIDTH)
    {
        uRow = 2;
    }

    // Determine if the point is at the left or right of the window.
    if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + LEFTEXTENDWIDTH)
    {
        uCol = 0; // left side
    }
    else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - RIGHTEXTENDWIDTH)
    {
        uCol = 2; // right side
    }

    // Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
    LRESULT hitTests[3][3] =
        {
            { HTTOPLEFT, fOnResizeBorder ? HTTOP : HTCAPTION, HTTOPRIGHT },
            { HTLEFT, HTNOWHERE, HTRIGHT },
            { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
        };

    return hitTests[uRow][uCol];
}

LRESULT CustomFrameCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, bool* passMessage)
{
    LRESULT lRet = 0;
    HRESULT hr = S_OK;
    bool fCallDWP = true; // Pass on to DefWindowProc?

    fCallDWP = !DwmDefWindowProc(hwnd, message, wParam, lParam, &lRet);

    // Handle window creation.
    if (message == WM_CREATE)
    {
        RECT rcClient;
        GetWindowRect(hwnd, &rcClient);

        // Inform application of the frame change.
//        SetWindowPos(
//            hwnd,
//            NULL,
//            rcClient.left, rcClient.top,
//            rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
//            SWP_FRAMECHANGED
//        );

        fCallDWP = true;
        lRet = 0;
    }

    // Handle window activation.
    if (message == WM_ACTIVATE)
    {
        // Extend the frame into the client area.
        MARGINS margins;

        margins.cxLeftWidth = LEFTEXTENDWIDTH;      // 8
        margins.cxRightWidth = RIGHTEXTENDWIDTH;    // 8
        margins.cyBottomHeight = BOTTOMEXTENDWIDTH; // 20
        margins.cyTopHeight = TOPEXTENDWIDTH;       // 27

        hr = DwmExtendFrameIntoClientArea(hwnd, &margins);

        if (!SUCCEEDED(hr))
        {
            // Handle error.
        }

        fCallDWP = true;
        lRet = 0;
    }

    if (message == WM_PAINT)
    {
        HDC hdc;
        {
            PAINTSTRUCT ps;
            hdc = BeginPaint(hwnd, &ps);
            PaintCaption(hwnd, hdc);
            EndPaint(hwnd, &ps);
        }

        fCallDWP = true;
        lRet = 0;
    }

    // Handle the non-client size message.
    if ((message == WM_NCCALCSIZE) && (wParam == TRUE))
    {
        // Calculate new NCCALCSIZE_PARAMS based on custom NCA inset.
        NCCALCSIZE_PARAMS* pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

        pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
        pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
        pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
        pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;

        lRet = 0;

        // No need to pass the message on to the DefWindowProc.
        fCallDWP = false;
    }

    // Handle hit testing in the NCA if not handled by DwmDefWindowProc.
    if ((message == WM_NCHITTEST) && (lRet == 0))
    {
        lRet = HitTestNCA(hwnd, wParam, lParam);

        if (lRet != HTNOWHERE)
        {
            fCallDWP = false;
        }
    }

    *passMessage = fCallDWP;

    return lRet;
}

LRESULT ApplicationMessageCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    HDC hdc;
    PAINTSTRUCT ps;

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
                sacw_Init(hwnd);

                CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
                sacw_UpdateViewport(cs->cx, cs->cy);

                RECT rcClient;
                GetWindowRect(hwnd, &rcClient);
                SetWindowPos(
                    hwnd,
                    NULL,
                    rcClient.left, rcClient.top,
                    rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                    SWP_FRAMECHANGED
                );
            }

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
            f32 delta = GET_WHEEL_DELTA_WPARAM(wParam); // @todo
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
                /*MoveWindow(
                    hwnd,
                    lParam & 0xffff,
                    (lParam & 0xffff0000) >> 16,
                    1000,
                    1000,
                    false
                );*/
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

        case WM_PAINT:
        {
            hdc = BeginPaint(hwnd, &ps);
            PaintCaption(hwnd, hdc);
            // Do I need to do things here?
            EndPaint(hwnd, &ps);
        }
            break;

        default:result = DefWindowProc(hwnd, message, wParam, lParam);
            break;
    }

    return result;
}

LRESULT CALLBACK WinMessageCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    HRESULT cpr = S_OK;
    BOOL dwmEnabled = FALSE;
    bool passMessage = true;
    cpr = DwmIsCompositionEnabled(&dwmEnabled);

    if (SUCCEEDED(cpr))
    {
        result = CustomFrameCallback(hwnd, message, wParam, lParam, &passMessage);
    }

    if(passMessage && ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))        
    {
        // @todo
        // This doesn't seem to work this way.
        ImGuiIO& io = ImGui::GetIO();
        passMessage = !io.WantCaptureMouse;
    }

    if (passMessage)
    {
        result = ApplicationMessageCallback(hwnd, message, wParam, lParam);
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

    HMENU menu = BuildMenu();
    HWND hwnd = CreateWindowEx(
        0,
        wndClass.lpszClassName,
        "SAC Weather",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,       // x, y
        //1000, 1000,                         // w, h
        CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

//    HWND hwnd = CreateWindowEx(
//        WS_EX_APPWINDOW,
//        wndClass.lpszClassName,
//        "SAC Weather",
//        WS_POPUP,
//        500, 250, //CW_USEDEFAULT, CW_USEDEFAULT,       // x, y
//        1000, 1000,                         // w, h
//        //CW_USEDEFAULT, CW_USEDEFAULT,
//        0,
//        nullptr,
//        instance,
//        0
//    );

    if (!hwnd)
    {
        LogMessage("Error creating window");
        return 0;
    }

    // maybe this isn't needed here
    DrawMenuBar(hwnd);

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



static LRESULT Win32InitOpenGL(HDC hdc)
{
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
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
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

