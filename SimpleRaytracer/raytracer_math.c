#include "raytracer_math.h"
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265

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

float** RotateX(float teta) {
	teta = teta * PI / 180;

	float sinT = sin(teta);
	float cosT = cos(teta);

	float* firstR = (float*)malloc(sizeof(float) * 3);
	float* secR = (float*)malloc(sizeof(float) * 3);
	float* thR = (float*)malloc(sizeof(float) * 3);

	firstR[0] = 1;
	firstR[1] = 0;
	firstR[2] = 0;

	secR[0] = 0;
	secR[1] = cosT;
	secR[2] = -sinT;

	thR[0] = 0;
	thR[1] = sinT;
	thR[2] = cosT;

	float** result = (float**)malloc(sizeof(float*) * 3);
	result[0] = firstR;
	result[1] = secR;
	result[2] = thR;

	return result;
}

float** RotateY(float teta) {
	teta = teta * PI / 180;

	float sinT = sin(teta);
	float cosT = cos(teta);

	float* firstR = (float*)malloc(sizeof(float) * 3);
	float* secR = (float*)malloc(sizeof(float) * 3);
	float* thR = (float*)malloc(sizeof(float) * 3);

	firstR[0] = cosT;
	firstR[1] = 0;
	firstR[2] = sinT;

	secR[0] = 0;
	secR[1] = 1;
	secR[2] = 0;

	thR[0] = -sinT;
	thR[1] = 0;
	thR[2] = cosT;

	float** result = (float**)malloc(sizeof(float*) * 3);
	result[0] = firstR;
	result[1] = secR;
	result[2] = thR;

	return result;
}

float** RotateZ(float teta) {
	teta = teta * PI / 180;

	float sinT = sin(teta);
	float cosT = cos(teta);

	float* firstR = (float*)malloc(sizeof(float) * 3);
	float* secR = (float*)malloc(sizeof(float) * 3);
	float* thR = (float*)malloc(sizeof(float) * 3);

	firstR[0] = cosT;
	firstR[1] = -sinT;
	firstR[2] = 0;

	secR[0] = sinT;
	secR[1] = cosT;
	secR[2] = 0;

	thR[0] = 0;
	thR[1] = 0;
	thR[2] = 1;

	float** result = (float**)malloc(sizeof(float*) * 3);
	result[0] = firstR;
	result[1] = secR;
	result[2] = thR;

	return result;
}

Vector3 MultiplyVectorMatrix(Vector3 v, float** m) {
	float x;
	float y;
	float z;

	x = v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2];
	y = v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2];
	z = v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2];

	Vector3 res = { x, y, z };

	return res;
}

float** ComposeRotation(float** m1, float** m2) {
	float* firstR = (float*)malloc(sizeof(float) * 3);
	float* secR = (float*)malloc(sizeof(float) * 3);
	float* thR = (float*)malloc(sizeof(float) * 3);

	for (int i = 0; i < 3; i++) {
		firstR[i] = 0;
		secR[i] = 0;
		thR[i] = 0;
	}

	float** result = (float**)malloc(sizeof(float*) * 3);
	result[0] = firstR;
	result[1] = secR;
	result[2] = thR;

	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			for (int k = 0; k < 3; k++) {
				result[x][y] += m1[x][k] * m2[k][y];
			}
		}
	}

	return result;
}
