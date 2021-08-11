#pragma once

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

