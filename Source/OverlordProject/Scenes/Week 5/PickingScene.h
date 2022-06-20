#pragma once
class PickingScene : public GameScene
{
public:
	PickingScene() : GameScene(L"PickingScene") {	}
	~PickingScene() override = default;
	PickingScene(const PickingScene& other) = delete;
	PickingScene(PickingScene&& other) noexcept = delete;
	PickingScene& operator=(const PickingScene& other) = delete;
	PickingScene& operator=(PickingScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
};
