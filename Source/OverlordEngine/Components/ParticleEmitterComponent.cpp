#include "stdafx.h"
#include "ParticleEmitterComponent.h"
#include "Misc/ParticleMaterial.h"

ParticleMaterial* ParticleEmitterComponent::m_pParticleMaterial{};

ParticleEmitterComponent::ParticleEmitterComponent(const std::wstring& assetFile, const ParticleEmitterSettings& emitterSettings,
	UINT particleCount, bool startSpawning, bool inCircle, float initialCircleRadius):
	m_ParticlesArray(new Particle[particleCount]),
	m_ParticleCount(particleCount), //How big is our particle buffer?
	m_MaxParticles(particleCount), //How many particles to draw (max == particleCount)
	m_AssetFile(assetFile),
	m_EmitterSettings(emitterSettings),
	m_Spawning(startSpawning),
	m_InCircle(inCircle),
	m_InitialCircleRadius(initialCircleRadius)
{
	m_enablePostDraw = true; //This enables the PostDraw function for the component
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	//TODO_W9(L"Implement Destructor")

	//Delete the Particle Pool
	delete[] m_ParticlesArray;
	m_ParticlesArray = nullptr;

	//Release the VertexBuffer
	SafeRelease(m_pVertexBuffer);
}

void ParticleEmitterComponent::Initialize(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement Initialize")

	//Use the MaterialManager to create an instance of the ParticleMaterial and store it in m_pParticleMaterial
	//Note that this member is STATIC, so only create an instance if it isn’t created yet
	//This material is shared across all ParticleEmitterComponents
	if(m_pParticleMaterial == nullptr)
		m_pParticleMaterial = MaterialManager::Get()->CreateMaterial<ParticleMaterial>();

	//Call CreateVertexBuffer()
	CreateVertexBuffer(sceneContext);

	//Use the ContentManager to load the particle texture and store it in m_pParticleTexture
	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);

}

void ParticleEmitterComponent::CreateVertexBuffer(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement CreateVertexBuffer")

	//If m_pVertexBuffer exists, release it
	if (m_pVertexBuffer)
		SafeRelease(m_pVertexBuffer);

	//Create a dynamic vertexbuffer
	D3D11_BUFFER_DESC bufferDesc{};
	//	Set the usage to Dynamic
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//	Use the ParticleCount and size of a VertexParticle to calculate the ByteWidth
	bufferDesc.ByteWidth = m_ParticleCount * sizeof(VertexParticle);
	//	This is a VertexBuffer, select the appropriate BindFlag
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//	We want to edit the buffer at runtime, so we need CPU write access. Select the appropriate CPUAccessFlag
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//	There are no MiscFlags(0)
	bufferDesc.MiscFlags = 0;
	//	Create the VertexBuffer(GraphicsDevice > ...)
	sceneContext.d3dContext.pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pVertexBuffer);
	//	Don’t forget to check for errors!(HANDLE_ERROR)
	if (m_pVertexBuffer == nullptr)
		HANDLE_ERROR(L"ParticleEmitterComponent::CreateVertexBuffer -> The vertex buffer was not initialized");
}

