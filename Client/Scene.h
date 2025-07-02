#pragma once
class CShader;
class CCamera;

class CScene
{
public:
	CScene();
	virtual ~CScene();

	bool OnProcessingMouseMessage(const HWND& hWnd, const HINSTANCE& hInst, const UINT& nMessageID, const WPARAM& wParam, const LPARAM& lParam);
	bool OnProcessingKeyboardMessage(const HWND& hWnd, const HINSTANCE& hInst, const UINT& nMessageID, const WPARAM& wParam, const LPARAM& lParam);

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();
	bool ProcessInput();
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, const float& fTimeElapsed);

	void ReleaseUploadBuffers();

	//그래픽 루트 시그너쳐를 생성한다. 
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature();

	void SetCamera(const int& ViewPortWidth, const int& ViewPortHeight);
protected:
	//씬은 셰이더들의 집합이다. 셰이더들은 게임 객체들의 집합이다.
	CShader **m_ppShaders = NULL;
	int m_nShaders = 0;
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	CCamera* m_pCamera = NULL;
};

