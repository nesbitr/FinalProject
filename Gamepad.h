/* Simple2D Graphics Library for C++
   (c) Katy Coe (sole proprietor) 2012-2013

   No unauthorized copying or distribution. You may re-use Simple2D in your own non-commercial projects.
   For commercial applications, you must obtain a commercial license.

   Support and documentation: www.djkaty.com/simple2d
*/

/* XInput support */

#include <Windows.h>
#include <Xinput.h>
#include <algorithm>
#include <boost/assign.hpp>
#include <string>
#include <map>

using namespace boost::assign;
using std::string;

namespace S2D
{
// Gamepad dispatch functions
typedef std::function<void (unsigned int, bool)> GamepadPressCallback;
typedef std::function<void (unsigned int)> GamepadReleaseCallback;

// The gamepad support class
class Gamepad
{
public:
	// =========================================================================
	// Public usable sub-types
	// =========================================================================

	// Names for each axis of movement for each analog item
	enum AnalogButtons {
		OneBeforeStartOfButtons = 0xff00,
		LeftStickLeft,
		LeftStickRight,
		LeftStickUp,
		LeftStickDown,
		RightStickLeft,
		RightStickRight,
		RightStickUp,
		RightStickDown,
		LeftTrigger,
		RightTrigger,
		EndOfButtons
	};

	// Analog keyboard mappings have a percentage threshold and a target keyboard key
	struct AnalogMapping {
		float threshold;
		int key;
	};

private:
	// The port on which the controller is connected (0-3)
	int cId;

	// The last retrieved state of the controller
	XINPUT_STATE state;

	// The window to send keyboard events to (or NULL for none)
	HWND targetWindow;

	// The X-axis analog stick deadzone (0-1)
	float deadzoneX;

	// The Y-axis analog stick deadzone (0-1)
	float deadzoneY;

	// True to enable keyboard event dispatch, false to disable (false by default)
	bool keyEvents;

	// Callback functions (sends to Windows message pump otherwise)
	GamepadPressCallback onGamepadPress;
	GamepadReleaseCallback onGamepadRelease;

	// Mapping of controller buttons to keys
	std::map<WORD, int> keyMap;

	// Mapping of analog controller items to keys
	std::map<AnalogButtons, AnalogMapping> analogMap;

	// Repeat rate of generated key events from controller buttons
	std::map<WORD, unsigned int> repeatMs;

	// Repeat rate of generated key events from analog controller items
	std::map<AnalogButtons, unsigned int> analogRepeatMs;

	// The GetTickCount() of when each button was last pressed
	std::map<WORD, DWORD> lastPress;

	// The GetTickCount() of when each analog item was pressed (passed the threshold)
	std::map<AnalogButtons, DWORD> analogLastPress;

	// Configure the controller button names. Internal use only.
	void setButtons()
	{
		std::map<WORD, string> bn = map_list_of
			(XINPUT_GAMEPAD_A, "A")
			(XINPUT_GAMEPAD_B, "B")
			(XINPUT_GAMEPAD_X, "X")
			(XINPUT_GAMEPAD_Y, "Y")
			(XINPUT_GAMEPAD_DPAD_LEFT, "Left")
			(XINPUT_GAMEPAD_DPAD_RIGHT, "Right")
			(XINPUT_GAMEPAD_DPAD_UP, "Up")
			(XINPUT_GAMEPAD_DPAD_DOWN, "Down")
			(XINPUT_GAMEPAD_LEFT_SHOULDER, "LB")
			(XINPUT_GAMEPAD_RIGHT_SHOULDER, "RB")
			(XINPUT_GAMEPAD_BACK, "Back")
			(XINPUT_GAMEPAD_START, "Start")
			(XINPUT_GAMEPAD_LEFT_THUMB, "LS")
			(XINPUT_GAMEPAD_RIGHT_THUMB, "RS")
			(LeftStickLeft, "LS")
			(LeftStickRight, "LS")
			(LeftStickUp, "LS")
			(LeftStickDown, "LS")
			(RightStickLeft, "RS")
			(RightStickRight, "RS")
			(RightStickUp, "RS")
			(RightStickDown, "RS")
			(LeftTrigger, "LT")
			(RightTrigger, "RT");

		Buttons.insert(bn.begin(), bn.end());
	}