void ParticleEmitterComponent::Update(const SceneContext& sceneContext)
{
	if (m_Paused == false)
	{
		//TODO_W9(L"Implement Update")

		//Create a local variable, called particleInterval of type float
		//This variable needs to contain the average particle emit threshold
		//We know the average energy of a particle(maxEnergy vs minEnergy) and the number of particles
		//With those values we can calculate the value for particleInterval
		const auto particleInterval = (m_EmitterSettings.maxEnergy - m_EmitterSettings.minEnergy) / m_ParticleCount;


		//Increase m_LastParticleInit by the elapsed GameTime.This value keeps track of the last particle spawn
		m_LastParticleSpawn += sceneContext.pGameTime->GetElapsed();


		//Time to validate the particles and add some of them to the VertexBuffer
		//	Set m_ActiveParticles to zero
		m_ActiveParticles = 0;

		//	Use DeviceContext::Map to map our vertexbuffer, this method retrieves an element of type
		//	D3D11_MAPPED_SUBRESOURCE, which contains a pointer to the first element in our vertexbuffer
		//	Between the ‘Map’and ‘Unmap’ method we can update the VertexBuffer data
		//		We want to override the current data in the vertexbuffer, so we can discard the previous data (D3D11_MAP_WRITE_DISCARD)
		//		There are no mapflags(0)
		//		The last parameter is of type D3D11_MAPPED_SUBRESOURCE which contains a pointer(pData)
		//		to the first VertexParticle element in the buffer
		auto pMappedResource = new D3D11_MAPPED_SUBRESOURCE{};
		sceneContext.d3dContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, pMappedResource);

		//	Create an empty pointer of type VertexParticle(ParticleVertex * pBuffer) and cast & assign pData to it
		auto pBuffer = static_cast<VertexParticle*>(pMappedResource->pData);

		//	The VertexBuffer is mapped, and we have access to write new data to it
		//	Iterate the Particle Array(You only want to iterate ‘m_ParticleCount’ number of times)
		const auto elapsedTime = sceneContext.pGameTime->GetElapsed();
		//	For every particle :
		for (UINT i = 0; i < m_ParticleCount; i++)
		{
			// If the particle is currently Active > UpdateParticle function. (Hint this call needs the elapsedTime,
				// cache this value outside the loop because it’s the same for every´particle)
			if (m_ParticlesArray[i].isActive)
				UpdateParticle(m_ParticlesArray[i], elapsedTime);

			// If the particle is NOT Active(do not use ‘else if’), and the value of m_LastParticleInit is bigger than
			// or equal to particleInterval > Call SpawnParticle. Reason for not using ‘else if’ is that a particle can
			// become inactive during the update cycle, meaning we can immediately respawn that particle
			if (m_ParticlesArray[i].isActive == false && m_LastParticleSpawn >= particleInterval && m_Spawning)
			{
				if (m_InCircle)
				{
					const auto angle = (360.f / m_MaxParticles) * (i + 1);
					const auto newDirection = XMFLOAT3{ cos(angle), 0.f, sin(angle) };
					const auto newVelocity = XMFLOAT3{ newDirection.x * m_EmitterSettings.velocity.x, m_EmitterSettings.velocity.y, newDirection.z * m_EmitterSettings.velocity.z };
					const auto newPosVec = XMLoadFloat3(&m_pGameObject->GetTransform()->GetPosition()) + XMLoadFloat3(&newDirection) * m_InitialCircleRadius;
					XMFLOAT3 newPosition;
					XMStoreFloat3(&newPosition, newPosVec);
					SpawnParticle(m_ParticlesArray[i], newVelocity, newPosition);
				}
				else
				{
					SpawnParticle(m_ParticlesArray[i], m_EmitterSettings.velocity, m_pGameObject->GetTransform()->GetPosition());
				}
			}

			// If(after potential update and /or spawn) the particle is Active, add the particle to the VertexBuffer,
			// use m_ActiveParticles as index. After that we increase m_ActiveParticles by one
			if (m_ParticlesArray[i].isActive)
			{
				pBuffer[m_ActiveParticles] = m_ParticlesArray[i].vertexInfo;
				m_ActiveParticles++;
			}
		}

		//	Use DeviceContext::Unmap to unmap our vertexbuffer.
		sceneContext.d3dContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);

		delete(pMappedResource);
	}
}

