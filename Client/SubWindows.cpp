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

LRESULT CALLBACK CSubWindows::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CSubWindows* pThis;

    switch (message) {
    case WM_NCCREATE:
    {
        // CreateWindow에서 넘긴 this를 받아서 저장
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<CSubWindows*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
        break;
    default:
        // 생성 이후에 다른 행위가 감지되면 저장된 this 불러오기
        pThis = reinterpret_cast<CSubWindows*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        break;
    }

    if (pThis) {
        return pThis->WndProc(hWnd, message, wParam, lParam);  // 실제 멤버 함수 호출
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void CSubWindows::ReleaseSubWindow()
{
    DestroyWindow(m_hWnd);
}

CStreamDeckWindow::CStreamDeckWindow(HINSTANCE& hInstance)
{
    hInst = hInstance;
    wcscpy_s(szTitle, L"HotkeyWindow");
    wcscpy_s(szWindowClass, L"HotkeyWindowClass");
}

CStreamDeckWindow::~CStreamDeckWindow()
{
    for (const auto& [id, hotkeydata] : m_mHotkeys) {
        UnregisterHotKey(m_hWnd, id);
        delete hotkeydata.HotKeyFunc;
    }
}

ATOM CStreamDeckWindow::RegisterSubWindowClass()
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

LRESULT CStreamDeckWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_NCDESTROY:  // 윈도우 삭제시 메모리 해제
        delete this;
        return 0;
        break;
    case WM_KEYDOWN:
        if (wParam == VK_RETURN) {
            std::cout << "엔터다\n";
        }
        break;
    case WM_HOTKEY:// 핫키에 등록된 키보드 입력시, 핫키가 register되어있는 윈도우에 이벤트 전송
    {
        auto it = m_mHotkeys.find(static_cast<int>(wParam));
        if (it != m_mHotkeys.end()) {   // 등록된 id이면 실행
            it->second.HotKeyFunc->execute(hWnd);
        }
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

bool CStreamDeckWindow::CreateSubWindow()
{
    RegisterSubWindowClass();
    InitInstance();
    DefineHotKeys();
    RegisterHotKeys();
    return true;
}

BOOL CStreamDeckWindow::InitInstance()
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
    SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);

    ShowWindow(m_hWnd, 1);
    UpdateWindow(m_hWnd);

    return TRUE;
}

bool CStreamDeckWindow::RegisterHotKeys()
{
    for (const auto& [id, hotkeydata] : m_mHotkeys) {
        RegisterHotKey(m_hWnd, id, hotkeydata.InputModifier, hotkeydata.HotKeyInput);
    }
    return true;
}

bool CStreamDeckWindow::DefineHotKeys()
{
    /*JSON파일 읽어서 키보드 인풋 값을 불러온 후 m_mHotkeys에 삽입*/
    m_mHotkeys.try_emplace(m_currentID++, VK_PAUSE, 0, new CMIC);
    return true;
}
