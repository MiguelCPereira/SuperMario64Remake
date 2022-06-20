#include "stdafx.h"
#include "OverlordGame.h"

OverlordGame::OverlordGame():
	m_IsActive(true)
{
	Logger::Initialize();
}


OverlordGame::~OverlordGame()
{
	//GameSettings Cleanup
	SafeRelease(m_GameContext.d3dContext.pAdapter);
	SafeRelease(m_GameContext.d3dContext.pOutput);

	//Game Cleanup
	MaterialManager::Destroy();
	ContentManager::Release(); //TODO > Singleton
	DebugRenderer::Release(); //TODO > Singleton
	InputManager::Release(); //Todo > Rename to Destroy
	SceneManager::Destroy();
	PhysXManager::Destroy();
	SoundManager::Destroy();
	SpriteRenderer::Destroy();
	TextRenderer::Destroy();
	ShadowMapRenderer::Destroy();
	Logger::Release(); //TODO > Singleton

	//ImGui Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//DirectX Cleanup
	SafeDelete(m_pDefaultRenderTarget);
	SafeRelease(m_pDxgiFactory);
	SafeRelease(m_pSwapchain);

	if(m_GameContext.d3dContext.pDeviceContext)
	{
		m_GameContext.d3dContext.pDeviceContext->ClearState();
		m_GameContext.d3dContext.pDeviceContext->Flush();
		SafeRelease(m_GameContext.d3dContext.pDeviceContext);
	}

	SafeRelease(m_GameContext.d3dContext.pDevice);
	
}

void OverlordGame::ValidateGameContext()
{
	ASSERT_IF(m_GameContext.windowWidth <= 0, L"GameContext - WindowWidth({}}) is invalid.", m_GameContext.windowWidth);
	ASSERT_IF(m_GameContext.windowHeight <= 0, L"GameContext - WindowHeight({}}) is invalid.", m_GameContext.windowHeight);
}

HRESULT OverlordGame::Run(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
	m_GameContext.pGame = this;

	//PREPARATION
	//***********
	OnGamePreparing(m_GameContext);
	ValidateGameContext();

	//INITIALIZE
	//**********
	HANDLE_ERROR(InitializeAdapterAndOutput())
	HANDLE_ERROR(InitializeWindow())
	HANDLE_ERROR(InitializeDirectX())
	HANDLE_ERROR(InitializeImGui())
	HANDLE_ERROR(InitializeGame())

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while(msg.message != WM_QUIT)
	{
		while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				break;
		}

		GameLoop();
	}

	//TODO: should return 'msg.wParam'
	return S_OK;
}

#pragma region
HRESULT OverlordGame::InitializeAdapterAndOutput()
{
	HANDLE_ERROR(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDxgiFactory)))

	if(!m_GameContext.d3dContext.pAdapter)
	{	
		HANDLE_ERROR(m_pDxgiFactory->EnumAdapters(0, &m_GameContext.d3dContext.pAdapter))
	}

	if(!m_GameContext.d3dContext.pOutput)
	{
		HANDLE_ERROR(m_GameContext.d3dContext.pAdapter->EnumOutputs(0, &m_GameContext.d3dContext.pOutput))
	}

	return S_OK;
}

HRESULT OverlordGame::InitializeWindow()
{
	//1. Create Windowclass
	//*********************
	const auto className = L"OverlordWindowClass";
	WNDCLASS windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASS));
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.hIcon = nullptr;
	windowClass.hbrBackground = nullptr;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowsProcedureStatic;
	windowClass.hInstance = m_hInstance;
	windowClass.lpszClassName = className;

	if(!RegisterClass(&windowClass))
	{
		const auto error = GetLastError();
		HANDLE_ERROR(HRESULT_FROM_WIN32(error))
	}

	//2. Create Window
	//****************
	DXGI_OUTPUT_DESC outputDesc{};
	HANDLE_ERROR(m_GameContext.d3dContext.pOutput->GetDesc(&outputDesc))

	RECT r = {0, 0, LONG(m_GameContext.windowWidth), LONG(m_GameContext.windowHeight) };
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
	const auto winWidth = r.right - r.left;
	const auto winHeight = r.bottom - r.top;

	const int x = outputDesc.DesktopCoordinates.left + ((outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left)/2) - winWidth/2;
	const int y = (outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top)/2 - winHeight/2;

	m_GameContext.windowHandle = CreateWindow(className,
									m_GameContext.windowTitle.c_str(),
									WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
									x, 
									y, 
									winWidth, 
									winHeight, 
									NULL,
									nullptr, 
									m_hInstance, 
									this);

	if(!m_GameContext.windowHandle)
	{
		HANDLE_ERROR(HRESULT_FROM_WIN32(GetLastError()))
	}

	//3. Show The Window
	//******************

