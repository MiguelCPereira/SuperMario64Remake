#include "stdafx.h"
#include "EffectLoader.h"

ID3DX11Effect* EffectLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	//If compiled effect
	const auto& assetPath = loadInfo.assetFullPath;
	if(assetPath.extension() == L".fxc")
	{
		ID3DX11Effect* pEffect;
		D3DX11CreateEffectFromFile(assetPath.c_str(),0, m_GameContext.d3dContext.pDevice, &pEffect);
		return pEffect;
	}


	HRESULT hr;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;
	
	DWORD shaderFlags = 0;
#if defined( _DEBUG )
    shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	hr = D3DX11CompileEffectFromFile(assetPath.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		m_GameContext.d3dContext.pDevice,
		&pEffect,
		&pErrorBlob);

	if(FAILED(hr))
	{
		std::wstringstream ss;
		if(pErrorBlob!=nullptr)
		{
			const char *errors = static_cast<char*>(pErrorBlob->GetBufferPointer());
 
			
			for(unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss<<errors[i];
 
			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;
		}
		else
		{
			ss<<"Failed to CreateEffectFromFile!\nPath: ";
			ss<< assetPath;
		}

		HANDLE_ERROR_INFO(hr, ss.str())
		return nullptr;
	}

	return pEffect;
}

void EffectLoader::Destroy(ID3DX11Effect* objToDestroy)
{
	SafeRelease(objToDestroy);
}
