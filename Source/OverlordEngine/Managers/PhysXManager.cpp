#include "stdafx.h"
#include "PhysXManager.h"

#define PVD_HOST "127.0.0.1"
#define PVD_PORT 5425

void PhysXManager::Initialize()
{
	//WIN32 Allocator Implementation
	m_pDefaultAllocator = new PhysxAllocator();
	m_pDefaultErrorCallback = new PhysxErrorCallback();

	//Create Foundation
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *m_pDefaultAllocator, *m_pDefaultErrorCallback);
	ASSERT_NULL(m_pFoundation, L"Foundation creation failed!");

#ifdef _DEBUG
	m_pPvd = PxCreatePvd(*m_pFoundation);
	if (!m_pPvd)
	{
		Logger::LogWarning(L"Physx: PhysX Visual Debugger (PVD) creation failed! (PVD connection will not be possible)");
	}
#endif

	//TODO: Fix CudaContextManager for PhysX 4.1
	////Cude context manager
	//PxCudaContextManagerDesc cudaContextManDesc = PxCudaContextManagerDesc();
	//cudaContextManDesc.interopMode = PxCudaInteropMode::D3D11_INTEROP;
	//cudaContextManDesc.graphicsDevice = pDevice;

	//m_pCudaContextManager = PxCreateCudaContextManager(*m_pFoundation, cudaContextManDesc, m_pProfileZoneManager);
	//if (m_pCudaContextManager)
	//{
	//	if (!m_pCudaContextManager->contextIsValid())
	//	{
	//		m_pCudaContextManager->release();
	//		m_pCudaContextManager = nullptr;
	//	}
	//	else
	//	{
	//		auto deviceName =std::string(m_pCudaContextManager->getDeviceName());
	//		Logger::LogFormat(LogLevel::Info, L"PhysXManager> Using Cuda Context Manager [%s]", std::wstring(deviceName.begin(), deviceName.end()).c_str());
	//	}
	//}

	//Create Main Physics Object
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), true, m_pPvd);
	ASSERT_NULL(m_pPhysics, L"Physx: Physics Object creation failed!");

	PxInitExtensions(*m_pPhysics, m_pPvd);

	//Create Default CpuDispatcher (2 threads)
	m_pDefaultCpuDispatcher = PxDefaultCpuDispatcherCreate(2);

	//Try to connect with the PVD
	ToggleVisualDebuggerConnection();
} 

PhysXManager::~PhysXManager()
{
	if (m_pPhysics)
	{
		PxCloseExtensions();
	}

	if (m_pPvd && m_pPvd->getTransport())
	{
		if (m_pPvd->isConnected())
		{
			m_pPvd->getTransport()->flush();
			m_pPvd->getTransport()->disconnect();
			m_pPvd->disconnect();
		}

		m_pPvd->getTransport()->release();
	}

	PxSafeRelease(m_pPhysics);
	PxSafeRelease(m_pPvd);
	PxSafeRelease(m_pCudaContextManager);
	PxSafeRelease(m_pDefaultCpuDispatcher);
	PxSafeRelease(m_pFoundation);

	SafeDelete(m_pDefaultAllocator);
	SafeDelete(m_pDefaultErrorCallback);
}

PxScene* PhysXManager::CreateScene(GameScene* pScene) const
{
	auto sceneDesc = PxSceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.setToDefault(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_pDefaultCpuDispatcher;
	sceneDesc.cudaContextManager = m_pCudaContextManager;
	sceneDesc.filterShader = OverlordSimulationFilterShader;
	sceneDesc.userData = pScene;

	const auto physxScene = m_pPhysics->createScene(sceneDesc);
	ASSERT_IF(physxScene == nullptr, L"Scene creation failed!")

	return physxScene;
}

bool PhysXManager::ToggleVisualDebuggerConnection() const
{
	if (!m_pPhysics || !m_pPvd) return false;

	if (m_pPvd->isConnected())
	{
		//DISCONNECT
		m_pPvd->disconnect();
		return false;
	}

	//CONNECT
	PxPvdTransport* pTransport = m_pPvd->getTransport();
	if (!pTransport)
	{
		pTransport = PxDefaultPvdSocketTransportCreate(PVD_HOST, PVD_PORT, 10);
	}

	if (m_pPvd->connect(*pTransport, PxPvdInstrumentationFlag::eALL))
	{
		return true;
	}

	return false;
}