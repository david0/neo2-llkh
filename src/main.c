#define UNICODE
/**
 * Alternative Windows driver for the Neo2-Keyboard layout (www.neo-layout.org)
 */

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>
#include "trayicon.h"
#include "resources.h"

HHOOK keyhook = NULL;
#define APPNAME "neo2-llkh"

/**
 * Map a key scancode to the char that should be displayed after typing
 **/
TCHAR mapKey(unsigned level, char in)
{
	unsigned len = 103;
	TCHAR mappingTable[len];
	for (int i = 0; i < len; i++)
		mappingTable[i] = 0;

	switch (level) {
	case 1:
		wcscpy(mappingTable + 16, L"xvlcwkhgfqß");
		wcscpy(mappingTable + 30, L"uiaeosnrtdy");
		wcscpy(mappingTable + 44, L"üöäpzbm,.j");
		break;
	case 2:
		wcscpy(mappingTable + 2, L"°§ℓ»«$€„“”—");
		wcscpy(mappingTable + 16, L"XVLCWKHGFQSS");
		wcscpy(mappingTable + 30, L"UIAEOSNRTDY");
		wcscpy(mappingTable + 44, L"ÜÖÄPZBM–•J");
		break;
	case 3:
		wcscpy(mappingTable + 2, L"¹²³›‹¢¥‚‘’‐");
		wcscpy(mappingTable + 16, L"…_[]^!<>=&");
		wcscpy(mappingTable + 30, L"\\/{}*?()-:@");
		wcscpy(mappingTable + 44, L"#$|~`+%\"';");
		break;
	case 4:
		wcscpy(mappingTable + 2, L"ªº№⋮·£¤\0/*-");
		wcscpy(mappingTable + 21, L"¡789+−");
		wcscpy(mappingTable + 35, L"¿456,.");
		wcscpy(mappingTable + 49, L":123;");
		break;
	}
	return mappingTable[in];
}

/**
 * Sends a char using emulated keyboard input
 *
 * This works for most cases, but not for dead keys etc
 **/
void sendChar(TCHAR key, KBDLLHOOKSTRUCT keyInfo)
{
	SHORT keyScanResult = VkKeyScanEx(key, GetKeyboardLayout(0));
	keyInfo.vkCode = keyScanResult;
	char modifiers = keyScanResult >> 8;
	bool shift = ((modifiers & 1) != 0);
	bool alt = ((modifiers & 2) != 0);
	bool ctrl = ((modifiers & 4) != 0);

	if (shift)
		keybd_event(VK_SHIFT, 0, 0, 0);
	if (alt)
		keybd_event(VK_MENU, 0, 0, 0);	// ALT
	if (ctrl)
		keybd_event(VK_CONTROL, 0, 0, 0);

	keybd_event(keyInfo.vkCode, keyInfo.scanCode, keyInfo.flags, keyInfo.dwExtraInfo);

	if (shift)
		keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
	if (alt)
		keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);	// ALT
	if (ctrl)
		keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
}

bool handleLayer4SpecialCases(KBDLLHOOKSTRUCT keyInfo)
{
	unsigned len = 103;
	CHAR mappingTable[len];
	for (int i = 0; i < len; i++)
		mappingTable[i] = 0;

	mappingTable[17] = VK_BACK;
	mappingTable[18] = VK_UP;
	mappingTable[19] = VK_DELETE;
	mappingTable[31] = VK_LEFT;
	mappingTable[32] = VK_DOWN;
	mappingTable[33] = VK_RIGHT;
	mappingTable[44] = VK_ESCAPE;
	mappingTable[45] = VK_TAB;
	mappingTable[47] = VK_RETURN;

	if (mappingTable[keyInfo.scanCode] != 0) {
		keybd_event(mappingTable[keyInfo.scanCode], 0, 0, 0);
		return true;
	}
	return false;
}

bool isShift(KBDLLHOOKSTRUCT keyInfo)
{
	return keyInfo.vkCode == VK_SHIFT || keyInfo.vkCode == VK_LSHIFT
	    || keyInfo.vkCode == VK_RSHIFT;
}

bool isMod3(KBDLLHOOKSTRUCT keyInfo)
{
	return keyInfo.vkCode == VK_CAPITAL || keyInfo.scanCode == 541 || keyInfo.scanCode == 43;
}

