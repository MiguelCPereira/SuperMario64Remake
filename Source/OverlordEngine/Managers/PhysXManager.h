#pragma once
class PhysxAllocator;
class PhysxErrorCallback;
class GameScene;
class OverlordGame;

class PhysXManager final : public Singleton<PhysXManager>
{
public:
	PhysXManager(const PhysXManager& other) = delete;
	PhysXManager(PhysXManager&& other) noexcept = delete;
	PhysXManager& operator=(const PhysXManager& other) = delete;
	PhysXManager& operator=(PhysXManager&& other) noexcept = delete;

	PxPhysics* GetPhysics() const { return m_pPhysics; }
	PxScene* CreateScene(GameScene* pScene) const;


	bool ToggleVisualDebuggerConnection() const;
	bool IsPvdConnected() const { return m_pPvd && m_pPvd->isConnected(); }

protected:
	void Initialize() override;

private:
	friend class Singleton<PhysXManager>;
	PhysXManager() = default;
	~PhysXManager();

	PhysxAllocator* m_pDefaultAllocator{};
	PhysxErrorCallback* m_pDefaultErrorCallback{};
	PxFoundation* m_pFoundation{};
	PxPhysics* m_pPhysics{};
	PxPvd* m_pPvd{};
	PxDefaultCpuDispatcher* m_pDefaultCpuDispatcher{};
	PxCudaContextManager* m_pCudaContextManager{};
};