#pragma warning(push)
#pragma warning(disable: 6387)
	ShowWindow(m_GameContext.windowHandle, SW_SHOWDEFAULT);
#pragma warning(pop)

	return S_OK;
}

HRESULT OverlordGame::InitializeDirectX()
{
	//Create DX11 Device & Context
	UINT createDeviceFlags = 0;

	#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

#pragma warning(push)
#pragma warning(disable: 26812)
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	HANDLE_ERROR(D3D11CreateDevice(m_GameContext.d3dContext.pAdapter,
								D3D_DRIVER_TYPE_UNKNOWN,
								nullptr,
								createDeviceFlags,
								nullptr,0,
								D3D11_SDK_VERSION,
								&m_GameContext.d3dContext.pDevice,
								&featureLevel,
								&m_GameContext.d3dContext.pDeviceContext))
#pragma warning(pop)
	

	ASSERT_IF(featureLevel < D3D_FEATURE_LEVEL_10_0, L"Feature level 10.0+ not supported on this device!");

	if (featureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		Logger::LogWarning(L"Feature level 10.1, some DirectX11 specific features won't be available on this device!");
	}

	//Create Swapchain descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc.Width = m_GameContext.windowWidth;
	swapChainDesc.BufferDesc.Height = m_GameContext.windowHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	// Update PP
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = m_GameContext.windowHandle;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0 ;
#pragma warning(push)
#pragma warning(disable: 6387)
	HANDLE_ERROR(m_pDxgiFactory->CreateSwapChain(m_GameContext.d3dContext.pDevice, &swapChainDesc, &m_pSwapchain))
#pragma warning(pop)

	//Create the default rendertarget.
	m_pDefaultRenderTarget = new RenderTarget(m_GameContext.d3dContext);
	
	ID3D11Texture2D *pBackbuffer = nullptr;
	HANDLE_ERROR(m_pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackbuffer)))

	RENDERTARGET_DESC rtDesc;
	rtDesc.pColor = pBackbuffer;
	HANDLE_ERROR(m_pDefaultRenderTarget->Create(rtDesc))

	//Set Default Rendertarget 
	SetRenderTarget(nullptr);

	//TODO: Logger::LogTodo(L"Viewport ownership, overlordgame");
	m_Viewport.Width	= static_cast<FLOAT>(m_GameContext.windowWidth);
	m_Viewport.Height	= static_cast<FLOAT>(m_GameContext.windowHeight);
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
	m_GameContext.d3dContext.pDeviceContext->RSSetViewports(1,&m_Viewport);

#ifdef _DEBUG
	ID3D11Debug* pDebug{};
	if (SUCCEEDED(m_GameContext.d3dContext.pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pDebug))))
	{
		ID3D11InfoQueue* infoQueue = nullptr;
		if (SUCCEEDED(pDebug->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&infoQueue))))
		{
			D3D11_MESSAGE_ID knownMessages[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
			};

			D3D11_INFO_QUEUE_FILTER filter{};
			filter.DenyList.NumIDs = _countof(knownMessages);
			filter.DenyList.pIDList = knownMessages;

			infoQueue->AddStorageFilterEntries(&filter);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			infoQueue->Release();
		}
	}

	pDebug->Release();
#endif

	return S_OK;
}

HRESULT OverlordGame::InitializeImGui() const
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	//ImguiStyles::Apply_GoldAndBlack();
	//ImguiStyles::Apply_Custom();

	// Load DIN Fonts
	ImguiHelper::Fonts::DIN::LoadFonts();
	ImGui::GetIO().FontDefault = ImguiFonts::pFont_DIN_Regular_14;

	// Setup Platform/Renderer backends
	if(!ImGui_ImplWin32_Init(m_GameContext.windowHandle))
	{
		HANDLE_ERROR(L"Failed to initialize ImGui. (ImGui_ImplWin32_Init");
	}

	if(!ImGui_ImplDX11_Init(m_GameContext.d3dContext.pDevice, m_GameContext.d3dContext.pDeviceContext))
	{
		HANDLE_ERROR(L"Failed to initialize ImGui. (ImGui_ImplDX11_Init");
	}



	return S_OK;
}

