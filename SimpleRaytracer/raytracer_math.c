#include "raytracer_math.h"
#include <math.h>

float DotProduct(Vector3 v1, Vector3 v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 AddVector(Vector3 v1, Vector3 v2) {
	Vector3 result = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	return result;
}

Vector3 SubtractVector(Vector3 v1, Vector3 v2) {
	Vector3 result = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	return result;
}

float LengthVector(Vector3 v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float ClampRGB(float x) {
	if (0 <= x && x <= 255) {
		return x;
	}
	else if (x < 0) {
		return 0;
	}

	return 255;
}

Vector3 ScaleVector(Vector3 v, float k) {
	Vector3 result = { v.x * k, v.y * k, v.z * k };
	return result;
}

Vector3 ReflectVector(Vector3 v, Vector3 n) {
	return SubtractVector(ScaleVector(n, 2 * DotProduct(n, v)), v);
}