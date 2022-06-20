#pragma once
class ModelAnimator final
{
public:
	ModelAnimator(MeshFilter* pMeshFilter);
	~ModelAnimator() = default;
	ModelAnimator(const ModelAnimator& other) = delete;
	ModelAnimator(ModelAnimator&& other) noexcept = delete;
	ModelAnimator& operator=(const ModelAnimator& other) = delete;
	ModelAnimator& operator=(ModelAnimator&& other) noexcept = delete;

	void SetAnimation(const std::wstring& clipName);
	void SetAnimation(UINT clipNumber);
	void SetAnimation(const AnimationClip& clip);
	void Update(const SceneContext& sceneContext);
	void Reset(bool pause = true);
	void Play(bool oneShot = false) { m_IsPlaying = true; m_OneShot = oneShot; }
	void Pause() { m_IsPlaying = false; }
	void SetPlayReversed(bool reverse) { m_Reversed = reverse; }
	void SetAnimationSpeed(float speedPercentage) { m_AnimationSpeed = speedPercentage; }

	bool IsPlaying() const { return m_IsPlaying; }
	bool IsReversed() const { return m_Reversed; }
	float GetAnimationSpeed() const { return m_AnimationSpeed; }
	const AnimationClip& GetClip(int clipId) { ASSERT_IF_(clipId >= m_pMeshFilter->m_AnimationClips.size())return m_pMeshFilter->m_AnimationClips[clipId]; }
	UINT GetClipCount() const { return UINT(m_pMeshFilter->m_AnimationClips.size()); }
	const std::wstring& GetClipName() const { ASSERT_IF_(!m_ClipSet) return m_CurrentClip.name; }
	const std::vector<XMFLOAT4X4>& GetBoneTransforms() const { return m_Transforms; }

private:
	AnimationClip m_CurrentClip{};
	MeshFilter* m_pMeshFilter{};
	std::vector<XMFLOAT4X4> m_Transforms{};
	bool m_IsPlaying{}, m_OneShot{}, m_Reversed{}, m_ClipSet{};
	float m_TickCount{}, m_AnimationSpeed{1.f};
};

