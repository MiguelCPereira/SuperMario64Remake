#pragma once
class HardwareSkinningScene: public GameScene
{
public:
	HardwareSkinningScene() : GameScene(L"HardwareSkinningScene") {}
	~HardwareSkinningScene() override;
	HardwareSkinningScene(const HardwareSkinningScene& other) = delete;
	HardwareSkinningScene(HardwareSkinningScene&& other) noexcept = delete;
	HardwareSkinningScene& operator=(const HardwareSkinningScene& other) = delete;
	HardwareSkinningScene& operator=(HardwareSkinningScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void OnGUI() override;

private:
	ModelAnimator* pAnimator{};

	int m_AnimationClipId{ 0 };
	float m_AnimationSpeed{ 1.f };

	char** m_ClipNames{};
	UINT m_ClipCount{};
};

