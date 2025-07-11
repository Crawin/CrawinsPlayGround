#include "stdafx.h"
#include "Timer.h"
#include "Shader.h"
#include "Camera.h"
#include "Scene.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}

bool CScene::OnProcessingMouseMessage(const HWND& hWnd, const HINSTANCE& hInst, const UINT& nMessageID, const WPARAM& wParam, const LPARAM& lParam)
{
	return false;
}

bool CScene::OnProcessingKeyboardMessage(const HWND& hWnd, const HINSTANCE& hInst, const UINT& nMessageID, const WPARAM& wParam, const LPARAM& lParam)
{
	return false;
}


void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//그래픽 루트 시그너쳐를 생성한다. 
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	//씬을 그리기 위한 셰이더 객체를 생성한다. 
	m_nShaders = 1;
	m_ppShaders = new CShader * [m_nShaders];
	//CShader* pShader = new CGeometryShader();
	CShader* pShader = new CSkyboxShader;
	//CShader* pShader = new CInstancingILShader();
	//CShader* pShader = new CObjectsShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->BuildObjects(pd3dDevice, pd3dCommandList);
	m_ppShaders[0] = pShader;
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}
}

bool CScene::ProcessInput()
{
	return false;
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	}
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//그래픽 루트 시그너쳐를 파이프라인에 연결(설정)한다. 
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	m_pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	m_pCamera->UpdateShaderVariables(pd3dCommandList);
	
	//씬을 렌더링하는 것은 씬을 구성하는 셰이더(셰이더가 포함하는 객체)들을 렌더링하는 것이다. 
	for (int i = 0; i < m_nShaders; i++)
	{
		//m_ppShaders[i]->Render(pd3dCommandList);
		m_ppShaders[i]->Render(pd3dCommandList,m_pCamera);
		//reinterpret_cast<CInstancingILShader*>(m_ppShaders[i])->Render(pd3dCommandList, m_pCamera);
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_ppShaders)
	{
		for (int j = 0; j < m_nShaders; j++) if (m_ppShaders[j])
			m_ppShaders[j]->ReleaseUploadBuffers();
	}
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];
	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; // t0
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[4];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 16;
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].Constants.ShaderRegister = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].Constants.ShaderRegister = 1;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 1;
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].Constants.ShaderRegister = 2;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
	return(pd3dGraphicsRootSignature);
}

ID3D12RootSignature* CScene::GetGraphicsRootSignature()
{
	return(m_pd3dGraphicsRootSignature);
}

void CScene::SetCamera(const int& ViewPortWidth, const int& ViewPortHeight)
{
	if (!m_pCamera)
		m_pCamera = new CCamera;

	m_pCamera->SetViewport(0, 0, ViewPortWidth, ViewPortHeight, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, ViewPortWidth, ViewPortHeight);
	m_pCamera->GenerateProjectionMatrix(1.0f, 500.0f, float(ViewPortWidth) / float(ViewPortHeight), 90.0f);
	m_pCamera->GenerateViewMatrix(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_pCamera->GenerateFrustum();
}
