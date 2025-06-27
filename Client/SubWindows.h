#pragma once
#include "Hotkey.h"
#define MAX_LOADSTRING 100

class CSubWindows
{
protected:
	WCHAR szWindowClass[MAX_LOADSTRING];
	WCHAR szTitle[MAX_LOADSTRING];
	HINSTANCE hInst;
	HWND m_hWnd;
	int m_iX, m_iY, m_iWidth, m_iHeight;
public:
	CSubWindows();
	virtual ~CSubWindows();

	virtual ATOM RegisterSubWindowClass() = 0;
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
	virtual bool CreateSubWindow() = 0;
	virtual BOOL InitInstance() = 0;
	void ReleaseSubWindow();
};

class CStreamDeckWindow : public CSubWindows {
private:
	typedef int ID;
	ID m_currentID = 0;
	std::map<ID, HotKeyData> m_mHotkeys;
public:
	CStreamDeckWindow(const HINSTANCE& hInstance);
	CStreamDeckWindow(const HINSTANCE& hInstance, const int& X, const int& Y, const int& nWidth, const int& nHeight);

	virtual ~CStreamDeckWindow();
	virtual ATOM RegisterSubWindowClass() override;
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	virtual bool CreateSubWindow() override;
	virtual BOOL InitInstance() override;
	bool RegisterHotKeys();
	bool DefineHotKeys();
};