	// The previous state of the controller
	XINPUT_STATE previous;

	// The previous state of the analog sticks and triggers
	float prevLeftStickX;
	float prevLeftStickY;
	float prevRightStickX;
	float prevRightStickY;
	float prevLeftTrigger;
	float prevRightTrigger;

	// The last time the connection to the controller was checked
	unsigned int lastConnectionCheck;

	// The connection polling interval
	unsigned int checkInterval;

	// Internal use only
	void sendKeysOnThreshold(AnalogButtons, float, float, float, int);

public:
	// =========================================================================
	// Public methods and properties
	// =========================================================================

	// Enable gamepad support
	Gamepad() : deadzoneX(0.05f), deadzoneY(0.02f), targetWindow(NULL), keyEvents(false), lastConnectionCheck(0), checkInterval(2000) { setButtons(); SetRepeatIntervalMsAll(0); }

	// Enable gamepad support supplying default X and Y-axis deadzones
	Gamepad(float dzX, float dzY) : deadzoneX(dzX), deadzoneY(dzY), targetWindow(NULL), keyEvents(false), lastConnectionCheck(0), checkInterval(2000) { setButtons(); SetRepeatIntervalMsAll(0); }

	// A map of XINPUT_GAMEPAD_* button IDs to string button names
	std::map<WORD, string> Buttons;

	// The current state of the analog sticks and triggers (movement amount from 0-1)
	float leftStickX;
	float leftStickY;
	float rightStickX;
	float rightStickY;
	float leftTrigger;
	float rightTrigger;

	// Get the port on which the active controller is plugged in (1-4)
	int  GetPort();

	// Get the current state of the controller (not normally needed)
	XINPUT_GAMEPAD *GetState();

	// Try to establish a connection with the controller (returns true if succeeded)
	bool CheckConnection();

	// Refresh the state of the controller. Call once per frame (calls CheckConnection())
	bool Refresh();

	// Returns true if the specified XINPUT_GAMEPAD_* button is pressed
	bool IsPressed(WORD);

	// Set the X and Y-axis analog stick deadzones
	void SetDeadzone(float, float);

	// Set the target window to receive key events and enable them
	void SetWindow(HWND);

	// Enable key events and send them to the application's main window
	// (if no arguments supplied) or the specified dispatch function
	void EnableKeyEvents(GamepadPressCallback = nullptr, GamepadReleaseCallback = nullptr);

	// Disable key events
	void DisableKeyEvents();

	// Add a key translation mapping from XINPUT_GAMEPAD_* to a virtual key code (VK_*)
	void AddKeyMapping(WORD, int);

	// Remove a key translation mapping from XINPUT_GAMEPAD_* button
	void RemoveKeyMappingByButton(WORD);

	// Remove a key translation mapping from a virtual key code (VK_*)
	void RemoveKeyMapping(int);

	// Add a key translation mapping from an analog item moved more than the specified threshold to a virtual key code (VK_*)
	void AddAnalogKeyMapping(AnalogButtons, float, int);

	// Remove a key translation mapping from an analog item
	void RemoveAnalogKeyMapping(AnalogButtons);

	// Remove all digital and analog key translation mappings
	void ClearMappings();

	// Set the global keyboard repeat interval for all buttons and analog items on the controller - overwrites any previous settings
	void SetRepeatIntervalMsAll(unsigned int);

	// Set the keyboard repeat interval for the specified XINPUT_GAMEPAD_* button in milliseconds
	void SetRepeatIntervalMs(WORD, unsigned int);

	// Set the keyboard repeat interval for the specified analog item in milliseconds
	void SetAnalogRepeatIntervalMs(AnalogButtons, unsigned int);

	// Set the connection check interval in milliseconds
	void SetConnectionCheckInterval(unsigned int);
};

}
