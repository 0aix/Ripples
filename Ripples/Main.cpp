#include "stdafx.h"

#define WS_DEFAULT ((WS_OVERLAPPEDWINDOW | WS_SYSMENU) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME))
#define CLASSNAME "Ripples"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = CLASSNAME;
	wcex.hIconSm = NULL;

	if (!RegisterClassEx(&wcex))
		return 1;

	RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&rect, WS_DEFAULT, false);

	HWND hwnd = CreateWindow(CLASSNAME, CLASSNAME, WS_DEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

	if (!hwnd)
		return 1;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg = { 0 };

	if (Ripples::Initialize(hwnd))
	{
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg); //Translates VK to WM_CHAR 
			DispatchMessageA(&msg); //Sends message to Window Proc
		}
		Ripples::Uninitialize();
	}
	UnregisterClass(CLASSNAME, hInstance);

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		Ripples::Ripple(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 100000.0f);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		EndPaint(hwnd, NULL);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}