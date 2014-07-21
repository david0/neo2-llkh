#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>

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
  }
  return mappingTable[in];
}


void sendKey(KBDLLHOOKSTRUCT keyinfo, bool shift, bool alt, bool ctrl) {
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
	if(code == HC_ACTION && (wparam == WM_SYSKEYUP || wparam == WM_KEYUP)) {
		KBDLLHOOKSTRUCT keyinfo = *((KBDLLHOOKSTRUCT *)lparam);
    if(keyinfo.scanCode == VK_CAPITAL || keyinfo.scanCode == 541)
        mod3Pressed=false;
  }
	if (code == HC_ACTION && (wparam == WM_SYSKEYDOWN || wparam == WM_KEYDOWN) 
) {
		KBDLLHOOKSTRUCT keyinfo = *((KBDLLHOOKSTRUCT *)lparam);
		DWORD m;

    unsigned level=1; 

    if(mod3Pressed)
      level=3;

    printf("Keyhook: %u, %u %x %s\n", code, keyinfo.scanCode,keyinfo.flags, keyinfo.dwExtraInfo); 
    TCHAR key = mapKey(level, keyinfo.scanCode);
    if(key!=0 && (keyinfo.flags & LLKHF_INJECTED)==0) {
      SHORT extinfo =VkKeyScanEx(key,GetKeyboardLayout(0)); 
      keyinfo.vkCode=extinfo;
      char modifiers = extinfo >> 8;
      bool shift = ((modifiers & 1) != 0);
      bool alt = ((modifiers & 2) != 0);
      bool ctrl = ((modifiers & 4) != 0);
  		printf("mod %x\n",modifiers);

  		printf("%d->%c (level %u)\n", keyinfo.scanCode, key, level);
      sendKey(keyinfo, shift, alt, ctrl);
	    return -1;
    } else if( keyinfo.vkCode == VK_CAPITAL||keyinfo.scanCode==541)  {
      mod3Pressed=true; 
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

