#pragma once
#include <windows.h>
#include "raytracer_math.h"

#define RT_RGB(r,g,b) (RGB(b,g,r))              // COLORREF are handled in different byte-order
#define RT_GetRValue(a) (GetBValue(a))
#define RT_GetGValue(a) (GetGValue(a))
#define RT_GetBValue(a) (GetRValue(a)) 

void StartRaytracer(HWND wndHandle, int wWidth, int wHeight);

typedef struct {
	Vector3 cnt;
	float radius;
	COLORREF clr;
}Sphere;

/*
when ambient - pos has no meaning and can be ignored;
when directional - pos is the directional vector of the light;
*/
typedef struct {
	LightType type;
	float ins;
	Vector3 pos;
}Light;

typedef struct {
	float vwpSize;
	float prjPlaneZ;
	Vector3 cmrPos;
	COLORREF bgClr;
	Sphere* objs;
	Light* lights;
	int objCount;
	int lightCount;
}Scene;



Scene mainScn;