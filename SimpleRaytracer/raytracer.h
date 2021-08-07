#pragma once
#include <windows.h>
#include "raytracer_math.h"

#define RT_RGB(r,g,b) (RGB(b,g,r))              // COLORREF are handled in different byte-order

void StartRaytracer(HWND wndHandle, int wWidth, int wHeight);


typedef struct {
	Vector3 cnt;
	float radius;
	COLORREF clr;
}Sphere;

typedef struct {
	float vwpSize;
	float prjPlaneZ;
	Vector3 cmrPos;
	COLORREF bgClr;
	Sphere* objs;
	int objCount;
}Scene;

Scene mainScn;