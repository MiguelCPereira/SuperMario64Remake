#pragma once

//Overlord Engine Headers
#include "Utils/Macros.h"

#include "Utils/BinaryReader.h"
#include "Utils/Utils.h"
#include "Utils/Singleton.h"

#include "Utils/EffectHelper.h"
#include "Utils/ImguiHelper.h"
#include "Utils/MathHelper.h"
#include "Utils/PhysxHelper.h"
#include "Utils/VertexHelper.h"

#include "Base/Structs.h"
#include "Base/Enumerations.h"
#include "Base/OverlordGame.h"
#include "Base/GameTime.h"
#include "Base/GameStats.h"
#include "Base/Logger.h"

#include "Managers/ContentManager.h"
#include "Managers/InputManager.h"
#include "Managers/LightManager.h"
#include "Managers/MaterialManager.h"
#include "Managers/SceneManager.h"
#include "Managers/SoundManager.h"
#include "Managers/PhysXManager.h"

#include "Components/BaseComponent.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "Components/MeshDrawComponent.h"
#include "Components/MeshIndexedDrawComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/ModelComponent.h"
#include "Components/SpriteComponent.h" //Week 4
#include "Components/ControllerComponent.h" //Week 5
#include "Components/ParticleEmitterComponent.h" //Week 9

#include "Content/ContentLoader.h"
#include "Content/EffectLoader.h"
#include "Content/MeshFilterLoader.h"
#include "Content/PxMeshLoader.h"
#include "Content/SpriteFontLoader.h"
#include "Content/TextureDataLoader.h"

#include "Graphics/ShadowMapRenderer.h" //Week 8
#include "Graphics/DebugRenderer.h"
#include "Graphics/SpriteRenderer.h" //Week 4
#include "Graphics/TextRenderer.h" //Week 5

#include "Misc/BaseMaterial.h"
#include "Misc/Material.h"
#include "Misc/MeshFilter.h"
#include "Misc/ModelAnimator.h" //Week 7
#include "Misc/RenderTarget.h"
#include "Misc/SpriteFont.h" //Week 4
#include "Misc/TextureData.h"
#include "Misc/PostProcessingMaterial.h" //Week 10

#include "PhysX/OverlordSimulationFilterShader.h"
#include "PhysX/PhysxAllocator.h"
#include "PhysX/PhysxErrorCallback.h"
#include "PhysX/PhysxProxy.h"

#include "Scenegraph/GameObject.h"
#include "SceneGraph/GameScene.h"

#include "Prefabs/FixedCamera.h"
#include "Prefabs/FreeCamera.h"