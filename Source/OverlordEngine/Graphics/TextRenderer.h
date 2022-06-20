#pragma once
struct TextCache
{
	const std::wstring text{};
	XMFLOAT2 position{};
	XMFLOAT4 color{};
};

struct TextRenderGroup
{
	std::vector<TextCache> m_TextCaches{};
	UINT bufferStart{};
	UINT bufferSize{};
};

class TextRenderer final: public Singleton<TextRenderer>
{
public:
	TextRenderer(const TextRenderer& other) = delete;
	TextRenderer(TextRenderer&& other) noexcept = delete;
	TextRenderer& operator=(const TextRenderer& other) = delete;
	TextRenderer& operator=(TextRenderer&& other) noexcept = delete;

#undef DrawText
	void DrawText(SpriteFont* pFont, const std::wstring& text, const XMFLOAT2& position, const XMFLOAT4& color = XMFLOAT4{ Colors::White });
	void Draw(const SceneContext& sceneContext);

protected:
	void Initialize() override;

private:
	friend class Singleton<TextRenderer>;
	TextRenderer() = default;
	~TextRenderer();

	void UpdateBuffer();
	void CreateTextVertices(SpriteFont* pFont, const TextCache& textCache, VertexText* pBuffer, int& bufferPosition);

	XMFLOAT4X4 m_Transform{};
	ID3DX11Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pTechnique{};
	ID3DX11EffectMatrixVariable* m_pEVar_TransformMatrix{};
	ID3DX11EffectVectorVariable* m_pEVar_TextureSize{};
	ID3DX11EffectShaderResourceVariable* m_pEVar_TextureSRV{};

	ID3D11InputLayout* m_pInputLayout{};
	ID3D11Buffer* m_pVertexBuffer{};

	std::map<SpriteFont*, TextRenderGroup> m_TextRenderGroups{};

	UINT m_TotalCharacters{};
	UINT m_BufferSize{100};
};