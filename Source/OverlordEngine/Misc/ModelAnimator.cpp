#include "stdafx.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter):
	m_pMeshFilter{pMeshFilter}
{
	SetAnimation(0);
}

void ModelAnimator::Update(const SceneContext& sceneContext)
{
	//TODO_W7_();

	// We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		// 1. 
		// Calculate the passedTicks (see the lab document)
		auto passedTicks = sceneContext.pGameTime->GetElapsed() * m_CurrentClip.ticksPerSecond * m_AnimationSpeed;
		// Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		passedTicks = fmod(passedTicks, m_CurrentClip.duration);

		// 2. 
		if (m_Reversed) // IF m_Reversed is true
		{
			// Subtract passedTicks from m_TickCount
			m_TickCount -= passedTicks;
			// If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
			// (I added an extra multiplication, in case of incredibly slow hardware)
			// ((Otherwise, if a pc was slow enough for the entire clip duration to pass by in between updates,
			// the m_TickCount would decrease so much that simply adding the duration once wouldn't be enough to make it positive))
			if (m_OneShot && m_TickCount <= -m_CurrentClip.duration + 1.f)
			{
				m_IsPlaying = false;
				return;
			}

			if (m_TickCount < 0)
				m_TickCount += m_CurrentClip.duration * (1 + int( -m_TickCount / m_CurrentClip.duration));
		}
		else // ELSE
		{
			// Add passedTicks to m_TickCount
			m_TickCount += passedTicks;

			if(m_OneShot && m_TickCount >= m_CurrentClip.duration - 1.f)
			{
				m_IsPlaying = false;
				return;
			}

			// If m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
			m_TickCount = fmod(m_TickCount, m_CurrentClip.duration);
		}

		//3.
		//Find the enclosing keys
		//Iterate all the keys of the clip and find the following keys:
		//keyA > Closest Key with Tick before/smaller than m_TickCount
		//keyB > Closest Key with Tick after/bigger than m_TickCount
		AnimationKey keyA = m_CurrentClip.keys[0];
		AnimationKey keyB = m_CurrentClip.keys[m_CurrentClip.keys.size() - 1];
		for (const auto& key : m_CurrentClip.keys)
		{
			if (key.tick > keyA.tick && key.tick < m_TickCount)
				keyA = key;
			else if (key.tick < keyB.tick && key.tick > m_TickCount)
				keyB = key;
		}

		//4.
		//Interpolate between keys

		//Figure out the BlendFactor (See lab document)
		auto blendFactor = (m_TickCount - keyA.tick) / (keyB.tick - keyA.tick);

		//Clear the m_Transforms vector
		m_Transforms.clear();

		//FOR every boneTransform in a key (So for every bone)
		for (int i = 0; i < m_pMeshFilter->m_BoneCount; i++)
		{
			//	Retrieve the transform from keyA (transformA)
			auto transformA = keyA.boneTransforms[i];
			// 	Retrieve the transform from keyB (transformB)
			auto transformB = keyB.boneTransforms[i];
			//	Decompose both transforms
			XMVECTOR scaleA;
			XMVECTOR rotQuatA;
			XMVECTOR transA;
			XMMatrixDecompose(&scaleA, &rotQuatA, &transA, XMLoadFloat4x4(&transformA));
			XMVECTOR scaleB;
			XMVECTOR rotQuatB;
			XMVECTOR transB;
			XMMatrixDecompose(&scaleB, &rotQuatB, &transB, XMLoadFloat4x4(&transformB));
			//	Lerp between all the transformations (Position, Scale, Rotation)
			auto scaleLerped = XMVectorLerp(scaleA, scaleB, blendFactor);
			auto rotQuatLerped = XMQuaternionSlerp(rotQuatA, rotQuatB, blendFactor);
			auto transLerped = XMVectorLerp(transA, transB, blendFactor);
			//	Compose a transformation matrix with the lerp-results
			auto finalTransfMatrix = XMMatrixScalingFromVector(scaleLerped) *
				XMMatrixRotationQuaternion(rotQuatLerped) * XMMatrixTranslationFromVector(transLerped);
			//finalTransfMatrix = XMMatrixTranspose(finalTransfMatrix);
			//	Add the resulting matrix to the m_Transforms vector
			XMFLOAT4X4 finalTransform;
			XMStoreFloat4x4(&finalTransform, finalTransfMatrix);
			m_Transforms.push_back(finalTransform);
		}
	}
}

void ModelAnimator::SetAnimation(const std::wstring& clipName)
{
	//TODO_W7_()

	// Set m_ClipSet to false
	m_ClipSet = false;

	// Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName);
	const auto it = std::find_if(m_pMeshFilter->m_AnimationClips.begin(), m_pMeshFilter->m_AnimationClips.end(),
		[&clipName](const auto& clip) {return clip.name == clipName; });

	if (it != m_pMeshFilter->m_AnimationClips.end()) // If found
	{
		// Call SetAnimation(Animation Clip) with the found clip
		SetAnimation(*it);
	}
	else // If not
	{
		// Call Reset
		Reset(true);

		// Log a warning with an appropriate message
		Logger::LogError(L"ModelAnimator::SetAnimation > The requested clipName doesn't match any AnimationClip stored in MeshFilter!");
	}
}

void ModelAnimator::SetAnimation(UINT clipNumber)
{
	//TODO_W7_()

	// Set m_ClipSet to false
	m_ClipSet = false;

	// Check if clipNumber exceeds the actual m_AnimationClips vector size
	if (clipNumber >= m_pMeshFilter->m_AnimationClips.size())
	{
		// Call Reset
		Reset(true);

		// Log a warning with an appropriate message
		Logger::LogError(L"ModelAnimator::SetAnimation > The requested clipNumber doesn't match any AnimationClip stored in MeshFilter!");

		// And return
		return;
	}

	// If the clipNumber is valid, retrieve the AnimationClip from the m_AnimationClips vector
	const auto& requestedClip = m_pMeshFilter->m_AnimationClips[clipNumber];

	// And call SetAnimation(AnimationClip clip)
	SetAnimation(requestedClip);

}

void ModelAnimator::SetAnimation(const AnimationClip& clip)
{
	//TODO_W7_()

	// Set m_ClipSet to true
	m_ClipSet = true;

	// Set m_CurrentClip
	m_CurrentClip = clip;

	// Call Reset(false)
	Reset(false);
}

void ModelAnimator::Reset(bool pause)
{
	//TODO_W7_()

	// If pause is true, set m_IsPlaying to false
	if (pause)
		m_IsPlaying = false;

	// Set m_TickCount to zero
	m_TickCount = 0;

	// Set m_AnimationSpeed to 1.0f
	m_AnimationSpeed = 1.0f;

	// If m_ClipSet is true
	if (m_ClipSet)
	{
		// Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
		const auto& retrievedBoneTrans = m_CurrentClip.keys[0].boneTransforms;

		// Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
		m_Transforms.assign(retrievedBoneTrans.begin(), retrievedBoneTrans.end());
	}
	else // Else
	{
		// Create an IdentityMatrix
		XMFLOAT4X4 identityMat{};
		XMStoreFloat4x4(&identityMat, XMMatrixIdentity());

		// Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
		m_Transforms.assign(m_pMeshFilter->m_BoneCount, identityMat);
	}
}
