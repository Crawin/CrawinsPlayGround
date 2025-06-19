#pragma once

#define MAX_LOADSTRING 100

class CSubWindows
{
protected:
	WCHAR szWindowClass[MAX_LOADSTRING];
	WCHAR szTitle[MAX_LOADSTRING];
	HINSTANCE hInst;
	HWND m_hWnd;
public:
	CSubWindows();
	virtual ~CSubWindows();

	virtual ATOM RegisterSubWindowClass();
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
	virtual bool CreateSubWindow();
	virtual BOOL InitInstance();
};

class CHotkeyWindow : public CSubWindows {
private:
	std::vector<int> m_vIDHotkeys;
	std::list<CHotKey*> m_lptrHotkeys;
public:
	CHotkeyWindow(HINSTANCE& hInstance);
	virtual ~CHotkeyWindow();
	virtual ATOM RegisterSubWindowClass() override;
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	virtual bool CreateSubWindow() override;
	virtual BOOL InitInstance() override;
	bool RegisterHotKeys();
	bool DefineHotKeys();
};