#pragma once
#include <stdarg.h>

typedef enum {
	AMBIENT,
	POINTED,
	DIRECTIONAL
} LightType;

typedef struct {
	float x;
	float y;
	float z;
} Vector3;

typedef struct {
	float x;
	float y;
} Vector2;

typedef struct {
	float t1;
	float t2;
	char isValid;
} TTupel;

float ClampRGB(float x);

float DotProduct(Vector3 v1, Vector3 v2);

Vector3 AddVector(Vector3 v1, Vector3 v2);

Vector3 SubtractVector(Vector3 v1, Vector3 v2);

Vector3 ScaleVector(Vector3 v, float k);

Vector3 ReflectVector(Vector3 v, Vector3 n);

float LengthVector(Vector3 v);

/*
	Rotate functions return a rotation matrix around the specified axis.
	Teta is a degree angle.
	ComposeRotation returns the composition of matrix m1 with m2 -> m1 X m2;
*/


float** RotateX(float teta);

float** RotateY(float teta);

float** RotateZ(float teta);

float** ComposeRotation(float** m1, float** m2);

Vector3 MultiplyVectorMatrix(Vector3 v, float** m);