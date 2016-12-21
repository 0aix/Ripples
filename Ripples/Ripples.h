#pragma once

#include "stdafx.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define DAMPING_FACTOR 0.01f

struct WAVE_PIXEL
{
	float u = 0.0f;
	float du = 0.0f;
	float dv = 0.0f;
};

namespace Ripples
{
	extern IDirect3DDevice9* d3ddev;

	bool Initialize(HWND hwnd);
	void Uninitialize();

	void Ripple(int x, int y, float delta);

	void Update();
	void Render();
}