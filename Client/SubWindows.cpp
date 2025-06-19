#include "stdafx.h"
#include "Resource.h"
#include "Hotkey.h"
#include "SubWindows.h"

CSubWindows::CSubWindows()
{
}

CSubWindows::~CSubWindows()
{
}

ATOM CSubWindows::RegisterSubWindowClass()
{
	std::cout << "서브 윈도우 기본생성자 오류\n";
	return ATOM();
}

LRESULT CALLBACK CSubWindows::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CSubWindows* pThis;

    if (message == WM_NCCREATE) {
        // CreateWindow에서 넘긴 this를 받아서 저장
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<CSubWindows*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else {
        // 생성 이후에 다른 행위가 감지되면 저장된 this 불러오기
        pThis = reinterpret_cast<CSubWindows*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->WndProc(hWnd, message, wParam, lParam);  // 실제 멤버 함수 호출
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool CSubWindows::CreateSubWindow()
{
    return false;
}

BOOL CSubWindows::InitInstance()
{
    return 0;
}

CHotkeyWindow::CHotkeyWindow(HINSTANCE& hInstance)
{
    hInst = hInstance;
    wcscpy_s(szTitle, L"HotkeyWindow");
    wcscpy_s(szWindowClass, L"HotkeyWindowClass");
}

CHotkeyWindow::~CHotkeyWindow()
{
    for (int i = 0; i < m_vIDHotkeys.size(); ++i) {
        UnregisterHotKey(m_hWnd, i);
    }

}

ATOM CHotkeyWindow::RegisterSubWindowClass()
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = CSubWindows::StaticWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

LRESULT CHotkeyWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        if (wParam == VK_RETURN) {
            std::cout << "엔터다\n";
        }
        break;
    case WM_HOTKEY:// 핫키에 등록된 키보드 입력시, 핫키가 register되어있는 윈도우에 이벤트 전송
        switch (m_vIDHotkeys[wParam]) {
        case VK_PAUSE:
            std::cout << "마이크 음소거\n";
            // need modify => 기능들을 id로 묶고, 이를 execute하는 형식으로 수정필요
            m_lptrHotkeys.front()->execute();
            break;
        }
        break;
    case WM_LBUTTONDOWN:    // 창 위치 이동
        ReleaseCapture();
        SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool CHotkeyWindow::CreateSubWindow()
{
    RegisterSubWindowClass();
    InitInstance();
    m_lptrHotkeys.emplace_back(new CMIC());
    return true;
}

BOOL CHotkeyWindow::InitInstance()
{
    HWND hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED, // 확장 스타일 (필요시)
        szWindowClass,       // 등록된 윈도우 클래스
        szTitle,                     // 창 제목 없음
        WS_POPUP,                // ★ 테두리, 캡션 없는 스타일
        100, 100, 300, 200,      // 위치와 크기
        NULL, NULL, hInst, this);   // lParam 에 this를 넣어서 wndProc 클래스별 할당
    if (!hWnd)
    {
        DWORD dwError = GetLastError();
        wchar_t msg[256];
        swprintf(msg, 256, L"CreateWindowEx failed! Error code: %lu", dwError);
        MessageBox(NULL, msg, L"Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    m_hWnd = hWnd;
    SetLayeredWindowAttributes(m_hWnd, 0, 100, LWA_ALPHA);

    DefineHotKeys();
    RegisterHotKeys();

    SetFocus(m_hWnd);
    ShowWindow(m_hWnd, 1);
    UpdateWindow(m_hWnd);

    return TRUE;
}

bool CHotkeyWindow::RegisterHotKeys()
{
    for (int i = 0; i < m_vIDHotkeys.size(); ++i) {
        RegisterHotKey(m_hWnd, i, 0, m_vIDHotkeys[i]);
    }
    return true;
}

bool CHotkeyWindow::DefineHotKeys()
{
    m_vIDHotkeys.emplace_back(VK_PAUSE);
    return true;
}
