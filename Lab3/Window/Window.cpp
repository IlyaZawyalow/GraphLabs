#include "Window.h"

#include <iostream>
#include "../ImGUI/imgui.h"
#include "../ImGUI/imgui_impl_win32.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	Window* window;
	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	try {
		switch (msg)
		{
		case WM_CREATE:
		{

			window = (Window*)((LPCREATESTRUCT)lparam)->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
			break;
		}
		case WM_SIZE:
			window = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			window->width = LOWORD(lparam);
			window->height = HIWORD(lparam);
			window->checkResizeCallbacks();
			break;
		case WM_DESTROY:
		{
			window = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			window->needToClose = true;
			::PostQuitMessage(0);
			break;
		}


		default:
			window = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			window->checkEvent(hwnd, msg, wparam, lparam);
			return ::DefWindowProc(hwnd, msg, wparam, lparam);
		}
	}
	catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
	}
	

	return NULL;
}


Window* Window::createWindow(HINSTANCE instance, uint32_t width, uint32_t height, const wchar_t* windowTitle) {
	WNDCLASSEX wc;
	wc.cbClsExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = NULL;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = instance;
	wc.lpszClassName = L"MyWindowClass";
	wc.lpszMenuName = L"";
	wc.style = NULL;
	wc.lpfnWndProc = &WndProc;
	
	auto result = new Window(instance);
	if (!::RegisterClassEx(&wc))
		throw std::runtime_error("Failed to register window class");

	HWND windowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"MyWindowClass", windowTitle,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, NULL, result);
	if (!windowHandle)
		throw std::runtime_error("Failed to create window");
	result->windowHandle = windowHandle;
	result->width = width;
	result->height = height;
	result->title = windowTitle;
	result->instance = instance;
	result->inputSystem = new WindowInputSystem(instance, windowHandle);
	ShowWindow(windowHandle, SW_SHOW);
	UpdateWindow(windowHandle);
	result->windowReady = true;
	return result;
}

Window::Window(HINSTANCE instance) : instance(instance){}

void Window::pollEvents() {
	MSG msg;


	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::addResizeCallback(void(*resizeCallback)(uint32_t, uint32_t)) {
	resizeCallbacks.push_back(resizeCallback);
}

HWND Window::getWindowHandle() {
	return windowHandle;
}

bool Window::isNeedToClose() {
	return needToClose;
}

uint32_t Window::getWidth() {
	return width;
}
uint32_t Window::getHeight() {
	return height;
}

WindowInputSystem* Window::getInputSystem()
{
	return inputSystem;
}

void Window::checkResizeCallbacks() {
	for (auto& item : resizeCallbacks) {
		item(width, height);
	}
}

void Window::checkEvent(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (this && windowReady) {
		inputSystem->handlePollEvents(hwnd, msg, wparam, lparam);
	}
	
}
