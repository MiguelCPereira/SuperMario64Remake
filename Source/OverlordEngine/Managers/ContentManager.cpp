#include "stdafx.h"
#include "ContentManager.h"

std::vector<BaseLoader*> ContentManager::m_Loaders = std::vector<BaseLoader*>();
GameContext ContentManager::m_GameContext = {};
bool ContentManager::m_IsInitialized = false;

void ContentManager::Release()
{
	for(BaseLoader *ldr:m_Loaders)
	{	
		ldr->Unload();
		SafeDelete(ldr);
	}

	m_Loaders.clear();
}

void ContentManager::Initialize(const GameContext& gameContext)
{
	if(!m_IsInitialized)
	{
		m_GameContext = gameContext;
		m_IsInitialized = true;

		AddLoader(new EffectLoader);
		AddLoader(new MeshFilterLoader);
		AddLoader(new PxConvexMeshLoader);
		AddLoader(new PxTriangleMeshLoader);
		AddLoader(new TextureDataLoader);
		AddLoader(new SpriteFontLoader);
	}
}

void ContentManager::AddLoader(BaseLoader* loader)
{ 
	for(const BaseLoader *ldr:m_Loaders)
	{	
		if(ldr->GetType()==loader->GetType())
		{
			SafeDelete(loader);
			break;
		}
	}

	m_Loaders.push_back(loader);
	loader->Initialize(m_GameContext);
}

fs::path ContentManager::GetFullAssetPath(const std::wstring& assetSubPath)
{
	if(m_GameContext.contentRoot.empty())
		return fs::absolute(fs::path{ assetSubPath });

	return fs::absolute(fs::path{ m_GameContext.contentRoot }).append(assetSubPath);
}