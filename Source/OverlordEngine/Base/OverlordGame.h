#pragma once

class RenderTarget;

class OverlordGame
{
public:

	OverlordGame();
	virtual ~OverlordGame();
	OverlordGame(const OverlordGame& other) = delete;
	OverlordGame(OverlordGame&& other) noexcept = delete;
	OverlordGame& operator=(const OverlordGame& other) = delete;
	OverlordGame& operator=(OverlordGame&& other) noexcept = delete;


	static LRESULT CALLBACK WindowsProcedureStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	//const GameContext& GetGameContext() const { return m_GameContext; }

	HRESULT Run(HINSTANCE hInstance);

	void SetRenderTarget(RenderTarget* renderTarget);
	RenderTarget* GetRenderTarget() const;

protected:
	virtual void OnGamePreparing(GameContext& /*gameContext*/){}
	virtual LRESULT WindowProcedureHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual void Initialize() = 0;

	bool m_IsActive{};

private:

	//FUNCTIONS
	//Initializations
	HRESULT InitializeAdapterAndOutput();
	HRESULT InitializeWindow();
	HRESULT InitializeDirectX();
	HRESULT InitializePhysX() const;
	HRESULT InitializeImGui() const;
	HRESULT InitializeGame();

	void ValidateGameContext();
	void GameLoop() const;

	//Windows Proc
	void StateChanged(int state, bool active);
	LRESULT WindowsProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HINSTANCE m_hInstance{};
	IDXGISwapChain* m_pSwapchain{};
	IDXGIFactory* m_pDxgiFactory{};
	RenderTarget* m_pDefaultRenderTarget{}, * m_pCurrentRenderTarget{};
	D3D11_VIEWPORT m_Viewport{};

	GameContext m_GameContext{};
};

