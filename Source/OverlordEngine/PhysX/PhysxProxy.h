#pragma once
#include "Base/Structs.h"
class GameScene;

class PhysxProxy final: public PxSimulationEventCallback
{
public:
	PhysxProxy() = default;
	~PhysxProxy() override;

	PhysxProxy(const PhysxProxy& other) = delete;
	PhysxProxy(PhysxProxy&& other) noexcept = delete;
	PhysxProxy& operator=(const PhysxProxy& other) = delete;
	PhysxProxy& operator=(PhysxProxy&& other) noexcept = delete;

	PxScene* GetPhysxScene() const { return m_pPhysxScene; }

	void AddActor(PxActor& actor) const { if(m_pPhysxScene)m_pPhysxScene->addActor(actor); }

	PxControllerManager* GetControllerManager() const { return m_pControllerManager; }

	void EnablePhysxDebugRendering(bool enable) { m_DrawPhysx = enable; }

	static void EnablePhysXFrameStepping(bool enable) { m_PhysXFrameStepping = enable; }
	static void NextPhysXFrame(float time = 0.03f) { m_PhysXStepTime = time; }
	void Initialize(GameScene* pParent);
	void Update(const SceneContext& sceneContext) const;
	void Draw(const SceneContext& sceneContext) const;
	bool Raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal distance,
	             PxRaycastCallback& hitCall,
	             PxHitFlags hitFlags = PxHitFlags(PxHitFlag::eDEFAULT),
	             const PxQueryFilterData& filterData = PxQueryFilterData(),
	             PxQueryFilterCallback* filterCall = nullptr,
	             const PxQueryCache* cache = nullptr) const;

private:
	void onConstraintBreak(PxConstraintInfo* /*constraints*/, PxU32 /*count*/) override {};
	void onWake(PxActor** /*actors*/, PxU32 /*count*/) override {};
	void onSleep(PxActor** /*actors*/, PxU32 /*count*/) override {};
	void onContact(const PxContactPairHeader& /*pairHeader*/, const PxContactPair* /*pairs*/, PxU32 /*nbPairs*/) override {};
	void onAdvance(const PxRigidBody* const* /*bodyBuffer*/, const PxTransform* /*poseBuffer*/, const PxU32 /*count*/) override {};
	void onTrigger(PxTriggerPair* pairs, PxU32 count) override;

	PxScene* m_pPhysxScene{};
	PxControllerManager* m_pControllerManager{};
	bool m_DrawPhysx{};
	bool m_IsInitialized{};

	//Static debug variables
	static bool m_PhysXFrameStepping;
	static float m_PhysXStepTime;
};
