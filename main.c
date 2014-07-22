#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>

HHOOK keyhook = NULL;

TCHAR mapKey(unsigned level, char in) {
  unsigned len=103;
  TCHAR mappingTable[len];
  for(int i=0; i<len; i++)
    mappingTable[i]=0;

  
  switch(level) {
  case 1:
  case 2:
    wcscpy(mappingTable+16, L"xvlcwkhgfqß");
    wcscpy(mappingTable+30, L"uiaeosnrtdy");
    wcscpy(mappingTable+44, L"üöäpzbm,.j");
    break;
  case 3:
    wcscpy(mappingTable+16, L"…_[]^!<>=&");
    wcscpy(mappingTable+30, L"\\/{}*?()-:@");
    wcscpy(mappingTable+44, L"#$|~`+%\"';");
    break;
  case 4:
    wcscpy(mappingTable+16, L"       789-");
    wcscpy(mappingTable+30, L"       456,");
    wcscpy(mappingTable+44, L"       123;");
    break;
  }
  return mappingTable[in];
}


void sendChar(TCHAR key) 
{
      SHORT extinfo =VkKeyScanEx(key,GetKeyboardLayout(0)); 
      KBDLLHOOKSTRUCT keyinfo;
      keyinfo.vkCode=extinfo;
      char modifiers = extinfo >> 8;
      bool shift = ((modifiers & 1) != 0);
      bool alt = ((modifiers & 2) != 0);
      bool ctrl = ((modifiers & 4) != 0);



      if(shift) keybd_event(VK_SHIFT,0,0,0);
      if(alt) keybd_event(VK_MENU,0,0,0); // ALT
      if(ctrl) keybd_event(VK_CONTROL,0,0,0);

      keybd_event(keyinfo.vkCode, keyinfo.scanCode, keyinfo.flags, keyinfo.dwExtraInfo);

      if(shift) keybd_event(VK_SHIFT,0,KEYEVENTF_KEYUP,0);
      if(alt) keybd_event(VK_MENU,0,KEYEVENTF_KEYUP,0); // ALT
      if(ctrl) keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);
}




__declspec(dllexport) LRESULT CALLBACK keyevent(int code, WPARAM wparam, LPARAM lparam)
{
    static bool mod3Pressed=false;
    static bool mod4Pressed=false;
	if(code == HC_ACTION && (wparam == WM_SYSKEYUP || wparam == WM_KEYUP)) {
		KBDLLHOOKSTRUCT keyinfo = *((KBDLLHOOKSTRUCT *)lparam);
    if(keyinfo.scanCode == VK_CAPITAL || keyinfo.scanCode == 541)
        mod3Pressed=false;
    if(keyinfo.scanCode == 43)
        mod4Pressed=false;
  }
	if (code == HC_ACTION && (wparam == WM_SYSKEYDOWN || wparam == WM_KEYDOWN) 
) {
		KBDLLHOOKSTRUCT keyinfo = *((KBDLLHOOKSTRUCT *)lparam);
		DWORD m;

    unsigned level=1; 
    if(mod3Pressed)
      level=3;
    if(mod4Pressed)
      level=4;

    printf("Keyhook: %u, %u %x %s\n", code, keyinfo.scanCode,keyinfo.flags, keyinfo.dwExtraInfo); 
    TCHAR key = mapKey(level, keyinfo.scanCode);
    if(key!=0 && (keyinfo.flags & LLKHF_INJECTED)==0) {
        		printf("%d->%c (level %u)\n", keyinfo.scanCode, key, level);
      sendChar(key);
	    return -1;
    } else if( keyinfo.vkCode == VK_CAPITAL||keyinfo.scanCode==541)  {
      mod3Pressed=true; 
      return -1;  
    } else if( keyinfo.scanCode==43)  {
      mod4Pressed=true; 
      return -1;  
     } else
	    return CallNextHookEx(NULL, code, wparam, lparam);

	}
  else
	return CallNextHookEx(NULL, code, wparam, lparam);
}


DWORD WINAPI driver(void *user)
{
	HINSTANCE base = GetModuleHandle(NULL);
	MSG msg;

	if (!base) {
		if (!(base = LoadLibrary((wchar_t *)user))) {
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

int main(int argc, char *argv[])
{
	HANDLE t;
	DWORD tid;

	t = CreateThread(0, 0, driver, argv[0], 0, &tid);
	if (t) {
		return WaitForSingleObject(t, INFINITE);
	}
	return 0;
}

