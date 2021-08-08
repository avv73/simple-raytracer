#include "raytracer.h"

#include <windows.h>
#include <math.h>

#define FLT_MAX          3.402823466e+38F        // max value of float

int RT_WINDOW_WIDTH;
int RT_WINDOW_HEIGHT;

COLORREF* frmBuffer = NULL;

void Draw();

void Update(HWND wndHandle);

// Initializes the raytracer and renders the scene provided.

void StartRaytracer(HWND wndHandle, int wWidth, int wHeight) {
	RT_WINDOW_WIDTH = wWidth;
	RT_WINDOW_HEIGHT = wHeight;

	if (!frmBuffer) {
		frmBuffer = (COLORREF*)calloc(RT_WINDOW_HEIGHT * RT_WINDOW_WIDTH, sizeof(COLORREF));
		Draw();
	}

	Update(wndHandle);
}

// Updates window handle DC with bitmap of framebuffer

void Update(HWND wndHandle) {
	HDC wndDc = GetDC(wndHandle);

	HBITMAP map = CreateBitmap(RT_WINDOW_WIDTH, RT_WINDOW_HEIGHT, 1, 8 * 4, (void*)frmBuffer);

	HDC src = CreateCompatibleDC(wndDc);
	SelectObject(src, map);

	BitBlt(wndDc,
		0,
		0,
		RT_WINDOW_WIDTH,
		RT_WINDOW_HEIGHT,
		src,
		0,
		0,
		SRCCOPY);

	DeleteDC(src);
	DeleteObject(map); // for now?
}

// Puts pixel on the screen, converts from canvas coordinate system to screen coordinate system itself.

void PutPixel(int x, int y, COLORREF clr) {
	x = RT_WINDOW_WIDTH / 2 + x;
	y = RT_WINDOW_HEIGHT / 2 - y - 1;

	if (x < 0 || x >= RT_WINDOW_WIDTH || y < 0 || y >= RT_WINDOW_HEIGHT) {
		return;
	}

	frmBuffer[RT_WINDOW_HEIGHT * y + x] = clr;
}

// Converts canvas coordinates to viewport coordinates.

Vector3 CanvasToViewport(Vector2 p2d) {
	Vector3 res = { p2d.x * mainScn.vwpSize / RT_WINDOW_WIDTH, p2d.y * mainScn.vwpSize / RT_WINDOW_HEIGHT, mainScn.prjPlaneZ };
	return res;
}

// Computes lighting of a single point using the normal vector of the object in scene and the point itself.
// Returns float in range [0,1] representing the intensity of the light.

float ComputeLighting(Vector3 p, Vector3 n) {
	float res = 0.0f;
	Vector3 direct;

	for (int i = 0; i < mainScn.lightCount; i++) {
		if (mainScn.lights[i].type == AMBIENT) {
			res += mainScn.lights[i].ins;
			continue;
		}
		
		if (mainScn.lights[i].type == POINTED) {
			direct = SubtractVector(mainScn.lights[i].pos, p);
		}
		else {
			direct = mainScn.lights[i].pos;
		}

		float n_dot_l = DotProduct(n, direct);
		if (n_dot_l > 0) {
			res += mainScn.lights[i].ins * n_dot_l / (LengthVector(n) * LengthVector(direct));
		}
	}

	return res;
}


// Intersects a ray (origin and direction) with a sphere. Returns a tuple with the quadratic solution parameters of the intersection.

TTupel IntersectRaySphere(Vector3 orig, Vector3 direct, Sphere sph) {
	TTupel result;
	result.isValid = 1;

	Vector3 odV = SubtractVector(orig, sph.cnt);

	float k1 = DotProduct(direct, direct);
	float k2 = 2 * DotProduct(odV, direct);
	float k3 = DotProduct(odV, odV) - sph.radius * sph.radius;

	float disc = k2 * k2 - 4 * k1 * k3;
	if (disc < 0) {
		result.isValid = 0;
		return result;
	}

	result.t1 = (-k2 + sqrt(disc)) / (2 * k1);
	result.t2 = (-k2 - sqrt(disc)) / (2 * k1);

	return result;
}

// Traces a ray (origin and direction) with all spheres in the scene. Returns the color of the struck sphere (if any).

COLORREF TraceRay(Vector3 orig, Vector3 direct, float minT, float maxT) {
	float closeT = FLT_MAX;
	Sphere* closeSph = NULL;

	for (int i = 0; i < mainScn.objCount; i++) {
		TTupel ts = IntersectRaySphere(orig, direct, mainScn.objs[i]);

		if (!ts.isValid) {
			continue;
		}

		if (ts.t1 < closeT && minT < ts.t1 && ts.t1 < maxT) {
			closeT = ts.t1;
			closeSph = &mainScn.objs[i];
		}

		if (ts.t2 < closeT && minT < ts.t2 && ts.t2 < maxT) {
			closeT = ts.t2;
			closeSph = &mainScn.objs[i];
		}
	}

	if (closeSph == NULL) {
		return mainScn.bgClr;
	}

	Vector3 interP = AddVector(orig, ScaleVector(direct, closeT)); // compute the intersection point by substituting the found parameter in the point equation
	Vector3 sphN = SubtractVector(interP, closeSph->cnt);          // compute sphere normal
	sphN = ScaleVector(sphN, (1 / LengthVector(sphN)));

	// multiply light intensity with sphere's color

	float factor = ComputeLighting(interP, sphN);

	int csR = RT_GetRValue(closeSph->clr) * factor;
	int csG = RT_GetGValue(closeSph->clr) * factor;
	int csB = RT_GetBValue(closeSph->clr) * factor;

	return RT_RGB(csR, csG, csB);
}

// Clears the screen with background color.

void Clear() {
	for (int x = 0; x < RT_WINDOW_WIDTH; x++) {
		for (int y = 0; y < RT_WINDOW_HEIGHT; y++) {
			PutPixel(x, y, mainScn.bgClr);
		}
	}
}

// Renders the scene.

void Draw() {
	for (int x = -RT_WINDOW_WIDTH / 2; x < RT_WINDOW_WIDTH / 2; x++) {
		for (int y = -RT_WINDOW_HEIGHT / 2; y < RT_WINDOW_HEIGHT / 2; y++) {
			Vector2 canvP = { x, y };
			Vector3 direct = CanvasToViewport(canvP);

			COLORREF clr = TraceRay(mainScn.cmrPos, direct, 1, FLT_MAX);
			PutPixel(x, y, clr);
		}
	}
}


