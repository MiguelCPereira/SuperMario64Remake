#include "stdafx.h"
#include "SpriteRenderer.h"

void SpriteRenderer::Initialize()
{
	//Effect
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"Effects/SpriteRenderer.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
	EffectHelper::BuildInputLayout(m_GameContext.d3dContext.pDevice, m_pTechnique, &m_pInputLayout);

	QUERY_EFFECT_VARIABLE_HALT(m_pEffect, m_pEVar_TransformMatrix, gTransform, Matrix);
	QUERY_EFFECT_VARIABLE_HALT(m_pEffect, m_pEVar_TextureSize, gTextureSize, Vector);
	QUERY_EFFECT_VARIABLE_HALT(m_pEffect, m_pEVar_TextureSRV, gSpriteTexture, ShaderResource);

	//Transform Matrix
	const float scaleX = (m_GameContext.windowWidth > 0) ? 2.0f / float(m_GameContext.windowWidth) : 0;
	const float scaleY = (m_GameContext.windowHeight > 0) ? 2.0f / float(m_GameContext.windowHeight) : 0;

	m_Transform = XMFLOAT4X4{
		scaleX, 0,       0, 0,
		0,      -scaleY, 0 ,0,
		0,      0,       1, 0,
		-1,     1,       0, 1
	};
}

bool SpriteRenderer::SpriteSortByTexture(const VertexSprite& v0, const VertexSprite& v1)
{
	return v0.TextureId < v1.TextureId;
}

bool SpriteRenderer::SpriteSortByDepth(const VertexSprite& v0, const VertexSprite& v1)
{
	return v0.TransformData.z < v1.TransformData.z;
}

SpriteRenderer::~SpriteRenderer()
{
	SafeRelease(m_pInputLayout);
	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pImmediateVertexBuffer);

	m_Sprites.clear();
	m_Textures.clear();
}

void SpriteRenderer::UpdateBuffer(const SceneContext& sceneContext)
{
	//TODO_W4(L"Complete UpdateBuffer")

	// if the vertex buffer does not exists, or the number of sprites is bigger then the buffer size
	if (!m_pVertexBuffer || m_Sprites.size() > m_BufferSize)
	{
		// Release the buffer
		if(m_pVertexBuffer)
			SafeRelease(m_pVertexBuffer);

		// Update the buffer size (if needed)
		if (m_Sprites.size() > m_BufferSize)
			m_BufferSize = UINT(m_Sprites.size());

		// Set up the descriptor for the Depth/Stencil Buffer
		// (make sure the Usage flag is set to Dynamic, bound as vertex buffer
		// and set the cpu access flags to access_write)
		D3D11_BUFFER_DESC buffDesc;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		buffDesc.ByteWidth = sizeof(VertexSprite) * m_BufferSize;
		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;

		//Create the buffer
		HANDLE_ERROR(sceneContext.d3dContext.pDevice->CreateBuffer(&buffDesc, nullptr, &m_pVertexBuffer));
	}

	//------------------------
	//Sort Sprites
	//SORT BY TEXTURE
	std::ranges::sort(m_Sprites, [](const VertexSprite& v0, const VertexSprite& v1)
	{
		return v0.TextureId < v1.TextureId;
	});

	//SORT BY DEPTH
	std::ranges::sort(m_Sprites, SpriteSortByDepth);
	std::ranges::sort(m_Sprites, [](const VertexSprite& v0, const VertexSprite& v1)
	{
		if (v0.TextureId == v1.TextureId)
		{
			return v0.TransformData.z < v1.TransformData.z;
		}

		return false;
	});
	//------------------------

	//Fill Buffer
	if (m_pVertexBuffer)
	{
		// Finally fill the  buffer

		// Create a D3D11_MAPPED_SUBRESOURCE
		D3D11_MAPPED_SUBRESOURCE mappedSubresource{};

		// Map the vertex buffer to the mapped resource
		sceneContext.d3dContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD,0, &mappedSubresource);
		//sceneContext.d3dContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedSubresource);

		// Copy all our sprite vertices to the mapped resource 
		memcpy(mappedSubresource.pData, m_Sprites.data(), sizeof(VertexSprite) * m_Sprites.size());

		// Unmap the vertex buffer
		sceneContext.d3dContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
	}
}

