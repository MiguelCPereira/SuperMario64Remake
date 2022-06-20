#include "stdafx.h"
#include "InputManager.h"

PBYTE InputManager::m_pCurrKeyboardState = nullptr;
PBYTE InputManager::m_pOldKeyboardState = nullptr;
PBYTE InputManager::m_pKeyboardState0 = nullptr;
PBYTE InputManager::m_pKeyboardState1 = nullptr;
bool InputManager::m_KeyboardState0Active = true;
POINT InputManager::m_OldMousePosition = POINT();
POINT InputManager::m_CurrMousePosition = POINT();
POINT InputManager::m_MouseMovement = POINT();
XINPUT_STATE InputManager::m_OldGamepadState[XUSER_MAX_COUNT];
XINPUT_STATE InputManager::m_CurrGamepadState[XUSER_MAX_COUNT];
bool InputManager::m_ConnectedGamepads[XUSER_MAX_COUNT];
bool InputManager::m_UserEnabled = true;
bool InputManager::m_Enabled = false;
bool InputManager::m_PrevEnable = false;
bool InputManager::m_EnableChanged = false;
bool InputManager::m_IsInitialized = false;
bool InputManager::m_ForceToCenter = false;
GameContext InputManager::m_GameContext = {};
std::chrono::time_point<std::chrono::steady_clock> InputManager::m_LastUpdate = {};
XMFLOAT2 InputManager::m_MouseMovementNormalized = {};

InputManager::InputManager()
{
	assert(m_IsInitialized);
}

void InputManager::Initialize(const GameContext& gameContext)
{
	if (!m_IsInitialized)
	{
		m_pKeyboardState0 = new BYTE[256];
		m_pKeyboardState1 = new BYTE[256];

		if(!GetKeyboardState(m_pKeyboardState0) ||
		!GetKeyboardState(m_pKeyboardState1))
		{
			Logger::LogWarning(L"InputManager::Initialize >> Failed to GetKeyboardState.");
		}

		m_GameContext = gameContext;

		m_IsInitialized = true;
	}
}

void InputManager::Release()
{
	if (m_IsInitialized)
	{
		delete[] m_pKeyboardState0;
		delete[] m_pKeyboardState1;

		m_pKeyboardState0 = nullptr;
		m_pKeyboardState1 = nullptr;
		m_pCurrKeyboardState = nullptr;
		m_pOldKeyboardState = nullptr;

		m_IsInitialized = false;
	}
}

bool InputManager::AddInputAction(InputAction action)
{
	if (const auto it = m_InputActions.find(action.actionID); it != m_InputActions.end()) return false;

	m_InputActions[action.actionID] = action;

	return true;
}

bool InputManager::IsActionTriggered(int actionID) const
{
	return m_InputActions.at(actionID).isTriggered;
}

void InputManager::ForceMouseToCenter(bool force)
{
	m_ForceToCenter = force;

	if (force)
	{
		POINT mouseCenter{};
		m_CurrMousePosition.x = static_cast<LONG>(m_GameContext.windowWidth) / 2;
		m_CurrMousePosition.y = static_cast<LONG>(m_GameContext.windowHeight) / 2;
		mouseCenter.x = m_CurrMousePosition.x;
		mouseCenter.y = m_CurrMousePosition.y;
		ClientToScreen(m_GameContext.windowHandle, &mouseCenter);

		SetCursorPos(mouseCenter.x, mouseCenter.y);
	}
}

void InputManager::UpdateGamepadStates()
{
	for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		m_OldGamepadState[i] = m_CurrGamepadState[i];

		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		const DWORD dwResult = XInputGetState(i, &state);
		m_CurrGamepadState[i] = state;

		if (dwResult == ERROR_SUCCESS)
		{
			m_ConnectedGamepads[i] = true;
		}
		else
		{
			m_ConnectedGamepads[i] = false;
		}
	}
}

bool InputManager::UpdateKeyboardStates()
{
	//Get Current KeyboardState and set Old KeyboardState
	BOOL getKeyboardResult;
	if (m_KeyboardState0Active)
	{
		getKeyboardResult = GetKeyboardState(m_pKeyboardState1);
		m_pOldKeyboardState = m_pKeyboardState0;
		m_pCurrKeyboardState = m_pKeyboardState1;
	}
	else
	{
		getKeyboardResult = GetKeyboardState(m_pKeyboardState0);
		m_pOldKeyboardState = m_pKeyboardState1;
		m_pCurrKeyboardState = m_pKeyboardState0;
	}

	m_KeyboardState0Active = !m_KeyboardState0Active;

	return getKeyboardResult > 0 ? true : false;
}

