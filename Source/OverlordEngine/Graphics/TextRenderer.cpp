#include "stdafx.h"
#include "TextRenderer.h"

TextRenderer::~TextRenderer()
{
	SafeRelease(m_pInputLayout)
	SafeRelease(m_pVertexBuffer)
}

void TextRenderer::Initialize()
{
	//TODO_W5(L"Complete TextRenderer.fx")

	//Effect
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"Effects/TextRenderer.fx");
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

void TextRenderer::DrawText(SpriteFont* pFont, const std::wstring& text, const XMFLOAT2& position,
                            const XMFLOAT4& color)
{
	//skip if alpha is near 0
	if (color.w <= 0.0001f)
		return;

	if(!m_TextRenderGroups.contains(pFont))
	{
		m_TextRenderGroups.insert(std::make_pair(pFont, TextRenderGroup{}));
	}

	auto& renderGroup = m_TextRenderGroups.at(pFont);
	renderGroup.m_TextCaches.emplace_back(text, position, color);

	m_TotalCharacters += UINT(text.size());
}

void TextRenderer::Draw(const SceneContext& sceneContext)
{
	if (m_TextRenderGroups.empty())
		return;

	//Refresh Dynamic Vertex Buffer
	UpdateBuffer();

	//Set Render Pipeline
	const auto pDeviceContext = sceneContext.d3dContext.pDeviceContext;

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	constexpr UINT stride = sizeof(VertexText);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	for(const auto& pair : m_TextRenderGroups)
	{
		//Set Texture
		m_pEVar_TextureSRV->SetResource(pair.first->GetTexture()->GetShaderResourceView());

		//Set TextureSize
		m_pEVar_TextureSize->SetFloatVector(&pair.first->GetTextureSize().x);

		//Set Transform
		m_pEVar_TransformMatrix->SetMatrix(&m_Transform._11);

		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pTechnique->GetDesc(&techDesc);
		for(UINT i = 0; i < techDesc.Passes; ++i)
		{
			m_pTechnique->GetPassByIndex(i)->Apply(0, pDeviceContext);
			pDeviceContext->Draw(pair.second.bufferSize, pair.second.bufferStart);
		}
	}

	m_TextRenderGroups.clear();
}

void TextRenderer::UpdateBuffer()
{
	if (!m_pVertexBuffer || m_TotalCharacters > m_BufferSize)
	{
		//Release Buffer if it exists
		SafeRelease(m_pVertexBuffer);

		//Set new buffersize if needed
		if (m_TotalCharacters > m_BufferSize)
			m_BufferSize = m_TotalCharacters;

		//Create Dynamic Buffer
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = sizeof(VertexText) * m_BufferSize;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;

		HANDLE_ERROR(m_GameContext.d3dContext.pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pVertexBuffer));
	}

	//Refresh Buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	_Analysis_assume_(m_pVertexBuffer != nullptr);
	m_GameContext.d3dContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	int bufferPosition{ 0 };
	const auto pBuffer = static_cast<VertexText*>(mappedResource.pData);

	for (auto& pair : m_TextRenderGroups)
	{
		auto& renderGroup = pair.second;
		renderGroup.bufferStart = bufferPosition;
		for (const auto& textCache : renderGroup.m_TextCaches)
		{
			CreateTextVertices(pair.first, textCache, pBuffer, bufferPosition);
		}

		renderGroup.bufferSize = bufferPosition - renderGroup.bufferStart;
		renderGroup.m_TextCaches.clear();
	}

	m_GameContext.d3dContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
	m_TotalCharacters = 0;
}

void TextRenderer::CreateTextVertices(SpriteFont* pFont, const TextCache& textCache, VertexText* pBuffer, int& bufferPosition)
{
	int totalAdvanceX{ 0 };
	for(const wchar_t& character: textCache.text)
	{
		if (!pFont->HasMetric(character))
		{
			Logger::LogError(L"Character \'{}\' not supported by SpriteFont \'{}\' (size {})", character, pFont->GetName(), pFont->GetSize());
			continue;
		}

		const auto& metric = pFont->GetMetric(character);

		if(character == L' ')
		{
			totalAdvanceX += metric.advanceX;
			continue;
		}

		VertexText vertex;
		vertex.position.x = textCache.position.x + float(totalAdvanceX + metric.offsetX);
		vertex.position.y = textCache.position.y + float(metric.offsetY);
		vertex.position.z = .9f;
		vertex.color = textCache.color;
		vertex.texCoord = metric.texCoord;
		vertex.characterDimension = { float(metric.width), float(metric.height) };
		vertex.channelId = metric.channel;

		pBuffer[bufferPosition] = vertex;
		++bufferPosition;

		totalAdvanceX += metric.advanceX;
	}
}