void ParticleEmitterComponent::UpdateParticle(Particle& p, float elapsedTime) const
{
	//TODO_W9(L"Implement UpdateParticle")

	//Check if the particle is active, if not, return
	if (p.isActive == false)
		return;

	//Subtract the elapsedTime from the particle’s currentEnergy.
	p.currentEnergy -= elapsedTime;

	//If currentEnergy is smaller than ZERO, deactivate the particle and return
	if(p.currentEnergy < 0.f)
	{
		p.isActive = false;
		return;
	}

	//The update method will update all the variables of our VertexParticle parameter

	//	a.vertexInfo.Position
	//		i.Add the velocity(m_EmitterSettings) multiplied by the elapsedTime, this way our
	//		  particle moves in the direction of the velocity defined by the emitter settings.
	const auto newPosVec = XMLoadFloat3(&p.vertexInfo.Position) + XMLoadFloat3(&p.velocity) * elapsedTime;
	XMFLOAT3 newPos{};
	XMStoreFloat3(&newPos, newPosVec);
	p.vertexInfo.Position = newPos;

	//Create a local variable, called ‘lifePercent' of type float, this is the percentual particle lifetime.This
	//value can be obtained by dividing the particle’s ‘currentEnergy’ by its ‘totalEnergy’
	//[At start : lifePercent = 1, At end : lifePercent = 0]
	const float lifePercent = p.currentEnergy / p.totalEnergy;

	//	b.vertexInfo.Color
	//		i.Our color equals the color given by the emitter settings
	//		ii.The alpha value of the particle color should fade out over time.Use the initial
	//		   alpha value(m_EmitterSettings.Color.w) multiplied with ‘lifePercent’ additionally
	//		   you can multiply with an extra constant(like 2) to delay the fade out effect.
	p.vertexInfo.Color = m_EmitterSettings.color;
	p.vertexInfo.Color.w = m_EmitterSettings.color.w * lifePercent;

	//	c.vertexInfo.Size
	//		i.Based on the particle’s sizeChange value, our particle shrinks or grows over time.
	//		ii.If sizeChange is smaller than 1 (Shrink)
	//			1. Example: 0.5 > half its initial size at the end of its life
	//			2. Calculate the value for vertexInfo.Size using the Initial size(initialSize),
	//			   the size grow(sizeChange) and the particle’s life percentage(lifePercent).
	//			   [Use a sheet of paper to create a visual representation]
	//		iii.If sizeChange is bigger than 1 (Grow)
	//			1. Example: 2 > double its initial size at the end of its life
	p.vertexInfo.Size = p.initialSize + p.initialSize * p.sizeChange * (1-lifePercent);
}

void ParticleEmitterComponent::SpawnParticle(Particle& p, XMFLOAT3 velocity, XMFLOAT3 position)
{
	//TODO_W9(L"Implement SpawnParticle")

	m_LastParticleSpawn = 0.f;

	//Set particle’s isActive to true
	p.isActive = true;

	//Energy Initialization
	//	a.totalEnergy and currentEnergy of the particle are both equal to a random float between
	//	minEnergy and maxEnergy (see m_EmitterSettings) [Random float > MathHelper::RandF(...)]
	p.totalEnergy = MathHelper::randF(m_EmitterSettings.minEnergy, m_EmitterSettings.maxEnergy);
	p.currentEnergy = p.totalEnergy;

	//Position Initialization
	//	a.We need to calculate a random position; this position is determined by the emitter radius
	//	  of our particle system.But before we can talk about a radius, we need a random direction.
	//	b.There are several ways to calculate a random directionand the following is one of the
	//	  most straightforward way to do it:
	//		i.We start by defining a unit vector. (randomDirection = (1, 0, 0))
	const auto unitFloat = XMFLOAT3(1, 0, 0);
	auto unitVect = XMLoadFloat3(&unitFloat);
	//		ii.We are going to rotate this unit vector using a random generated rotation matrix,
	//		   this way we obtain a random normalized vector.Use the XMMatrixRotationRollPitchYaw(...)
	//		   to create a random rotation matrix(called randomRotationMatrix).
	//		   (use RandF(-XM_PI, XM_PI) to generate values for Yaw, Pitchand Roll).
	const auto randomRotationMatrix = XMMatrixRotationRollPitchYaw(MathHelper::randF(-XM_PI, XM_PI),
		MathHelper::randF(-XM_PI, XM_PI), MathHelper::randF(-XM_PI, XM_PI));
	//		iii.Now we need to transform our randomDirection vector with our randomRotationMatrix.
	//			(XMVector3TransformNormal).After this step we created a random normalized vector.
	const auto randomDir = XMVector3TransformNormal(unitVect, randomRotationMatrix);
	//	c.We’ve already got our direction; the second step is calculating the distance of our particle
	//	  starting point.The distance is determined by the minEmitterRange and the
	//	  maxEmitterRange(see EmitterSettings).Make sure it is a random value that lays between those two bounds
	const auto randomDist = MathHelper::randF(m_EmitterSettings.minEmitterRadius, m_EmitterSettings.maxEmitterRadius);
	//	d.Everything is in place to calculate the initial position
	//		i.vertexInfo.Position = ‘our random direction’ * ‘our random distance’
	const auto finalPos = XMLoadFloat3(&position) + randomDir * randomDist;
	XMStoreFloat3(&p.vertexInfo.Position, finalPos);

	//Size Initialization
	//	a.Our vertexInfo.Size and initialSize are both equal to a random value that lays between
	//	  MinSize and MaxSize(see EmitterSettings)
	p.vertexInfo.Size = MathHelper::randF(m_EmitterSettings.minSize, m_EmitterSettings.maxSize);
	p.initialSize = p.vertexInfo.Size;
	//	b.sizeChange is equal to a random value that lays between minScale and maxScale(see EmitterSettings)
	p.sizeChange = MathHelper::randF(m_EmitterSettings.minScale, m_EmitterSettings.maxScale);

	//Rotation Initialization
	//	a.The rotation(vertexInfo.Rotation) is a random value between –PI and PI.
	p.vertexInfo.Rotation = MathHelper::randF(-XM_PI, XM_PI);

	//Color Initialization
	//	a.The particle’s color(vertexInfo.Color) is equal to the color from the emitter settings.
	p.vertexInfo.Color = m_EmitterSettings.color;



	p.velocity = velocity;
}

