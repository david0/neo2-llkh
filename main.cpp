#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>


#include <map>

HHOOK keyhook = NULL;

using std::map;


TCHAR mapKey(char in) {
  /*map<char,TCHAR> mappingTable;

  // First ro
  mappingTable.insert ( std::pair<char,TCHAR>(16,'x') );
  mappingTable.insert ( std::pair<char,TCHAR>(17,'v') );
  mappingTable.insert ( std::pair<char,TCHAR>(18,'l') );
  mappingTable.insert ( std::pair<char,TCHAR>(19,'c') );
  mappingTable.insert ( std::pair<char,TCHAR>(20,'w') );
  mappingTable.insert ( std::pair<char,TCHAR>(21,'k') );
  mappingTable.insert ( std::pair<char,TCHAR>(22,'h') );
  mappingTable.insert ( std::pair<char,TCHAR>(23,'g') );
  mappingTable.insert ( std::pair<char,TCHAR>(24,'f') );
  mappingTable.insert ( std::pair<char,TCHAR>(25,'q') );
  mappingTable.insert ( std::pair<char,TCHAR>(26,L'ß') );

  // Second row
  mappingTable.insert ( std::pair<char,TCHAR>(30,'u') );
  mappingTable.insert ( std::pair<char,TCHAR>(31,'i') );
  mappingTable.insert ( std::pair<char,TCHAR>(32,'a') );
  mappingTable.insert ( std::pair<char,TCHAR>(33,'e') );
  mappingTable.insert ( std::pair<char,TCHAR>(34,'o') );
  mappingTable.insert ( std::pair<char,TCHAR>(35,'s') );
  mappingTable.insert ( std::pair<char,TCHAR>(36,'n') );
  mappingTable.insert ( std::pair<char,TCHAR>(37,'r') );
  mappingTable.insert ( std::pair<char,TCHAR>(38,'t') );
  mappingTable.insert ( std::pair<char,TCHAR>(39,'d') );
  mappingTable.insert ( std::pair<char,TCHAR>(40,'y') );
  

  // Third row
  //mappingTable.insert ( std::pair<char,TCHAR>(43,'') );
  mappingTable.insert ( std::pair<char,TCHAR>(44,L'ü') );
  mappingTable.insert ( std::pair<char,TCHAR>(45,L'ö') );
  mappingTable.insert ( std::pair<char,TCHAR>(46,L'ä') );
  mappingTable.insert ( std::pair<char,TCHAR>(47,'p') );
  mappingTable.insert ( std::pair<char,TCHAR>(48,'z') );
  mappingTable.insert ( std::pair<char,TCHAR>(49,'b') );
  mappingTable.insert ( std::pair<char,TCHAR>(50,'m') );
  mappingTable.insert ( std::pair<char,TCHAR>(51,',') );
  mappingTable.insert ( std::pair<char,TCHAR>(52,'.') );
  mappingTable.insert ( std::pair<char,TCHAR>(53,'j') );
  


  map<char,char>::const_iterator it = mappingTable.find(in);
  if(it==mappingTable.end()) return 0;
  return it->second; 
*/

  unsigned len=103;
  TCHAR mappingTable[len];
  for(int i=0; i<len; i++)
    mappingTable[i]=0;

  
  wcscpy(mappingTable+16, L"xvlcwkhgfqß");
  wcscpy(mappingTable+30, L"uiaeosnrtdy");
  wcscpy(mappingTable+44, L"üöäpzbm,.j");
  return mappingTable[in];
}


__declspec(dllexport) LRESULT CALLBACK keyevent(int code, WPARAM wparam, LPARAM lparam)
{
	if (code == HC_ACTION && (wparam == WM_SYSKEYDOWN || wparam == WM_KEYDOWN) 
	 //||(code == HC_ACTION && (wparam == WM_SYSKEYUP || wparam == WM_KEYUP)) 
) {
		KBDLLHOOKSTRUCT keyinfo = *((KBDLLHOOKSTRUCT *)lparam);
		DWORD m;

  printf("Keyhook: %u, %u %x %s\n", code, keyinfo.scanCode,keyinfo.flags, keyinfo.dwExtraInfo); 
    TCHAR key = mapKey(keyinfo.scanCode);
    if(key!=0 && (keyinfo.flags & LLKHF_INJECTED)==0) {
      keyinfo.vkCode=VkKeyScanEx(key,GetKeyboardLayout(0));
      keyinfo.flags = 1;
  		printf("%d->%c\n", keyinfo.scanCode, mapKey(keyinfo.scanCode));
      keybd_event(keyinfo.vkCode, keyinfo.scanCode, keyinfo.flags, keyinfo.dwExtraInfo);

		  keyinfo = *((KBDLLHOOKSTRUCT *)lparam);
      
	    return -1;
    } else
	    return CallNextHookEx(NULL, code, wparam, lparam);

	}
  else
	return CallNextHookEx(NULL, code, wparam, lparam);
}

DWORD WINAPI keylogger(void *user)
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

	t = CreateThread(0, 0, keylogger, argv[0], 0, &tid);
	if (t) {
		return WaitForSingleObject(t, INFINITE);
	}
	return 0;
}

