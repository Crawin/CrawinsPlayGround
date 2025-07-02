﻿#pragma once

class CScene
{
public:
	CScene();
	virtual ~CScene();

	bool OnProcessingMouseMessage(const HWND& hWnd, const HINSTANCE& hInst, const UINT& nMessageID, const WPARAM& wParam, const LPARAM& lParam);
	bool OnProcessingKeyboardMessage(const HWND& hWnd, const HINSTANCE& hInst, const UINT& nMessageID, const WPARAM& wParam, const LPARAM& lParam);

	void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice);

	void BuildObjects(ID3D12Device* pd3dDevice);
	void ReleaseObjects();
	bool ProcessInput();
	void AnimateObjects(float fTimeElapsed);
	void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList);

	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	//루트 시그너쳐를 나타내는 인터페이스 포인터이다.
	ID3D12PipelineState *m_pd3dPipelineState = NULL;
	//파이프라인 상태를 나타내는 인터페이스 포인터이다. 
};

