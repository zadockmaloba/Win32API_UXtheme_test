#include <windows.h>
#include <tchar.h>
#include <uxtheme.h>

#pragma comment(lib, "uxtheme.lib")

#define WNDCLASSNAME L"BufferedPaintSample_WndClass"
#define ANIMATION_DURATION 500

bool g_fCurrentState = true;
bool g_fNewState = true;

void StartAnimation(HWND hWnd)
{
    g_fNewState = !g_fCurrentState;
    InvalidateRect(hWnd, NULL, TRUE);
}

void Paint(HWND hWnd, HDC hdc, bool state)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    LPCTSTR pszIconId = state ? IDI_APPLICATION : IDI_ERROR;
    HICON hIcon = LoadIcon(NULL, pszIconId);
    if (hIcon)
    {
        DrawIcon(hdc, 10, 10, hIcon);
        DestroyIcon(hIcon);
    }
}

void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    if (hdc)
    {
        // See if this paint was generated by a soft-fade animation
        if (!BufferedPaintRenderAnimation(hWnd, hdc))
        {
            BP_ANIMATIONPARAMS animParams;
            ZeroMemory(&animParams, sizeof(animParams));
            animParams.cbSize = sizeof(BP_ANIMATIONPARAMS);
            animParams.style = BPAS_LINEAR;

            // Check if animation is needed. If not set dwDuration to 0
            animParams.dwDuration = (g_fCurrentState != g_fNewState ? ANIMATION_DURATION : 0);

            RECT rc;
            GetClientRect(hWnd, &rc);

            HDC hdcFrom, hdcTo;
            HANIMATIONBUFFER hbpAnimation = BeginBufferedAnimation(hWnd, hdc, &rc,
                BPBF_COMPATIBLEBITMAP, NULL, &animParams, &hdcFrom, &hdcTo);
            if (hbpAnimation)
            {
                if (hdcFrom)
                {
                    Paint(hWnd, hdcFrom, g_fCurrentState);
                }
                if (hdcTo)
                {
                    Paint(hWnd, hdcTo, g_fNewState);
                }

                g_fCurrentState = g_fNewState;
                EndBufferedAnimation(hbpAnimation, TRUE);
            }
            else
            {
                Paint(hWnd, hdc, g_fCurrentState);
            }
        }

        EndPaint(hWnd, &ps);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        StartAnimation(hWnd);
        break;
    case WM_PAINT:
        OnPaint(hWnd);
        break;
    case WM_SIZE:
        BufferedPaintStopAllAnimations(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI _tWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR     lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (SUCCEEDED(BufferedPaintInit()))
    {
        WNDCLASSEX wcex;
        ZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wcex.lpszClassName = WNDCLASSNAME;
        RegisterClassEx(&wcex);

        HWND hWnd = CreateWindow(WNDCLASSNAME, L"Buffered Paint Sample",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
            CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

        if (hWnd)
        {
            ShowWindow(hWnd, nCmdShow);
            UpdateWindow(hWnd);

            MSG msg;
            while (GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        BufferedPaintUnInit();
    }

    return 0;
}