void InputManager::Update()
{
	if (!m_Enabled)
		return;

	if (ImGui::GetIO().WantCaptureMouseUnlessPopupClose)
		return;

	//Reset previous InputAction States
	for (auto it = m_InputActions.begin(); it != m_InputActions.end(); ++it)
	{
		const auto currAction = &(it->second);

		currAction->isTriggered = IsKeyboardKey(currAction->triggerState, currAction->keyboardCode);
		currAction->isTriggered = currAction->isTriggered || IsMouseButton(currAction->triggerState, currAction->mouseButtonCode);
		currAction->isTriggered = currAction->isTriggered || IsGamepadButton(currAction->triggerState, currAction->gamepadButtonCode, currAction->playerIndex);
	}
}

bool InputManager::IsKeyboardKey(InputState state, int key)
{
	if (!IsKeyCodeValid(key)) return false;

	switch (state)
	{
	case InputState::down:
		return IsKeyboardKeyDown_unsafe(key, true) && IsKeyboardKeyDown_unsafe(key, false);
	case InputState::pressed:
		return !IsKeyboardKeyDown_unsafe(key, true) && IsKeyboardKeyDown_unsafe(key, false);
	case InputState::released:
		return IsKeyboardKeyDown_unsafe(key, true) && !IsKeyboardKeyDown_unsafe(key, false);
	}

	return false;
}

bool InputManager::IsMouseButton(InputState state, int button)
{
	if (!IsMouseCodeValid(button)) return false;

	switch (state)
	{
	case InputState::down:
		return IsMouseButtonDown_unsafe(button, true) && IsMouseButtonDown_unsafe(button, false);
	case InputState::pressed:
		return !IsMouseButtonDown_unsafe(button, true) && IsMouseButtonDown_unsafe(button, false);
	case InputState::released:
		return IsMouseButtonDown_unsafe(button, true) && !IsMouseButtonDown_unsafe(button, false);
	}

	return false;
}

bool InputManager::IsGamepadButton(InputState state, WORD button, GamepadIndex playerIndex)
{
	if (!IsGamepadCodeValid(button)) return false;

	switch (state)
	{
	case InputState::down:
		return IsGamepadButtonDown_unsafe(button, playerIndex, true) && IsGamepadButtonDown_unsafe(button, playerIndex, false);
	case InputState::pressed:
		return !IsGamepadButtonDown_unsafe(button, playerIndex, true) && IsGamepadButtonDown_unsafe(button, playerIndex, false);
	case InputState::released:
		return IsGamepadButtonDown_unsafe(button, playerIndex, true) && !IsGamepadButtonDown_unsafe(button, playerIndex, false);
	}

	return false;
}

void InputManager::UpdateInputStates(bool overrideEnable)
{
	m_Enabled = m_UserEnabled && !overrideEnable;

	if(!m_Enabled && !m_EnableChanged)
	{
		m_MouseMovement = { 0,0 };
		m_MouseMovementNormalized = { 0.f,0.f };
		return;
	}

	//Update Keyboard/GamePad states once
	UpdateKeyboardStates();
	UpdateGamepadStates();

	//Input synchronization (VSync ON vs OFF) - Mouse Movement only
	if (m_GameContext.inputUpdateFrequency > 0)
	{
		const auto currTime = std::chrono::steady_clock::now();
		const std::chrono::duration<float> elapsed = currTime - m_LastUpdate;
		if (elapsed.count() < m_GameContext.inputUpdateFrequency) {

			m_EnableChanged = m_PrevEnable != m_Enabled;
			m_PrevEnable = m_Enabled;
			return;
		}
		m_LastUpdate = currTime;
	}

	//TODO: Refactor Mouse Updates
	//Update Mouse Position
	m_OldMousePosition = m_CurrMousePosition;
	if (GetCursorPos(&m_CurrMousePosition))
	{
		if (m_EnableChanged || !ScreenToClient(m_GameContext.windowHandle, &m_CurrMousePosition))
		{
			m_CurrMousePosition = m_OldMousePosition;
		}
	}
	
	m_MouseMovement.x = m_CurrMousePosition.x - m_OldMousePosition.x;
	m_MouseMovement.y = m_CurrMousePosition.y - m_OldMousePosition.y;

	//Normalized
	m_MouseMovementNormalized.x = m_MouseMovement.x > 0 ? 1.f : (m_MouseMovement.x < 0 ? -1.f : 0.f);
	m_MouseMovementNormalized.x = m_MouseMovement.y > 0 ? 1.f : (m_MouseMovement.y < 0 ? -1.f : 0.f);

	//@END
	m_EnableChanged = m_PrevEnable != m_Enabled;
	m_PrevEnable = m_Enabled;
}

