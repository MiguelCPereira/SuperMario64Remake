#pragma once
class FontTestScene : public GameScene
{
public:
	FontTestScene() :GameScene(L"FontTestScene") {}
	~FontTestScene() override = default;
	FontTestScene(const FontTestScene& other) = delete;
	FontTestScene(FontTestScene&& other) noexcept = delete;
	FontTestScene& operator=(const FontTestScene& other) = delete;
	FontTestScene& operator=(FontTestScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void OnGUI() override;

private:
	SpriteFont* m_pFont{};
	SpriteFont* m_pFont2{};

	std::string m_Text{ "Hello World! (Change Text/Position/Color in Scene Parameters)" };
	XMFLOAT2 m_TextPosition{};
	XMFLOAT4 m_TextColor{ 1.f,1.f,1.f,1.f };
};