#include "stdafx.h"
#include "TextureDataLoader.h"

TextureData* TextureDataLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	ID3D11Resource* pTexture{};
	ID3D11ShaderResourceView* pShaderResourceView{};
	TexMetadata info{};

	auto image = new ScratchImage();
	const auto& assetPath = loadInfo.assetFullPath;

	//Find Extension
	ASSERT_IF(!assetPath.has_extension(), L"Invalid File Extensions!\nPath: {}", assetPath.wstring())

	const auto extension = assetPath.extension().wstring();

	if (extension == L".dds")
	//if (lstrcmpiW(extension.c_str(), L"dds") == 0) //DDS Loader
	{
		HANDLE_ERROR(LoadFromDDSFile(assetPath.c_str(), DirectX::DDS_FLAGS_NONE, &info, *image));
	}
	else if (extension == L".tga")
	//else if (lstrcmpiW(extension.c_str(), L"tga") == 0) //TGA Loader
	{
		HANDLE_ERROR(LoadFromTGAFile(assetPath.c_str(), &info, *image));
	}
	else //WIC Loader
	{
		HANDLE_ERROR(LoadFromWICFile(assetPath.c_str(), DirectX::WIC_FLAGS_NONE, &info, *image));
	}
	

	HANDLE_ERROR(CreateTexture(m_GameContext.d3dContext.pDevice, image->GetImages(), image->GetImageCount(),image->GetMetadata(), &pTexture));
	HANDLE_ERROR(CreateShaderResourceView(m_GameContext.d3dContext.pDevice, image->GetImages(), image->GetImageCount(), image->GetMetadata(), &pShaderResourceView));

	SafeDelete(image);
	return new TextureData(pTexture, pShaderResourceView, loadInfo.assetSubPath);
}

void TextureDataLoader::Destroy(TextureData* objToDestroy)
{
	SafeDelete(objToDestroy);
}
