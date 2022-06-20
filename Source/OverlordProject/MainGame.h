#pragma once
class MainGame final : public OverlordGame
{
public:
	MainGame() = default;
	~MainGame() override = default;
	MainGame(const MainGame& other) = delete;
	MainGame(MainGame&& other) noexcept = delete;
	MainGame& operator=(const MainGame& other) = delete;
	MainGame& operator=(MainGame&& other) noexcept = delete;

protected:
	void OnGamePreparing(GameContext& gameContext) override;
	void Initialize() override;
	//LRESULT WindowProcedureHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
};

