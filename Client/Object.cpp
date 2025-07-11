#include "stdafx.h"
#include "Shader.h"
#include "Camera.h"
#include "Object.h"

CObject::CObject()
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	m_xmf3BaseWorld = { 0,0,0 };
	m_xmf3RotAngle = { 0,0,0 };
}

CObject::~CObject()
{
	if (m_pMesh) m_pMesh->Release();
}

void CObject::ReleaseUploadBuffers()
{//정점 버퍼를 위한 업로드 버퍼를 소멸시킨다.
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}

void CObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CObject::Animate(float fTimeElapsed)
{
}

void CObject::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//XMFLOAT4X4 xmf4x4World;
	//::XMStoreFloat4x4(&xmf4x4World, ::XMMatrixTranspose(::XMLoadFloat4x4(&m_xmf4x4World)));
	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}

void CObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	OnPrepareRender(pd3dCommandList);
	UpdateShaderVariables(pd3dCommandList);
	//게임 객체에 메쉬가 연결되어 있으면 메쉬를 렌더링한다.
	if (m_pMesh) m_pMesh->Render(pd3dCommandList);
}

void CObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = ::XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	//객체의 월드 변환 행렬을 루트 상수(32-비트 값)를 통하여 셰이더 변수(상수 버퍼)로 복사한다. 
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}

void CObject::ReleaseShaderVariables()
{
}

XMFLOAT3 CObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CObject::GetLook()
{
	XMFLOAT3 look = { m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33 };
	return(Vector3::Normalize(look));
}

XMFLOAT3 CObject::GetUp()
{
	XMFLOAT3 up = { m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23 };
	return(Vector3::Normalize(up));
}

XMFLOAT3 CObject::GetRight()
{
	XMFLOAT3 right = { m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13 };
	return(Vector3::Normalize(right));
}

CRotatingObject::CRotatingObject()
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 90.0f;
	m_fAnimationTime = 0;
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	//Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
	m_xmf3RotAngle.z += m_fRotationSpeed * fTimeElapsed;
	if (m_xmf3RotAngle.z > 360) {
		m_xmf3RotAngle.z -= 360;
	}
	m_fAnimationTime += fTimeElapsed;
	if (m_fAnimationTime > 2 * 3.141592) {
		m_fAnimationTime -= (2 * 3.141592);
	}
	//m_xmf4x4World._41 = (float)(16 * sin(m_fAnimationTime * 2)) + m_xmf3BaseWorld.x;
	m_xmf4x4World._42 = (float)(4*sin(m_fAnimationTime *2)) + m_xmf3BaseWorld.y;
}

void CRotatingObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView)
{
	OnPrepareRender(pd3dCommandList);
	UpdateShaderVariables(pd3dCommandList);
	//게임 객체에 메쉬가 연결되어 있으면 메쉬를 렌더링한다.
	if (m_pMesh) 
		reinterpret_cast<CCubeMeshDiffusedIndexed*>(m_pMesh)->Render(pd3dCommandList, nInstances, d3dInstancingBufferView);

}

bool CRotatingObject::IsVisible(CCamera* pCamera)
{
	auto boundingBox = m_pMesh->GetBoundingBox();
	XMMATRIX worldtransform = XMLoadFloat4x4(&m_xmf4x4World);
	boundingBox.Transform(boundingBox, worldtransform);
	return pCamera->IsInFrustum(boundingBox);
}

void CObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
	m_xmf3BaseWorld = { x,y,z };
}

void CObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	SetPosition(xmf3Position);
}

void CObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance); 
	SetPosition(xmf3Position);
}

void CObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	SetPosition(xmf3Position);
}

void CObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

CSkybox::CSkybox()
{
}

CSkybox::~CSkybox()
{
}

CTexture::CTexture()
{
	m_nTextures = 1;
	m_ppd3dTextureUploadBuffers = new ID3D12Resource * [m_nTextures];
	m_ppd3dTextures = new ID3D12Resource * [m_nTextures];
	m_ppd3dTextures[0] = m_ppd3dTextureUploadBuffers[0] = NULL;
		//CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, L"MilkyWayPanorama8K.dds", &m_ppd3dTextureUploadBuffers[0], D3D12_RESOURCE_STATE_GENERIC_READ);

}

CTexture::~CTexture()
{
	for (int i = 0; i < m_nTextures; ++i) {
		m_ppd3dTextures[i]->Release();
		m_ppd3dTextureUploadBuffers[i]->Release();
		m_pDescriptorHeap->Release();
	}
}

void CTexture::LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,const wchar_t* pszFileName, UINT nResourceType, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_GENERIC_READ);

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = 1; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pDescriptorHeap);
	m_d3dCPUDescriptorStartHandle = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dGPUDescriptorStartHandle = m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	D3D12_RESOURCE_DESC textureDesc = m_ppd3dTextures[nIndex]->GetDesc();
	D3D12_SHADER_RESOURCE_VIEW_DESC Desc;
	Desc.Format = textureDesc.Format;
	Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	Desc.TextureCube.MipLevels = textureDesc.MipLevels;
	Desc.TextureCube.MostDetailedMip = 0;
	Desc.TextureCube.ResourceMinLODClamp = 0.0f;
	Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	pd3dDevice->CreateShaderResourceView(m_ppd3dTextures[nIndex], &Desc, m_d3dCPUDescriptorStartHandle);
}

void CTexture::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	ID3D12DescriptorHeap* ppHeaps[] = { m_pDescriptorHeap };
	pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	pd3dCommandList->SetGraphicsRootDescriptorTable(3, m_d3dGPUDescriptorStartHandle);
}
