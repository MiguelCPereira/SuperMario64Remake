#include "stdafx.h"
#include "MeshFilter.h"
XMFLOAT4 MeshFilter::m_DefaultColor = XMFLOAT4(1, 0, 0, 1);
XMFLOAT4 MeshFilter::m_DefaultFloat4 = XMFLOAT4(0, 0, 0, 0);
XMFLOAT3 MeshFilter::m_DefaultFloat3 = XMFLOAT3(0, 0, 0);
XMFLOAT2 MeshFilter::m_DefaultFloat2 = XMFLOAT2(0, 0);

MeshFilter::~MeshFilter()
{
	for (auto& subMesh : m_Meshes)
	{
		subMesh.Release();
	}
	m_Meshes.clear();
}

void MeshFilter::BuildIndexBuffer(const SceneContext& sceneContext)
{
	for (auto& subMesh : m_Meshes)
	{
		if (subMesh.buffers.pIndexBuffer)
			SafeRelease(subMesh.buffers.pIndexBuffer);

		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(UINT) * UINT(subMesh.indexCount);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = subMesh.indices.data();

		HANDLE_ERROR(sceneContext.d3dContext.pDevice->CreateBuffer(&bd, &initData, &subMesh.buffers.pIndexBuffer))
	}
}

int MeshFilter::GetVertexBufferId(UINT inputLayoutId, UINT8 subMeshId) const
{
	ASSERT_IF_(subMeshId >= m_Meshes.size())

		for (UINT i = 0; i < m_Meshes[subMeshId].buffers.vertexbuffers.size(); ++i)
		{
			if (m_Meshes[subMeshId].buffers.vertexbuffers[i].InputLayoutID == inputLayoutId)
				return i;
		}

	return -1;
}

void MeshFilter::BuildVertexBuffer(const SceneContext& sceneContext, UINT inputLayoutID, UINT inputLayoutSize, const std::vector<ILDescription>& inputLayoutDescriptions)
{
	for (auto i{ 0 }; i < m_Meshes.size(); ++i)
	{
		BuildVertexBuffer(sceneContext, inputLayoutID, inputLayoutSize, inputLayoutDescriptions, static_cast<UINT8>(i));
	}
}

void MeshFilter::BuildVertexBuffer(const SceneContext& sceneContext, UINT inputLayoutID, UINT inputLayoutSize, const std::vector<ILDescription>& inputLayoutDescriptions, UINT8 subMeshId)
{
	ASSERT_IF_(subMeshId >= m_Meshes.size())

	auto& subMesh = m_Meshes[subMeshId];

	//Check if VertexBufferInfo already exists with requested InputLayout
	if (GetVertexBufferId(inputLayoutID, subMeshId) >= 0)
		return;

	VertexBufferData data;
	data.VertexStride = inputLayoutSize;
	data.VertexCount = subMesh.vertexCount;
	data.BufferSize = data.VertexStride * subMesh.vertexCount;
	data.IndexCount = subMesh.indexCount;

	void* pDataLocation = malloc(data.BufferSize);
	if (pDataLocation == nullptr)
	{
		Logger::LogWarning(L"Failed to allocate the required memory!");
		return;
	}

	data.pDataStart = pDataLocation;
	data.InputLayoutID = inputLayoutID;

	for (UINT i = 0; i < subMesh.vertexCount; ++i)
	{
		for (UINT j = 0; j < inputLayoutDescriptions.size(); ++j)
		{
			const auto& ilDescription = inputLayoutDescriptions[j];

			if (i == 0 && !subMesh.HasElement(ilDescription.SemanticType))
			{
				std::wstring name = EffectHelper::GetIlSemanticName(ilDescription.SemanticType);
				Logger::LogWarning(L"Mesh \"{}\" has no vertex {} data, using a default value!", m_MeshName.c_str(), name.c_str());
			}

			switch (ilDescription.SemanticType)
			{
			case ILSemantic::POSITION:
				memcpy(pDataLocation, subMesh.HasElement(ilDescription.SemanticType) ? &subMesh.positions[i] : &m_DefaultFloat3, ilDescription.Offset);
				break;
			case ILSemantic::NORMAL:
				memcpy(pDataLocation, subMesh.HasElement(ilDescription.SemanticType) ? &subMesh.normals[i] : &m_DefaultFloat3, ilDescription.Offset);
				break;
			case ILSemantic::COLOR:
				memcpy(pDataLocation, subMesh.HasElement(ilDescription.SemanticType) ? &subMesh.colors[i] : &m_DefaultColor, ilDescription.Offset);
				break;
			case ILSemantic::TEXCOORD:
				memcpy(pDataLocation, subMesh.HasElement(ilDescription.SemanticType) ? &subMesh.texCoords[i] : &m_DefaultFloat2, ilDescription.Offset);
				break;
			case ILSemantic::TANGENT:
				memcpy(pDataLocation, subMesh.HasElement(ilDescription.SemanticType) ? &subMesh.tangents[i] : &m_DefaultFloat3, ilDescription.Offset);
				break;
			case ILSemantic::BINORMAL:
				memcpy(pDataLocation, subMesh.HasElement(ilDescription.SemanticType) ? &subMesh.binormals[i] : &m_DefaultFloat3, ilDescription.Offset);
				break;
			case ILSemantic::BLENDINDICES:
				memcpy(pDataLocation, subMesh.HasElement(ilDescription.SemanticType) ? &subMesh.blendIndices[i] : &m_DefaultFloat4, ilDescription.Offset);
				break;
			case ILSemantic::BLENDWEIGHTS:
				memcpy(pDataLocation, subMesh.HasElement(ilDescription.SemanticType) ? &subMesh.blendWeights[i] : &m_DefaultFloat4, ilDescription.Offset);
				break;
			default:
				HANDLE_ERROR(L"Unsupported SemanticType!");
				break;
			}

			pDataLocation = static_cast<char*>(pDataLocation) + ilDescription.Offset;
		}
	}

	//fill a buffer description to copy the vertexdata into graphics memory
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = data.BufferSize;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = data.pDataStart;

	//create a ID3D10Buffer in graphics memory containing the vertex info
	sceneContext.d3dContext.pDevice->CreateBuffer(&bd, &initData, &data.pVertexBuffer);

	subMesh.buffers.vertexbuffers.push_back(data);
}