HRESULT OverlordGame::InitializeGame()
{
	//******************
	//MANAGER INITIALIZE
	TextureData::CreateGUID();

	ContentManager::Initialize(m_GameContext);
	DebugRenderer::Initialize(m_GameContext);
	InputManager::Initialize(m_GameContext);
	PhysXManager::Create(m_GameContext);
	SoundManager::Create(m_GameContext); //Constructor calls Initialize
	MaterialManager::Create(m_GameContext);
	SceneManager::Create(m_GameContext);
	SpriteRenderer::Create(m_GameContext);
	TextRenderer::Create(m_GameContext);
	ShadowMapRenderer::Create(m_GameContext);

	//***************
	//GAME INITIALIZE
	Initialize();

	//********************
	//GAME POST INITIALIZE
	SceneManager::Get()->PostInitialize();

	return S_OK;
}
#pragma endregion Initializations

#pragma region
void OverlordGame::StateChanged(int state, bool active)
{
	if (state == 1)
	{
		//WINDOW ACTIVE/INACTIVE
		if (m_IsActive != active)
		{
			m_IsActive = active;
			SceneManager::Get()->WindowStateChanged(state, active);
		}
	}
	else if (state == 2)
	{
		//INPUT ACTIVE/INACTIVE
		InputManager::SetEnabled(active);
	}
}

LRESULT CALLBACK OverlordGame::WindowsProcedureStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_CREATE)
	{
		const auto pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCS->lpCreateParams));
	}
	else
	{
		if(const auto pThisGame = reinterpret_cast<OverlordGame*>(GetWindowLongPtr(hWnd, GWLP_USERDATA))) 
			return pThisGame->WindowsProcedure(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT OverlordGame::WindowsProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)StateChanged(1, true);
			else StateChanged(1, false);

			return 0;
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED) StateChanged(0, false);
			else if (wParam == SIZE_RESTORED) StateChanged(0, true);
			return 0;
		case WM_SETFOCUS:
			if (HWND(wParam) == m_GameContext.windowHandle)
			{
				StateChanged(1, true);
				return 0;
			}
			break;
		case WM_KILLFOCUS:
			if (HWND(wParam) == m_GameContext.windowHandle)
			{
				StateChanged(1, false);
				return 0;
			}
			break;
		case WM_ENTERSIZEMOVE:
			StateChanged(0, false);
			StateChanged(1, false);
			break;
		case WM_EXITSIZEMOVE:
			StateChanged(0, true);
			StateChanged(1, true);
			break;
	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam) > 0)
		return 0;

	if(m_IsActive && WindowProcedureHook(hWnd, message, wParam, lParam) == 0)
		return 0;

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT OverlordGame::WindowProcedureHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(message);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	return -1;
}
#pragma endregion Windows Procedures

#pragma region
void OverlordGame::GameLoop() const
{
	GameStats::BeginFrame();

	//******
	//UPDATE
	InputManager::UpdateInputStates(ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard);
	SceneManager::Get()->Update();

	//****
	//DRAW
	const auto& activeSceneSettings = SceneManager::Get()->GetActiveSceneSettings();

	//Clear Backbuffer
	m_GameContext.d3dContext.pDeviceContext->ClearRenderTargetView(m_pCurrentRenderTarget->GetRenderTargetView(), reinterpret_cast<const float*>(&activeSceneSettings.clearColor));
	m_GameContext.d3dContext.pDeviceContext->ClearDepthStencilView(m_pCurrentRenderTarget->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Draw Scene
	SceneManager::Get()->Draw();

	//**********
	//DRAW IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//bool drawDemo{ true };
	//ImGui::ShowDemoWindow(&drawDemo);

	SceneManager::Get()->OnGUI();
	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());



	//*******
	//PRESENT
	m_pSwapchain->Present(activeSceneSettings.vSyncEnabled ? 1 : 0, 0);

	GameStats::EndFrame();
}

void OverlordGame::SetRenderTarget(RenderTarget* renderTarget)
{
	if(renderTarget == nullptr)
		renderTarget = m_pDefaultRenderTarget;

	const auto pRTV = renderTarget->GetRenderTargetView();
	m_GameContext.d3dContext.pDeviceContext->OMSetRenderTargets(1, &pRTV, renderTarget->GetDepthStencilView());

	m_pCurrentRenderTarget = renderTarget;
}

// Update PP
RenderTarget* OverlordGame::GetRenderTarget() const
{
	return m_pCurrentRenderTarget;
}
#pragma endregion METHODS
