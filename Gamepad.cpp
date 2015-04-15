/* Simple2D Graphics Library for C++
   (c) Katy Coe (sole proprietor) 2012-2013

   No unauthorized copying or distribution. You may re-use Simple2D in your own non-commercial projects.
   For commercial applications, you must obtain a commercial license.

   Support and documentation: www.djkaty.com/simple2d
*/

/* XInput support */

#include "Simple2DLib.h"
#include <math.h>

using std::string;
using std::map;
using std::pair;

namespace S2D
{

int Gamepad::GetPort()
{
	return cId + 1;
}

XINPUT_GAMEPAD *Gamepad::GetState()
{
	return &state.Gamepad;
}

bool Gamepad::CheckConnection()
{
	int controllerId = -1;

	// Check each port for a connection until one is found
	for (DWORD i = 0; i < XUSER_MAX_COUNT && controllerId == -1; i++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		if (XInputGetState(i, &state) == ERROR_SUCCESS)
			controllerId = i;
	}

	cId = controllerId;

	return controllerId != -1;
}

// Returns false if the controller has been disconnected
bool Gamepad::Refresh()
{
	// Find the main application window if key translation events are enabled
	if (keyEvents)
		targetWindow = Simple2D::App->GetWindow();

	// Try to establish a connection with the controller if none was connected last time
	if (cId == -1 && GetTickCount() >= lastConnectionCheck + checkInterval)
	{
		lastConnectionCheck = GetTickCount();
		CheckConnection();
	}

	// If the controller is connected...
	if (cId != -1)
	{
		// Store previous state
		previous = state;

		prevLeftStickX = leftStickX;
		prevLeftStickY = leftStickY;
		prevRightStickX = rightStickX;
		prevRightStickY = rightStickY;
		prevLeftTrigger = leftTrigger;
		prevRightTrigger = rightTrigger;

		// Check state and check for disconnection
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (XInputGetState(cId, &state) != ERROR_SUCCESS)
		{
			cId = -1;
			return false;
		}

		// Calculate deadzone-normalized percentages of movement for the
		// analog sticks and triggers

		float normLX = max(-1, (float) state.Gamepad.sThumbLX / 32767);
		float normLY = max(-1, (float) state.Gamepad.sThumbLY / 32767);

		leftStickX = (abs(normLX) < deadzoneX ? 0 : (fabsf(normLX) - deadzoneX) * (normLX / fabsf(normLX)));
		leftStickY = (abs(normLY) < deadzoneY ? 0 : (fabsf(normLY) - deadzoneY) * (normLY / fabsf(normLY)));

		if (deadzoneX > 0) leftStickX *= 1 / (1 - deadzoneX);
		if (deadzoneY > 0) leftStickY *= 1 / (1 - deadzoneY);

		float normRX = max(-1, (float) state.Gamepad.sThumbRX / 32767);
		float normRY = max(-1, (float) state.Gamepad.sThumbRY / 32767);

		rightStickX = (abs(normRX) < deadzoneX ? 0 : (fabsf(normRX) - deadzoneX) * (normRX / fabsf(normRX)));
		rightStickY = (abs(normRY) < deadzoneY ? 0 : (fabsf(normRY) - deadzoneY) * (normRY / fabsf(normRY)));

		if (deadzoneX > 0) rightStickX *= 1 / (1 - deadzoneX);
		if (deadzoneY > 0) rightStickY *= 1 / (1 - deadzoneY);

		leftTrigger = (float) state.Gamepad.bLeftTrigger / 255;
		rightTrigger = (float) state.Gamepad.bRightTrigger / 255;

		// Dispatch keyboard events if desired
		if (keyEvents || targetWindow != NULL)
		{
			for (auto button : Buttons)
			{
				// If button is pushed
				if ((state.Gamepad.wButtons & button.first) == button.first)
				{
					// Get key mapping or use XINPUT_GAMEPAD_* value if no mapping exists
					WORD mapping = (keyMap.find(button.first) != keyMap.end()?
						keyMap.find(button.first)->second : button.first);

					// Get current time and last WM_KEYDOWN message for repeat interval check
					DWORD now = GetTickCount();
					DWORD last = (lastPress.find(button.first) != lastPress.end()?
						lastPress.find(button.first)->second : 0);

					// Find desired repeat interval for this button
					unsigned int ms = repeatMs.find(button.first)->second;

					// If first press, or repeat interval passed (and repeat interval != 0)
					if ((now - last >= ms && ms > 0)
						|| last == 0
						|| (ms == 0 && (previous.Gamepad.wButtons & button.first) != button.first))
					{
						// Update last press time
						lastPress.erase(button.first);
						lastPress.insert(std::map<WORD, DWORD>::value_type(button.first, now));

						// Send keyboard event
						if (!onGamepadPress)
							SendMessage(targetWindow, WM_KEYDOWN, mapping,
							((previous.Gamepad.wButtons & button.first) != button.first? 0 << 30 : 1 << 30));

						else
							onGamepadPress(mapping, (previous.Gamepad.wButtons & button.first) == button.first? false : true);

					}
				}

				// Checking for button release events, will cause the state
				// packet number to be incremented
				if (previous.dwPacketNumber < state.dwPacketNumber)
				{
					// If the button was pressed but is no longer pressed
					if ((state.Gamepad.wButtons & button.first) != button.first
						&& (previous.Gamepad.wButtons & button.first) == button.first)
					{
						// Get key mapping if one exists
						WORD mapping = (keyMap.find(button.first) != keyMap.end()?
							keyMap.find(button.first)->second : button.first);

						// Remove last press time
						lastPress.erase(button.first);

						// Send keyboard event
						if (!onGamepadRelease)
							SendMessage(targetWindow, WM_KEYUP, mapping, 0);
						else
							onGamepadRelease(mapping);
					}
				}
			}

			// Do keyboard event dispatch processing for analog items
			// (unmapped items won't have events generated for them)
			for (auto item : analogMap)
			{
				WORD mapping = item.second.key;

				switch (item.first) {
				case AnalogButtons::LeftStickLeft:
					sendKeysOnThreshold(AnalogButtons::LeftStickLeft, leftStickX, prevLeftStickX, -item.second.threshold, mapping);
					break;

				case AnalogButtons::LeftStickRight:
					sendKeysOnThreshold(AnalogButtons::LeftStickRight, leftStickX, prevLeftStickX, item.second.threshold, mapping);
					break;

				case AnalogButtons::LeftStickUp:
					sendKeysOnThreshold(AnalogButtons::LeftStickUp, leftStickY, prevLeftStickY, item.second.threshold, mapping);
					break;

				case AnalogButtons::LeftStickDown:
					sendKeysOnThreshold(AnalogButtons::LeftStickDown, leftStickY, prevLeftStickY, -item.second.threshold, mapping);
					break;

				case AnalogButtons::RightStickLeft:
					sendKeysOnThreshold(AnalogButtons::RightStickLeft, rightStickX, prevRightStickX, -item.second.threshold, mapping);
					break;

				case AnalogButtons::RightStickRight:
					sendKeysOnThreshold(AnalogButtons::RightStickRight, rightStickX, prevRightStickX, item.second.threshold, mapping);
					break;

				case AnalogButtons::RightStickUp:
					sendKeysOnThreshold(AnalogButtons::RightStickUp, rightStickY, prevRightStickY, item.second.threshold, mapping);
					break;

				case AnalogButtons::RightStickDown:
					sendKeysOnThreshold(AnalogButtons::RightStickDown, rightStickY, prevRightStickY, -item.second.threshold, mapping);
					break;

				case AnalogButtons::LeftTrigger:
					sendKeysOnThreshold(AnalogButtons::LeftTrigger, leftTrigger, prevLeftTrigger, item.second.threshold, mapping);
					break;

				case AnalogButtons::RightTrigger:
					sendKeysOnThreshold(AnalogButtons::RightTrigger, rightTrigger, prevRightTrigger, item.second.threshold, mapping);
					break;
				}
			}
		}

		return true;
	}
	return false;
}

// Processing of analog item key event dispatch
void Gamepad::sendKeysOnThreshold(AnalogButtons button, float now, float before, float threshold, int key)
{
	// Check whether the item is and was passed the threshold or not
	bool isPressed = (now >= threshold && threshold > 0) || (now <= threshold && threshold < 0);
	bool wasPressed = (before >= threshold && threshold > 0) || (before <= threshold && threshold < 0);

	// If currently pressed
	if (isPressed)
	{
		// Repeat interval calculation
		DWORD now = GetTickCount();
		DWORD last = (analogLastPress.find(button) != analogLastPress.end()?
			analogLastPress.find(button)->second : 0);

		unsigned int ms = analogRepeatMs.find(button)->second;

		// Send message (uses same logic as digital buttons)
		if ((now - last >= ms && ms > 0) || last == 0 || (ms == 0 && !wasPressed))
		{
			analogLastPress.erase(button);
			analogLastPress.insert(std::map<AnalogButtons, DWORD>::value_type(button, now));

			if (!onGamepadPress)
				SendMessage(targetWindow, WM_KEYDOWN, key, (wasPressed? 1 << 30 : 0 << 30));
			else
				onGamepadPress(key, wasPressed);
		}
	}
	
	// Same logic as digital buttons
	if (previous.dwPacketNumber < state.dwPacketNumber)
		if (!isPressed && wasPressed)
		{
			analogLastPress.erase(button);

			if (!onGamepadRelease)
				SendMessage(targetWindow, WM_KEYUP, key, 0);
			else
				onGamepadRelease(key);
		}
}

bool Gamepad::IsPressed(WORD button)
{
	return (state.Gamepad.wButtons & button) == button;
}

void Gamepad::SetDeadzone(float x, float y)
{
	deadzoneX = x;
	deadzoneY = y;
}

void Gamepad::SetWindow(HWND hwnd)
{
	keyEvents = false;
	targetWindow = hwnd;
}

void Gamepad::EnableKeyEvents(GamepadPressCallback gpc, GamepadReleaseCallback grc)
{
	keyEvents = true;
	targetWindow = Simple2D::App->GetWindow();

	onGamepadPress = gpc;
	onGamepadRelease = grc;
}

void Gamepad::DisableKeyEvents()
{
	keyEvents = false;
	targetWindow = NULL;
}

void Gamepad::AddKeyMapping(WORD button, int key)
{
	keyMap.erase(button);
	keyMap.insert(std::map<WORD, int>::value_type(button, key));
}

void Gamepad::RemoveKeyMapping(int key)
{
	for (auto it = keyMap.begin(); it != keyMap.end(); ++it)
		if (it->second == key)
		{
			keyMap.erase(it);
			break;
		}
}

void Gamepad::RemoveKeyMappingByButton(WORD button)
{
	keyMap.erase(button);
}

void Gamepad::AddAnalogKeyMapping(AnalogButtons button, float threshold, int key)
{
	AnalogMapping a = { threshold, key };

	analogMap.erase(button);
	analogMap.insert(std::make_pair(button, a));
}

void Gamepad::RemoveAnalogKeyMapping(AnalogButtons button)
{
	analogMap.erase(button);
}

void Gamepad::ClearMappings()
{
	keyMap.clear();
	analogMap.clear();
}

void Gamepad::SetRepeatIntervalMsAll(unsigned int ms)
{
	repeatMs.clear();

	for (auto button : Buttons)
		repeatMs.insert(std::map<WORD, unsigned int>::value_type(button.first, ms));

	analogRepeatMs.clear();

	for (int i = OneBeforeStartOfButtons + 1; i < EndOfButtons; i++)
		analogRepeatMs.insert(std::map<AnalogButtons, unsigned int>::value_type((AnalogButtons) i, ms));
}

void Gamepad::SetRepeatIntervalMs(WORD button, unsigned int ms)
{
	repeatMs.erase(button);
	repeatMs.insert(std::map<WORD, unsigned int>::value_type(button, ms));
}

void Gamepad::SetAnalogRepeatIntervalMs(AnalogButtons button, unsigned int ms)
{
	analogRepeatMs.erase(button);
	analogRepeatMs.insert(std::map<AnalogButtons, unsigned int>::value_type(button, ms));
}

void Gamepad::SetConnectionCheckInterval(unsigned int ms)
{
	checkInterval = ms;
}

}