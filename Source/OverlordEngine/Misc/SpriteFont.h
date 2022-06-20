#pragma once
#include "Misc/TextureData.h"

#pragma region Helper Structs
struct FontMetric
{
	wchar_t character{};
	unsigned short width{};
	unsigned short height{};
	short offsetX{};
	short offsetY{};
	short advanceX{};
	unsigned char page{};
	unsigned char channel{};
	XMFLOAT2 texCoord{};
};

struct SpriteFontDesc
{
	std::wstring fontName{};
	short fontSize{};

	short textureWidth{};
	short textureHeight{};
	TextureData* pTexture{};

	std::unordered_map<wchar_t, FontMetric> metrics{};
};
#pragma endregion

class SpriteFont final
{
public:
	SpriteFont(const SpriteFontDesc& fontDesc);
	~SpriteFont() = default;
	SpriteFont(const SpriteFont& other) = delete;
	SpriteFont(SpriteFont&& other) noexcept = delete;
	SpriteFont& operator=(const SpriteFont& other) = delete;
	SpriteFont& operator=(SpriteFont&& other) noexcept = delete;

	TextureData* GetTexture() const { return m_FontDesc.pTexture; }
	const XMFLOAT2& GetTextureSize() const { return m_FontDesc.pTexture->GetDimension(); }
	const std::wstring& GetName() const { return m_FontDesc.fontName; }
	short GetSize() const { return m_FontDesc.fontSize; }
	bool HasMetric(const wchar_t& character) const { return m_FontDesc.metrics.contains(character); };
	const FontMetric& GetMetric(const wchar_t& character) const { return m_FontDesc.metrics.at(character); };

private:
	SpriteFontDesc m_FontDesc;
};

