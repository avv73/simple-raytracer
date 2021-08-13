#define UNICODE
#define _UNICODE
#include <windows.h>
#include "raytracer.h"
#include <stdio.h>
#include <strsafe.h>


// Window height properties.

const int WINDOW_HEIGHT = 600;
const int WINDOW_WIDTH = 600;

int isMinimized = 0;


// Custom configuration of the scene to render.

void ConfigureScene() {
	mainScn.bgClr = RT_RGB(0, 0, 0);

	mainScn.cmrPos.x = 0;
	mainScn.cmrPos.y = 0;
	mainScn.cmrPos.z = 0;

	//mainScn.rotMatrix = ComposeRotation(RotateZ(40), RotateX(30));
	mainScn.rotMatrix = RotateX(0);

	mainScn.vwpSize = 1;
	mainScn.prjPlaneZ = 1;

	Vector3 sp1Cnt = { 0, -1, 3 };
	Vector3 sp2Cnt = { 2, 0, 4 };
	Vector3 sp3Cnt = { -2, 0, 4 };
	Vector3 sp4Cnt = { 0, -5001, 0 };

	Sphere sp1 = { sp1Cnt, 1, 500, 0.2, RT_RGB(255, 0, 0) };
	Sphere sp2 = { sp2Cnt, 1, 500, 0.3, RT_RGB(0, 0, 255) };
	Sphere sp3 = { sp3Cnt, 1, 10, 0.4, RT_RGB(0, 255, 0) };
	Sphere sp4 = { sp4Cnt, 5000, 1000, 0.5, RT_RGB(255,255,0) };

	Sphere* spheres = (Sphere*)malloc(sizeof(Sphere) * 4);
	spheres[0] = sp1;
	spheres[1] = sp2;
	spheres[2] = sp3;
	spheres[3] = sp4;

	mainScn.sphs = spheres;
	mainScn.sphCount = 4;

	Triangle tr1 = { sp1Cnt, sp2Cnt, sp3Cnt, 500, 0.3, RT_RGB(255,0,255) };

	mainScn.trs = (Triangle*)malloc(sizeof(Triangle) * 1);
	mainScn.trs[0] = tr1;
	mainScn.trCount = 1;

	Vector3 l2P = { 2,1,0 };
	Vector3 l3P = { 1,4,4 };

	Light l1 = { AMBIENT, 0.2 };
	Light l2 = { POINTED, 0.6, l2P };
	Light l3 = { DIRECTIONAL, 0.2, l3P };

	Light* lights = (Light*)malloc(sizeof(Light) * 3);
	lights[0] = l1;
	lights[1] = l2;
	lights[2] = l3;

	mainScn.lights = lights;
	mainScn.lightCount = 3;
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp);

const TCHAR CLSNAME[] = TEXT("Simple Raytracer");

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdline, int cmdshow)
{
	WNDCLASSEX wc; // = { };
	MSG msg;
	HWND hwnd;

	wc.cbSize = sizeof(wc);
	wc.style = 0;
	wc.lpfnWndProc = WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = CLSNAME;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, TEXT("Could not register window class"),
			NULL, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_LEFT,
		CLSNAME,
		CLSNAME,
		WS_MINIMIZEBOX | WS_SYSMENU, // WS_OVERLAPPEDWINDOW
		0,
		0,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		NULL,
		NULL,
		hInst,
		NULL);
	if (!hwnd) {
		MessageBox(NULL, TEXT("Could not create window"), NULL, MB_ICONERROR);
		return 0;
	}

	ShowWindow(hwnd, cmdshow);

	UpdateWindow(hwnd);

	ConfigureScene();

	StartRaytracer(hwnd, WINDOW_WIDTH, WINDOW_HEIGHT);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
	int xPos;
	int yPos;

	switch (wm) {
	case WM_DESTROY:
		exit(0);
		return 0;
	case WM_EXITSIZEMOVE:
		xPos = (int)(short)LOWORD(lp);
		yPos = (int)(short)HIWORD(lp);

		POINT p1;
		p1.x = xPos;
		p1.y = yPos;

		POINT p2;
		p2.x = xPos + WINDOW_WIDTH;
		p2.y = yPos;

		POINT p3;
		p3.x = xPos;
		p3.y = yPos + WINDOW_HEIGHT;

		POINT p4;
		p4.x = xPos + WINDOW_WIDTH;
		p4.y = yPos + WINDOW_HEIGHT;

		if (MonitorFromPoint(p1, MONITOR_DEFAULTTONULL) != NULL ||
			MonitorFromPoint(p2, MONITOR_DEFAULTTONULL) != NULL ||
			MonitorFromPoint(p3, MONITOR_DEFAULTTONULL) != NULL ||
			MonitorFromPoint(p4, MONITOR_DEFAULTTONULL) != NULL) {
			// rerender if window goes off screen
			StartRaytracer(hwnd, WINDOW_WIDTH, WINDOW_HEIGHT);
		}

		break;
	case WM_SIZE:
		if (wp == SIZE_MINIMIZED) {
			isMinimized = 1;
		}

		else if (wp == SIZE_RESTORED && isMinimized) {
			StartRaytracer(hwnd, WINDOW_WIDTH, WINDOW_HEIGHT);
			isMinimized = 0;
		}
		break;
	case WM_RBUTTONDOWN:
		// print mouse position on right click

		xPos = (int)(short)LOWORD(lp);
		yPos = (int)(short)HIWORD(lp);

		int xPosC = xPos - WINDOW_WIDTH / 2;
		int yPosC = -yPos + WINDOW_HEIGHT / 2 - 1;

		TCHAR txt[100];

		TCHAR header[] = TEXT("Information");
		LPCSTR textPlc = TEXT("Screen coordinates X: %d  Y: %d\nCanvas coordinates X: %d Y: %d");

		StringCbPrintf(txt, 100 * sizeof(TCHAR), textPlc, xPos, yPos, xPosC, yPosC);

		MessageBox(NULL, txt, header, NULL);

		break;
	}
	return DefWindowProc(hwnd, wm, wp, lp);
}