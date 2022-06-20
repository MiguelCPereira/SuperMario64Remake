#pragma once

class ComponentTestScene final : public GameScene
{
public:
	ComponentTestScene() :GameScene(L"ComponentTestScene"){}
	~ComponentTestScene() override = default;

	ComponentTestScene(const ComponentTestScene& other) = delete;
	ComponentTestScene(ComponentTestScene&& other) noexcept = delete;
	ComponentTestScene& operator=(const ComponentTestScene& other) = delete;
	ComponentTestScene& operator=(ComponentTestScene&& other) noexcept = delete;

protected:
	void Initialize() override;
};

