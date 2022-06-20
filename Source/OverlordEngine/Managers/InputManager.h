#pragma once
class InputManager final
{
public:
	InputManager();
	~InputManager() = default;
	InputManager(const InputManager& other) = delete;
	InputManager(InputManager&& other) noexcept = delete;
	InputManager& operator=(const InputManager& other) = delete;
	InputManager& operator=(InputManager&& other) noexcept = delete;

	static void Initialize(const GameContext& gameContext);
	static void Release();

	static void ForceMouseToCenter(bool force);
	static void SetEnabled(bool enabled)
	{
		m_UserEnabled = enabled;
		Logger::LogDebug(L"INPUT ENABLED = {}", enabled);
	}
	static bool IsEnabled() { return m_Enabled; }

	static const POINT& GetMousePosition(bool previousFrame = false) { return (previousFrame) ? m_OldMousePosition : m_CurrMousePosition; }
	static const POINT& GetMouseMovement() { return m_MouseMovement; }
	static const XMFLOAT2& GetMouseMovementNormalized() { return m_MouseMovementNormalized; }
	static XMFLOAT2 GetThumbstickPosition(bool leftThumbstick = true, GamepadIndex playerIndex = GamepadIndex::playerOne);
	static float GetTriggerPressure(bool leftTrigger = true, GamepadIndex playerIndex = GamepadIndex::playerOne);
	static void SetVibration(float leftVibration, float rightVibration, GamepadIndex playerIndex = GamepadIndex::playerOne);
	static BYTE GetKeyState(int key, bool previousFrame = false);

	static void CursorVisible(bool visible) { ShowCursor(visible); }
	static bool IsGamepadConnected(GamepadIndex index) { return m_ConnectedGamepads[DWORD(index)]; }

	static bool IsKeyboardKey(InputState state, int key);
	static bool IsMouseButton(InputState state, int button);
	static bool IsGamepadButton(InputState state, WORD button, GamepadIndex playerIndex = GamepadIndex::playerOne);
	static void UpdateInputStates(bool overrideEnable = false);

	//Instance Interface
	void Update();
	bool AddInputAction(InputAction action);
	bool IsActionTriggered(int actionID) const;

private:

	std::map<int, InputAction> m_InputActions{};

	static BYTE* m_pCurrKeyboardState, * m_pOldKeyboardState, * m_pKeyboardState0, * m_pKeyboardState1;
	static POINT m_CurrMousePosition, m_OldMousePosition, m_MouseMovement;
	static XMFLOAT2 m_MouseMovementNormalized;
	static XINPUT_STATE m_OldGamepadState[XUSER_MAX_COUNT], m_CurrGamepadState[XUSER_MAX_COUNT];

	static std::chrono::time_point<std::chrono::steady_clock> m_LastUpdate;

	static bool m_IsInitialized;
	static bool m_KeyboardState0Active;
	static bool m_ConnectedGamepads[XUSER_MAX_COUNT];
	static bool m_Enabled, m_UserEnabled, m_PrevEnable, m_EnableChanged;
	static bool m_ForceToCenter;
	static GameContext m_GameContext;

	static constexpr int KBCODE_MIN{ 7 };
	static constexpr int KBCODE_MAX{ 254 };
	static constexpr int MSCODE_MIN{ 0 };
	static constexpr int MSCODE_MAX{ 6 };
	static constexpr WORD GPCODE_MIN{ 0 };
	static constexpr WORD GPCODE_MAX{ 0x8000 };

	static void UpdateGamepadStates();
	static bool UpdateKeyboardStates();

	static bool IsKeyboardKeyDown_unsafe(int key, bool previousFrame = false);
	static bool IsMouseButtonDown_unsafe(int button, bool previousFrame = false);
	static bool IsGamepadButtonDown_unsafe(WORD button, GamepadIndex playerIndex = GamepadIndex::playerOne, bool previousFrame = false);

	static bool IsKeyCodeValid(int code) { return code > KBCODE_MIN && code < KBCODE_MAX; }
	static bool IsMouseCodeValid(int code) { return code > MSCODE_MIN && code < MSCODE_MAX; }
	static bool IsGamepadCodeValid(DWORD code) { return code > GPCODE_MIN && code < GPCODE_MAX; }
};