BYTE InputManager::GetKeyState(int key, bool previousFrame)
{
	if (previousFrame)
		return m_pOldKeyboardState[key];

	return m_pCurrKeyboardState[key];
}

//NO RANGE CHECKS
bool InputManager::IsKeyboardKeyDown_unsafe(int key, bool previousFrame)
{
	if (!m_Enabled)return false;

	if (previousFrame)
		return (m_pOldKeyboardState[key] & 0xF0) != 0;

	return (m_pCurrKeyboardState[key] & 0xF0) != 0;
}

//NO RANGE CHECKS
bool InputManager::IsMouseButtonDown_unsafe(int button, bool previousFrame)
{
	if (!m_Enabled)return false;

	if (previousFrame)
		return (m_pOldKeyboardState[button] & 0xF0) != 0;

	return (m_pCurrKeyboardState[button] & 0xF0) != 0;
}

//NO RANGE CHECKS
bool InputManager::IsGamepadButtonDown_unsafe(WORD button, GamepadIndex playerIndex, bool previousFrame)
{
	if (!m_Enabled)return false;

	if (!m_ConnectedGamepads[int(playerIndex)])
		return false;

	if (previousFrame)
		return (m_OldGamepadState[int(playerIndex)].Gamepad.wButtons & button) != 0;

	return (m_CurrGamepadState[int(playerIndex)].Gamepad.wButtons & button) != 0;
}

XMFLOAT2 InputManager::GetThumbstickPosition(bool leftThumbstick, GamepadIndex playerIndex)
{
	XMFLOAT2 pos;
	if (leftThumbstick)
	{
		pos = XMFLOAT2(m_CurrGamepadState[int(playerIndex)].Gamepad.sThumbLX, m_CurrGamepadState[int(playerIndex)].Gamepad.sThumbLY);

		if (pos.x > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pos.x < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)pos.x = 0;
		if (pos.y > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pos.y < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)pos.y = 0;
	}
	else
	{
		pos = XMFLOAT2(m_CurrGamepadState[int(playerIndex)].Gamepad.sThumbRX, m_CurrGamepadState[int(playerIndex)].Gamepad.sThumbRY);

		if (pos.x > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pos.x < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)pos.x = 0;
		if (pos.y > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pos.y < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)pos.y = 0;
	}

	if (pos.x < 0)pos.x /= 32768;
	else pos.x /= 32767;

	if (pos.y < 0)pos.y /= 32768;
	else pos.y /= 32767;

	return pos;
}

float InputManager::GetTriggerPressure(bool leftTrigger, GamepadIndex playerIndex)
{
	if (leftTrigger)
	{
		return m_CurrGamepadState[int(playerIndex)].Gamepad.bLeftTrigger / 255.0f;
	}
	else
	{
		return m_CurrGamepadState[int(playerIndex)].Gamepad.bRightTrigger / 255.0f;
	}
}

void InputManager::SetVibration(float leftVibration, float rightVibration, GamepadIndex playerIndex)
{
	XINPUT_VIBRATION vibration;
	MathHelper::Clamp<float>(leftVibration, 0.0f, 1.0f);
	MathHelper::Clamp<float>(rightVibration, 0.0f, 1.0f);

	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	vibration.wLeftMotorSpeed = static_cast<WORD>(leftVibration * 65535);
	vibration.wRightMotorSpeed = static_cast<WORD>(rightVibration * 65535);

	XInputSetState(int(playerIndex), &vibration);
}