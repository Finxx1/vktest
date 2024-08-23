#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

#include <stdio.h>
#include <stdbool.h>

#define VOLK_IMPLEMENTATION
#define VK_USE_PLATFORM_WIN32_KHR
#include "volk.h"
#include "vk.c"

bool vk_finished_init = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE: {
			vk_init(hWnd, GetModuleHandle(NULL), 640, 360);
			vk_finished_init = true;
		} break;
		case WM_SIZE: {
			vk_update_swapchain(LOWORD(lParam), HIWORD(lParam));
		} break;
		case WM_DESTROY: {
			vk_cleanup();
			vk_finished_init = false;
			PostQuitMessage(0);
		} break;
	}
	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int main() {
	WNDCLASSA wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "vktest";
	wc.hInstance = GetModuleHandle(NULL);
	RegisterClassA(&wc);
	
	RECT clientrect = {0, 0, 640, 360};
	AdjustWindowRect(&clientrect, WS_OVERLAPPEDWINDOW, FALSE);
	int w = clientrect.right - clientrect.left;
	int h = clientrect.bottom - clientrect.top;
	
	HWND hWnd = CreateWindowA("vktest", "vktest", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, NULL, NULL, NULL, NULL);
	
	ShowWindow(hWnd, 1);
	
	MSG msg = {0};
	while (msg.message != WM_QUIT) {
		if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		if (vk_finished_init) vk_draw();
	}
	
	return 0;
}