void MeshFilter::BuildVertexBuffer(const SceneContext& sceneContext, BaseMaterial* pMaterial, UINT8 subMeshId)
{
	auto& techiqueContext = pMaterial->GetTechniqueContext();
	return BuildVertexBuffer(sceneContext, techiqueContext.inputLayoutID, techiqueContext.inputLayoutSize, techiqueContext.pInputLayoutDescriptions, subMeshId);
}

void MeshFilter::BuildVertexBuffer(const SceneContext& sceneContext, BaseMaterial* pMaterial)
{
	for (auto i{ 0 }; i < m_Meshes.size(); ++i)
	{
		BuildVertexBuffer(sceneContext, pMaterial, static_cast<UINT8>(i));
	}
}

const VertexBufferData& MeshFilter::GetVertexBufferData(UINT inputLayoutId, UINT8 subMeshId) const
{
	ASSERT_IF_(subMeshId >= m_Meshes.size())

	const int possibleBuffer = GetVertexBufferId(inputLayoutId, subMeshId);
	ASSERT_IF(possibleBuffer < 0, L"No VertexBufferInformation for given inputLayoutId found!")

	return m_Meshes[subMeshId].buffers.vertexbuffers[possibleBuffer];
}

const VertexBufferData& MeshFilter::GetVertexBufferData(const SceneContext& sceneContext, BaseMaterial* pMaterial, UINT8 subMeshId)
{
	ASSERT_IF_(subMeshId >= m_Meshes.size())

	auto& techniqueContext = pMaterial->GetTechniqueContext();
	const int possibleBuffer = GetVertexBufferId(techniqueContext.inputLayoutID, subMeshId);

	if (possibleBuffer < 0)
	{
		Logger::LogWarning(L"No VertexBufferInformation for this material found! Building matching VertexBufferInformation (Performance Issue).");
		BuildVertexBuffer(sceneContext, pMaterial, subMeshId);

		//Return last created vertexbufferinformation
		return m_Meshes[subMeshId].buffers.vertexbuffers.back();
	}

	return m_Meshes[subMeshId].buffers.vertexbuffers[possibleBuffer];
}

ID3D11Buffer* MeshFilter::GetIndexBuffer(UINT8 subMeshId) const
{
	ASSERT_IF_(subMeshId >= m_Meshes.size());
	return m_Meshes[subMeshId].buffers.pIndexBuffer;
}
