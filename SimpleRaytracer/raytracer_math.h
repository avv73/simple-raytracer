#pragma once

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

float DotProduct(Vector3 v1, Vector3 v2);

Vector3 SubtractVector(Vector3 v1, Vector3 v2);
