#pragma once
class TextureData
{
public:
	TextureData(ID3D11Resource* pTexture, ID3D11ShaderResourceView* pTextureShaderResourceView, const std::wstring& assetSubPath = L"");
	~TextureData();

	TextureData(const TextureData& other) = delete;
	TextureData(TextureData&& other) noexcept = delete;
	TextureData& operator=(const TextureData& other) = delete;
	TextureData& operator=(TextureData&& other) noexcept = delete;

	ID3D11Resource* GetResource() const { return m_pTexture; }
	ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pTextureShaderResourceView; }
	const XMFLOAT2& GetDimension() const { return m_Dimension; }
	const std::wstring& GetAssetSubPath() const { return m_AssetSubPath; }

	static void CreateGUID();
	static UUID GUID_TextureData;

private:
	ID3D11Resource *m_pTexture{};
	ID3D11ShaderResourceView *m_pTextureShaderResourceView{};
	XMFLOAT2 m_Dimension{};
	std::wstring m_AssetSubPath{};
};

