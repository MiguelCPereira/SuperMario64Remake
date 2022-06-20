#include "stdafx.h"
#include "TextureData.h"

UUID TextureData::GUID_TextureData{0,0,0,0};

TextureData::TextureData(ID3D11Resource* pTexture, ID3D11ShaderResourceView* pTextureShaderResourceView, const std::wstring& assetSubPath):
	m_pTexture(pTexture),
	m_pTextureShaderResourceView(pTextureShaderResourceView),
	m_AssetSubPath{assetSubPath}
{
	const auto tex2D = static_cast<ID3D11Texture2D*>(pTexture);
	D3D11_TEXTURE2D_DESC text2Ddesc;
	tex2D->GetDesc(&text2Ddesc);

	m_Dimension = XMFLOAT2(static_cast<float>(text2Ddesc.Width), static_cast<float>(text2Ddesc.Height));

	void* thisPtr = (void*)this;
	m_pTextureShaderResourceView->SetPrivateData(GUID_TextureData, sizeof(thisPtr), &thisPtr);
}


TextureData::~TextureData()
{
	SafeRelease(m_pTexture);
	SafeRelease(m_pTextureShaderResourceView);
}

void TextureData::CreateGUID()
{
	if(GUID_TextureData.Data1 == 0)
		HANDLE_ERROR(CoCreateGuid(&GUID_TextureData));
}
