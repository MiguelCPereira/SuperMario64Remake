#include "stdafx.h"
#include "MeshDrawComponent.h"

ID3DX11EffectMatrixVariable* MeshDrawComponent::m_pWorldVar = nullptr;
ID3DX11EffectMatrixVariable* MeshDrawComponent::m_pWvpVar = nullptr;

MeshDrawComponent::MeshDrawComponent(UINT triangleCapacity, bool enableTransparency):
	m_pVertexBuffer(nullptr),
	m_TriangleCapacity(triangleCapacity),
	m_pEffect(nullptr),
	m_pTechnique(nullptr),
	m_pInputLayout(nullptr),
	m_UseTransparency(enableTransparency)
{}

MeshDrawComponent::~MeshDrawComponent()
{
	SafeRelease(m_pInputLayout);
	SafeRelease(m_pVertexBuffer);
}

void MeshDrawComponent::Initialize(const SceneContext& sceneContext)
{
	LoadEffect(sceneContext);
	InitializeBuffer(sceneContext);
	UpdateBuffer();
}

void MeshDrawComponent::LoadEffect(const SceneContext& sceneContext)
{
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"Effects\\PosNormCol3D.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(int(m_UseTransparency));
	EffectHelper::BuildInputLayout(sceneContext.d3dContext.pDevice, m_pTechnique, &m_pInputLayout);

	if (!m_pWorldVar)
		m_pWorldVar = m_pEffect->GetVariableBySemantic("World")->AsMatrix();

	if (!m_pWvpVar)
		m_pWvpVar = m_pEffect->GetVariableBySemantic("WorldViewProjection")->AsMatrix();
}

void MeshDrawComponent::InitializeBuffer(const SceneContext& sceneContext)
{
	if (m_pVertexBuffer)
		SafeRelease(m_pVertexBuffer);

	//*************
	//VERTEX BUFFER
	D3D11_BUFFER_DESC vertexBuffDesc{};
	vertexBuffDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBuffDesc.ByteWidth = sizeof(TrianglePosNormCol) * m_TriangleCapacity;
	vertexBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	vertexBuffDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	vertexBuffDesc.MiscFlags = 0;

	sceneContext.d3dContext.pDevice->CreateBuffer(&vertexBuffDesc, nullptr, &m_pVertexBuffer);
}

void MeshDrawComponent::UpdateBuffer() const
{
	const auto scene = m_pGameObject->GetScene();
	if (!scene)
	{
#if _DEBUG
		Logger::LogWarning(L"MeshDrawComponent::UpdateBuffer > Can't update buffer, Component is not part of a scene. (= No DeviceContext)");
#endif
		return;
	}

	auto& d3d11 = scene->GetSceneContext().d3dContext;
	auto size = m_vecTriangles.size();

	if (size > 0)
	{
		if (size > m_TriangleCapacity)
		{
			Logger::LogInfo(L"MeshDrawComponent::UpdateBuffer > Buffer size clamped. (Increase TriangleCapacity)");
			size = m_TriangleCapacity;
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		d3d11.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
		memcpy(mappedResource.pData, m_vecTriangles.data(), sizeof(TrianglePosNormCol) * size);
		d3d11.pDeviceContext->Unmap(m_pVertexBuffer, 0);
	}
}

void MeshDrawComponent::Draw(const SceneContext& sceneContext)
{
	//Set Shader Variables
	if (m_vecTriangles.empty())
		return;

	auto& d3d11 = sceneContext.d3dContext;
	auto world = XMLoadFloat4x4(&GetTransform()->GetWorld());
	const auto viewProjection = XMLoadFloat4x4(&sceneContext.pCamera->GetViewProjection());

	m_pWorldVar->SetMatrix(reinterpret_cast<float*>(&world));
	auto wvp = world * viewProjection;
	m_pWvpVar->SetMatrix(reinterpret_cast<float*>(&wvp));

	d3d11.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d11.pDeviceContext->IASetInputLayout(m_pInputLayout);

	constexpr UINT offset = 0;
	constexpr UINT stride = sizeof(VertexPosNormCol);
	d3d11.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, d3d11.pDeviceContext);
		d3d11.pDeviceContext->Draw(static_cast<UINT>(m_vecTriangles.size()) * 3, 0);
	}
}

void MeshDrawComponent::AddQuad(VertexPosNormCol vertex1, VertexPosNormCol vertex2, VertexPosNormCol vertex3,
                                VertexPosNormCol vertex4, bool updateBuffer)
{
	AddTriangle(TrianglePosNormCol(vertex1, vertex2, vertex3), false);
	AddTriangle(TrianglePosNormCol(vertex3, vertex4, vertex1), updateBuffer);
}

void MeshDrawComponent::AddQuad(QuadPosNormCol quad, bool updateBuffer)
{
	AddTriangle(TrianglePosNormCol(quad.Vertex1, quad.Vertex2, quad.Vertex3), false);
	AddTriangle(TrianglePosNormCol(quad.Vertex3, quad.Vertex4, quad.Vertex1), updateBuffer);
}

void MeshDrawComponent::AddTriangle(VertexPosNormCol vertex1, VertexPosNormCol vertex2, VertexPosNormCol vertex3,
                                    bool updateBuffer)
{
	AddTriangle(TrianglePosNormCol(vertex1, vertex2, vertex3), updateBuffer);
}

void MeshDrawComponent::AddTriangle(TrianglePosNormCol triangle, bool updateBuffer)
{
	m_vecTriangles.push_back(triangle);

	if (updateBuffer && m_IsInitialized)
		UpdateBuffer();
}

void MeshDrawComponent::RemoveTriangles()
{
	m_vecTriangles.clear();
}
