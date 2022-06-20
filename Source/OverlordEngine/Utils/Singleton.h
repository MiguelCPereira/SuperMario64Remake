#pragma once

template<class T>
class Singleton
{
public:

	static T* Create(const GameContext& gameContext)
	{
		if(!m_IsInitialized)
		{
			m_pInstance = new T();
			m_pInstance->m_GameContext = gameContext;
			m_pInstance->Initialize();
			m_IsInitialized = true;
		}

		return m_pInstance;
	}
	
	static T* Get()
	{
		if (!m_IsInitialized) {
			Logger::LogWarning(L"Singleton not yet initialized");
			return nullptr;
		}

		if(!m_pInstance)
		{
			Logger::LogWarning(L"Singleton is initialized but instance in NULL");
		}

		return m_pInstance;
	}
	
	static void Destroy()
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}

protected:
	virtual void Initialize() = 0;

	static bool m_IsInitialized;
	GameContext m_GameContext{};

private:
	static T* m_pInstance;
};

template<class T> 
T* Singleton<T>::m_pInstance{};

template<class T>
bool Singleton<T>::m_IsInitialized{};