void SpriteRenderer::Draw(const SceneContext& sceneContext)
{
	if (m_Sprites.size() == 0)
		return;

	UpdateBuffer(sceneContext);

	const auto pDeviceContext = sceneContext.d3dContext.pDeviceContext;

	//Set Render Pipeline
	pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	constexpr UINT stride = sizeof(VertexSprite);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	UINT batchSize = 1;
	UINT batchOffset = 0;
	const size_t spriteCount = m_Sprites.size();
	for (size_t i = 0; i < spriteCount; ++i)
	{
		if (i < (spriteCount - 1) && m_Sprites[i].TextureId == m_Sprites[i + 1].TextureId)
		{
			++batchSize;
			continue;
		}

		//Set Texture
		const auto texData = m_Textures[m_Sprites[i].TextureId];
		m_pEVar_TextureSRV->SetResource(texData->GetShaderResourceView());

		//Set Texture Size
		auto texSize = texData->GetDimension();
		m_pEVar_TextureSize->SetFloatVector(&texSize.x);

		//Set Transform
		m_pEVar_TransformMatrix->SetMatrix(&m_Transform._11);

		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pTechnique->GetDesc(&techDesc);
		for (unsigned int j = 0; j < techDesc.Passes; ++j)
		{
			m_pTechnique->GetPassByIndex(j)->Apply(0, pDeviceContext);
			pDeviceContext->Draw(batchSize, batchOffset);
		}

		batchOffset += batchSize;
		batchSize = 1;
	}

	m_Sprites.clear();
	m_Textures.clear();
}

void SpriteRenderer::AppendSprite(TextureData* pTexture, const XMFLOAT2& position, const XMFLOAT4& color, const XMFLOAT2& pivot, const XMFLOAT2& scale, float rotation, float depth)
{
	VertexSprite vertex{};

	const auto it = std::ranges::find(m_Textures, pTexture);

	if (it == m_Textures.end())
	{
		m_Textures.push_back(pTexture);
		vertex.TextureId = UINT(m_Textures.size()) - 1;
	}
	else
	{
		vertex.TextureId = UINT(it - m_Textures.begin());
	}

	vertex.TransformData = XMFLOAT4(position.x, position.y, depth, rotation);
	vertex.TransformData2 = XMFLOAT4(pivot.x, pivot.y, scale.x, scale.y);
	vertex.Color = color;

	m_Sprites.push_back(vertex);
}

void SpriteRenderer::DrawImmediate(const D3D11Context& d3dContext, ID3D11ShaderResourceView* pSrv, const XMFLOAT2& position, const XMFLOAT4& color, const XMFLOAT2& pivot, const XMFLOAT2& scale, float rotation)
{
	//Create Immediate VB
	if (!m_pImmediateVertexBuffer)
	{
		//Create a new buffer
		D3D11_BUFFER_DESC buffDesc;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		buffDesc.ByteWidth = sizeof(VertexSprite);
		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;

		HANDLE_ERROR(d3dContext.pDevice->CreateBuffer(&buffDesc, nullptr, &m_pImmediateVertexBuffer));
	}

	//Map Vertex
	VertexSprite vertex{};
	vertex.TextureId = 0;
	vertex.TransformData = XMFLOAT4(position.x, position.y, 0, rotation);
	vertex.TransformData2 = XMFLOAT4(pivot.x, pivot.y, scale.x, scale.y);
	vertex.Color = color;

	if (m_pImmediateVertexBuffer && !m_ImmediateVertex.Equals(vertex))
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		d3dContext.pDeviceContext->Map(m_pImmediateVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
		memcpy(mappedResource.pData, &vertex, sizeof(VertexSprite));
		d3dContext.pDeviceContext->Unmap(m_pImmediateVertexBuffer, 0);
		m_ImmediateVertex = vertex;
	}

	//Set Render Pipeline
	d3dContext.pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	unsigned int stride = sizeof(VertexSprite);
	unsigned int offset = 0;
	d3dContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pImmediateVertexBuffer, &stride, &offset);
	d3dContext.pDeviceContext->IASetInputLayout(m_pInputLayout);

	//Set Texture
	m_pEVar_TextureSRV->SetResource(pSrv);

	ID3D11Resource* pResource{};
	pSrv->GetResource(&pResource);

	D3D11_TEXTURE2D_DESC texDesc{};
	const auto texResource = reinterpret_cast<ID3D11Texture2D*>(pResource);
	texResource->GetDesc(&texDesc);
	texResource->Release();

	//Set Texture Size
	auto texSize = XMFLOAT2(static_cast<float>(texDesc.Width), static_cast<float>(texDesc.Height));
	m_pEVar_TextureSize->SetFloatVector(&texSize.x);

	//Set Transform
	m_pEVar_TransformMatrix->SetMatrix(&m_Transform._11);

	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pTechnique->GetDesc(&techDesc);
	for (unsigned int i = 0; i < techDesc.Passes; ++i)
	{
		m_pTechnique->GetPassByIndex(i)->Apply(0, d3dContext.pDeviceContext);
		d3dContext.pDeviceContext->Draw(1, 0);
	}
}