void ParticleEmitterComponent::PostDraw(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement PostDraw")

	//Set the following shader variables to our ParticleMaterial(m_pParticleMaterial)
	//	a.gWorldViewProj > Camera ViewProjection(we don’t need to pass the actual WorldViewProjection.
	//	  Our particles already live in World - Space, so we don’t need to transform them again inside the shader.
	//	  That’s why we only need to pass the camera’s ViewProjection as WVP variable)
	m_pParticleMaterial->SetVariable_Matrix(L"gWorldViewProj", sceneContext.pCamera->GetViewProjection());
	//	b.gViewInverse > Camera ViewInverse
	m_pParticleMaterial->SetVariable_Matrix(L"gViewInverse", sceneContext.pCamera->GetViewInverse());
	//	c.gParticleTexture > m_pParticleTexture
	m_pParticleMaterial->SetVariable_Texture(L"gParticleTexture", m_pParticleTexture);

	//Retrieve the TechniqueContext from the material, this structure contains relevant information to
	//setup the pipeline(BaseMaterial::GetTechniqueContext)
	const auto techniqueContext = m_pParticleMaterial->GetTechniqueContext(0);
	
	//Set the InputLayout
	sceneContext.d3dContext.pDeviceContext->IASetInputLayout(techniqueContext.pInputLayout);
	
	//Set the PrimitiveTopology
	//	a.Remember we are only using points for our particles, so select the appropriate topology.
	sceneContext.d3dContext.pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	//Set the VertexBuffer
	//	a.We have no startslot and only one buffer
	//	b.We have no offset
	//	c.The size of one vertex(stride) is equal to the size of VertexParticle
	const unsigned int stride = sizeof(VertexParticle);
	const unsigned int offset = 0;
	sceneContext.d3dContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//For each pass of our technique
	//	a.Apply the pass
	//	b.Draw the vertices!(The number of vertices we want to draw is equal to m_ActiveParticles)
	D3DX11_TECHNIQUE_DESC techDesc{};
	techniqueContext.pTechnique->GetDesc(&techDesc);
	for (unsigned int i = 0; i < techDesc.Passes; ++i)
	{
		techniqueContext.pTechnique->GetPassByIndex(i)->Apply(0, sceneContext.d3dContext.pDeviceContext);
		sceneContext.d3dContext.pDeviceContext->Draw(m_ActiveParticles, 0);
	}
}

void ParticleEmitterComponent::DrawImGui()
{
	if(ImGui::CollapsingHeader("Particle System"))
	{
		ImGui::SliderUInt("Count", &m_ParticleCount, 0, m_MaxParticles);
		ImGui::InputFloatRange("Energy Bounds", &m_EmitterSettings.minEnergy, &m_EmitterSettings.maxEnergy);
		ImGui::InputFloatRange("Size Bounds", &m_EmitterSettings.minSize, &m_EmitterSettings.maxSize);
		ImGui::InputFloatRange("Scale Bounds", &m_EmitterSettings.minScale, &m_EmitterSettings.maxScale);
		ImGui::InputFloatRange("Radius Bounds", &m_EmitterSettings.minEmitterRadius, &m_EmitterSettings.maxEmitterRadius);
		ImGui::InputFloat3("Velocity", &m_EmitterSettings.velocity.x);
		ImGui::ColorEdit4("Color", &m_EmitterSettings.color.x, ImGuiColorEditFlags_NoInputs);
	}
}

void ParticleEmitterComponent::TogglePause(bool paused)
{
	m_Paused = paused;
}

void ParticleEmitterComponent::ToggleSpawning(bool spawning)
{
	m_Spawning = spawning;
}