bool isMod4(KBDLLHOOKSTRUCT keyInfo)
{
	return keyInfo.vkCode == VK_RMENU || keyInfo.vkCode == VK_CAPITAL;	//keyInfo.scanCode == 43 || keyInfo.scanCode == 86;
}


void logKeyEvent(char* desc, KBDLLHOOKSTRUCT keyInfo) {
	printf("%-10s sc %u vk 0x%x 0x%x %d\n", desc, keyInfo.scanCode, keyInfo.vkCode,
	       keyInfo.flags, keyInfo.dwExtraInfo);
}

__declspec(dllexport)
LRESULT CALLBACK keyevent(int code, WPARAM wparam, LPARAM lparam)
{
	static bool shiftPressed = false;
	static bool mod3Pressed = false;
	static bool mod4Pressed = false;

	KBDLLHOOKSTRUCT keyInfo;
	if (code == HC_ACTION
	    && (wparam == WM_SYSKEYUP || wparam == WM_KEYUP || wparam == WM_SYSKEYDOWN
		|| wparam == WM_KEYDOWN)) {
		keyInfo = *((KBDLLHOOKSTRUCT *) lparam);
		if (keyInfo.flags & LLKHF_INJECTED)	// process injected events like normal, because most probably we are injecting them
			return CallNextHookEx(NULL, code, wparam, lparam);
	}

	if (code == HC_ACTION && (wparam == WM_SYSKEYUP || wparam == WM_KEYUP)) {
		logKeyEvent("key up", keyInfo);

		if (isShift(keyInfo)) {
			shiftPressed = false;
			keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
			return -1;
		} else if (isMod3(keyInfo)) {
			mod3Pressed = false;
			return -1;
		} else if (isMod4(keyInfo)) {
			mod4Pressed = false;
			return -1;
		}
	}

	else if (code == HC_ACTION && (wparam == WM_SYSKEYDOWN || wparam == WM_KEYDOWN)) {
		logKeyEvent("key down", keyInfo);

		unsigned level = 1;
		if (shiftPressed)
			level = 2;
		if (mod3Pressed)
			level = 3;
		if (mod4Pressed)
			level = 4;

		if (isShift(keyInfo)) {
			shiftPressed = true;
			keybd_event(VK_SHIFT, 0, 0, 0);
			return -1;
		} else if (isMod3(keyInfo)) {
			mod3Pressed = true;
			return -1;
		} else if (isMod4(keyInfo)) {
			mod4Pressed = true;
			return -1;
		} else if (level == 4 && handleLayer4SpecialCases(keyInfo)) {
			return -1;
		} else {
			TCHAR key = mapKey(level, keyInfo.scanCode);
			if (key != 0 && (keyInfo.flags & LLKHF_INJECTED) == 0) {
				// if key must be mapped
				printf("Mapped %d->%c (level %u)\n", keyInfo.scanCode, key, level);
				//BYTE state[256];
				//GetKeyboardState(state);
				sendChar(key, keyInfo);
				//SetKeyboardState(state);
				return -1;
			}
		}
	}
	return CallNextHookEx(NULL, code, wparam, lparam);
}

DWORD WINAPI hookThreadMain(void *user)
{
	HINSTANCE base = GetModuleHandle(NULL);
	MSG msg;

	if (!base) {
		if (!(base = LoadLibrary((wchar_t *) user))) {
			return 1;
		}
	}

	keyhook = SetWindowsHookEx(WH_KEYBOARD_LL, keyevent, base, 0);

	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(keyhook);

	return 0;
}

void exitApplication()
{
  trayicon_remove();
  PostQuitMessage(0);
}

int main(int argc, char *argv[])
{
	DWORD tid;

	HINSTANCE hInstance = GetModuleHandle(NULL);
  trayicon_init(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON)), APPNAME);
  //trayicon_add_item(NULL, &toggleWindowVisible);
  trayicon_add_item("Exit", &exitApplication);


	HANDLE thread = CreateThread(0, 0, hookThreadMain, argv[0], 0, &tid);
	if (thread) {
		return WaitForSingleObject(thread, INFINITE);
	}
	return 0;
}
