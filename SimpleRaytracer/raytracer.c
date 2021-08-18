#define UNICODE
#define _UNICODE

#include "raytracer.h"
#include <windows.h>
#include <math.h>
#include <time.h>
#include <strsafe.h>

#define FLT_MAX          3.402823466e+38F        // max value of float
#define EPSILON			 0.1					 // very small delta from 0, good precision is needed here; if epsilion is too small it will cause bright spots in the big sphere

typedef struct {
	float t;
	Sphere* sph;
	Triangle* tr;
} STTupel;

typedef struct {
	float t1;
	float t2;
	char isValid;
} TTupel;

int RT_WINDOW_WIDTH;
int RT_WINDOW_HEIGHT;

// Show elapsed time when loading finishes
const int SHOW_ELAPSED = 1;
clock_t startTime;

// Reflection depth of the raytracer, 0 for disabled
const int RT_DEPTH = 3;

// Subsampling factor, 0 for disabled
const int SUBSAMPLE_FACTOR = 0;

COLORREF* frmBuffer = NULL;

void Draw();

void Update(HWND wndHandle);

STTupel ClosestIntersection(Vector3 orig, Vector3 direct, float minT, float maxT);


// Initializes the raytracer and renders the scene provided.

void StartRaytracer(HWND wndHandle, int wWidth, int wHeight) {
	RT_WINDOW_WIDTH = wWidth;
	RT_WINDOW_HEIGHT = wHeight;

	startTime = clock();

	if (!frmBuffer) {
		frmBuffer = (COLORREF*)calloc(RT_WINDOW_HEIGHT * RT_WINDOW_WIDTH, sizeof(COLORREF));
		Draw();

		if (SHOW_ELAPSED) {
			clock_t elapsed = clock() - startTime;

			TCHAR txt[50];

			TCHAR header[] = TEXT("Information");
			LPCTSTR textPlc = TEXT("Rendering time: %d ms");

			StringCbPrintf(txt, 50 * sizeof(TCHAR), textPlc, elapsed);

			MessageBox(NULL, txt, header, NULL);
		}
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
	DeleteObject(map);
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

// Computes lighting of a single point using the normal vector of the object in scene, the point itself, vector from point to camera & specular reflection of surface
// Returns float in range [0,1] representing the intensity of the light.

float ComputeLighting(Vector3 p, Vector3 n, Vector3 v, float s) {
	float res = 0.0f;
	float tMax = 1; // maximum allowed parameter for the shadow computation, inf for directional lights, 1 for point lights

	Vector3 directL;

	for (int i = 0; i < mainScn.lightCount; i++) {
		if (mainScn.lights[i].type == AMBIENT) {
			res += mainScn.lights[i].ins;
			continue;
		}

		if (mainScn.lights[i].type == POINTED) {
			directL = SubtractVector(mainScn.lights[i].pos, p);
		}
		else {
			directL = mainScn.lights[i].pos;
			tMax = FLT_MAX;
		}

		// shadowing

		STTupel shadow = ClosestIntersection(p, directL, EPSILON, tMax);
		if (shadow.sph) {
			continue;
		}

		// diffuse 

		float n_dot_l = DotProduct(n, directL);
		if (n_dot_l > 0) {
			res += mainScn.lights[i].ins * n_dot_l / (LengthVector(n) * LengthVector(directL));
		}

		// specular

		if (s != -1) {
			Vector3 refl = ReflectVector(directL, n);
			float refl_dot_v = DotProduct(refl, v);
			if (refl_dot_v > 0) {
				res += mainScn.lights[i].ins * pow(refl_dot_v / (LengthVector(refl) * LengthVector(v)), s);
			}
		}
	}

	if (res > 1) {
		res = 1;
	}

	if (res < 0) {
		res = 0;
	}

	return res;
}

// Intersects a ray (origin and direction) with a triangle. Returns a tuple with the quadratic solution parameter of the intersection.

TTupel IntersectRayTriangle(Vector3 orig, Vector3 direct, Triangle tr) {
	TTupel result;
	result.isValid = 1;

	Vector3 ab = SubtractVector(tr.b, tr.a);
	Vector3 ac = SubtractVector(tr.c, tr.a);

	Vector3 n = CrossProduct(ab, ac);
	n = ScaleVector(n, (1 / LengthVector(n)));

	float area = LengthVector(n);

	float n_dot_direct = DotProduct(direct, n);
	if (fabs(n_dot_direct) < EPSILON) { // 0.01?
		result.isValid = 0;
		return;
	}

	float d = DotProduct(n, tr.a); // arbitrary point can be chosen

	float t = (DotProduct(n, orig) + d) / n_dot_direct; 

	if (t < 0) {
		result.isValid = 0;
		return;
	}

	Vector3 interP = AddVector(orig, ScaleVector(direct, t));

	// inside-outside test

	Vector3 c;

	Vector3 e0 = SubtractVector(tr.b, tr.a);
	Vector3 p0 = SubtractVector(interP, tr.a);
	c = CrossProduct(e0, p0);

	if (DotProduct(n, c) < 0) {
		result.isValid = 0;
		return;
	}

	Vector3 e1 = SubtractVector(tr.c, tr.b);
	Vector3 p1 = SubtractVector(interP, tr.b);
	c = CrossProduct(e1, p1);

	if (DotProduct(n, c) < 0) {
		result.isValid = 0;
		return;
	}

	Vector3 e2 = SubtractVector(tr.a, tr.c);
	Vector3 p2 = SubtractVector(interP, tr.c);
	c = CrossProduct(e2, p2);

	if (DotProduct(n, c) < 0) {
		result.isValid = 0;
		return;
	}

	result.t1 = t;
	return result;
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

// Finds the closest sphere or triangle in respect to an origin point and direction vector; returns a tuple with the closest sphere (triangle) and the computed parameter for the ray equation.

STTupel ClosestIntersection(Vector3 orig, Vector3 direct, float minT, float maxT) {
	float closeT = FLT_MAX;
	Sphere* closeSph = NULL;
	Triangle* closeTr = NULL;

	for (int i = 0; i < mainScn.sphCount; i++) {
		TTupel ts = IntersectRaySphere(orig, direct, mainScn.sphs[i]);

		if (!ts.isValid) {
			continue;
		}

		if (ts.t1 < closeT && minT < ts.t1 && ts.t1 < maxT) {
			closeT = ts.t1;
			closeSph = &mainScn.sphs[i];
		}

		if (ts.t2 < closeT && minT < ts.t2 && ts.t2 < maxT) {
			closeT = ts.t2;
			closeSph = &mainScn.sphs[i];
		}
	}

	for (int i = 0; i < mainScn.trCount; i++) {
		TTupel ts = IntersectRayTriangle(orig, direct, mainScn.trs[i]);

		if (!ts.isValid) {
			continue;
		}

		if (ts.t1 < closeT && minT < ts.t1 && ts.t1 < maxT) {
			closeT = ts.t1;
			closeSph = NULL;
			closeTr = &mainScn.trs[i];
		}
	}

	STTupel result = { closeT, closeSph, closeTr };
	return result;
}

// Traces a ray (origin and direction) with all spheres & triangles in the scene. Returns the color of the struck object (if any).

COLORREF TraceRay(Vector3 orig, Vector3 direct, float minT, float maxT, int depth) {
	STTupel intersect = ClosestIntersection(orig, direct, minT, maxT);

	if (!intersect.sph && !intersect.tr) {
		return mainScn.bgClr;
	}

	Vector3 interP = AddVector(orig, ScaleVector(direct, intersect.t)); // compute the intersection point by substituting the found parameter in the point equation
	Vector3 normal; 
	float objSpecFactor;
	COLORREF objClr;
	float objRefl;

	if (intersect.sph) {
		normal = SubtractVector(interP, intersect.sph->cnt);          // compute sphere normal
	}
	else {
		Vector3 ab = SubtractVector(intersect.tr->b, intersect.tr->a);
		Vector3 ac = SubtractVector(intersect.tr->c, intersect.tr->a);

		normal = CrossProduct(ab, ac);
	}

	normal = ScaleVector(normal, (1 / LengthVector(normal)));

	// channel-wise multiply light intensity with object's color & clamp in rgb range [0-255]
	Vector3 directIn = ScaleVector(direct, -1);

	objSpecFactor = intersect.sph ? intersect.sph->specFactor : intersect.tr->specFactor;
	objClr = intersect.sph ? intersect.sph->clr : intersect.tr->clr;
	objRefl = intersect.sph ? intersect.sph->refl : intersect.tr->refl;

	float factor = ComputeLighting(interP, normal, directIn, objSpecFactor);

	int csR = RT_GetRValue(objClr) * factor;
	int csG = RT_GetGValue(objClr) * factor;
	int csB = RT_GetBValue(objClr) * factor;

	if (depth <= 0 || objRefl <= 0) {
		return RT_RGB(csR, csG, csB);
	}

	// compute reflected color
	Vector3 refl = ReflectVector(directIn, normal);

	COLORREF reflClr = TraceRay(interP, refl, EPSILON, FLT_MAX, depth - 1);

	int rfR = RT_GetRValue(reflClr) * objRefl;
	int rfG = RT_GetGValue(reflClr) * objRefl;
	int rfB = RT_GetBValue(reflClr) * objRefl;

	csR *= (1 - objRefl);
	csG *= (1 - objRefl);
	csB *= (1 - objRefl);

	COLORREF finClr = RT_RGB(csR + rfR, csG + rfG, csB + rfB);

	return finClr;
}

// Clears the screen with background color.

void Clear() {
	for (int x = -RT_WINDOW_WIDTH / 2; x < RT_WINDOW_WIDTH / 2; x++) {
		for (int y = -RT_WINDOW_HEIGHT / 2; y < RT_WINDOW_HEIGHT / 2; y++) {
			PutPixel(x, y, mainScn.bgClr);
		}
	}
}

// Renders the scene.

void Draw() {
	COLORREF clr;
	COLORREF prevClr;
	int count = 0;

	for (int x = -RT_WINDOW_WIDTH / 2; x < RT_WINDOW_WIDTH / 2; x++) {
		for (int y = -RT_WINDOW_HEIGHT / 2; y < RT_WINDOW_HEIGHT / 2; y++) {
			Vector2 canvP = { x, y };
			Vector3 direct = CanvasToViewport(canvP);

			direct = MultiplyVectorMatrix(direct, mainScn.rotMatrix);

			if (SUBSAMPLE_FACTOR && count % SUBSAMPLE_FACTOR != 0) {
				clr = prevClr;
				count++;
			}
			else {
				clr = TraceRay(mainScn.cmrPos, direct, 1, FLT_MAX, RT_DEPTH);
				prevClr = clr;
				count++;
			}
			
			int clrR = ClampRGB(RT_GetRValue(clr));
			int clrG = ClampRGB(RT_GetGValue(clr));
			int clrB = ClampRGB(RT_GetBValue(clr));

			PutPixel(x, y, RT_RGB(clrR, clrG, clrB));
		}
	}
}


