#include "stdafx.h"

namespace Ripples
{
	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;
	ID3DXSprite* sprite = NULL;
	IDirect3DTexture9* texture = NULL;

	WAVE_PIXEL wave[SCREEN_HEIGHT][SCREEN_WIDTH];
	std::mutex wave_mtx;

	bool running = true;
	HANDLE update = NULL;
	HANDLE render = NULL;

	bool Initialize(HWND hwnd)
	{
		d3d = Direct3DCreate9(D3D_SDK_VERSION);

		if (!d3d)
			return false;

		D3DPRESENT_PARAMETERS d3dpp = { 0 };

		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = hwnd;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;

		if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev)))
			return false;

		if (FAILED(D3DXCreateSprite(d3ddev, &sprite)))
			return false;

		// default alpha blend states 
		d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// set linear interpolation
		d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		// create texture
		if (FAILED(d3ddev->CreateTexture(SCREEN_WIDTH, SCREEN_HEIGHT, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL)))
			return false;

		update = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Update, NULL, 0, NULL);
		render = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Render, NULL, 0, NULL);

		return true;
	}

	void Uninitialize()
	{
		running = false;

		if (update)
			WaitForSingleObject(update, INFINITE);
		
		if (render)
			WaitForSingleObject(render, INFINITE);

		if (texture)
			texture->Release();

		sprite->Release();
		d3ddev->Release();
		d3d->Release();
	}

	void Ripple(int x, int y, float delta)
	{
		if (x > 0 && x < SCREEN_WIDTH - 1 && y > 0 && y < SCREEN_HEIGHT - 1)
		{
			wave_mtx.lock();
			wave[y][x].u += delta;
			wave_mtx.unlock();
		}
	}

	void Update()
	{
		while (running)
		{
			wave_mtx.lock();

			const int n = 4;
			const float timestep = 0.5f;

			for (int i = 0; i < n; i++)
			{
				for (int y = 1; y < SCREEN_HEIGHT - 1; y++)
					for (int x = 1; x < SCREEN_WIDTH - 1; x++)
						wave[y][x].dv = (wave[y - 1][x].u + wave[y][x - 1].u + wave[y][x + 1].u + wave[y + 1][x].u - 4 * wave[y][x].u) / 4.0f - DAMPING_FACTOR * wave[y][x].du;

				for (int y = 1; y < SCREEN_HEIGHT - 1; y++)
				{
					for (int x = 1; x < SCREEN_WIDTH - 1; x++)
					{
						wave[y][x].du += timestep * wave[y][x].dv;
						wave[y][x].u += timestep * wave[y][x].du;
					}
				}
			}
		
			wave_mtx.unlock();
			Sleep(1);
		}
	}

	void Render()
	{
		while (running)
		{
			d3ddev->BeginScene();
			sprite->Begin(D3DXSPRITE_ALPHABLEND);

			D3DLOCKED_RECT rect;
			texture->LockRect(0, &rect, NULL, 0);

			DWORD* pixel = (DWORD*)rect.pBits;

			wave_mtx.lock();

			for (int y = 0; y < SCREEN_HEIGHT; y++)
			{
				for (int x = 0; x < SCREEN_WIDTH; x++)
				{
					float u = wave[y][x].u;
					float r = 0.0f + u;
					float g = 0.0f + u;
					float b = 0.0f + u;

					if (r < 0.0f)
						r = 0.0f;
					else if (r > 255.0f)
						r = 255.0f;
					if (g < 0.0f)
						g = 0.0f;
					else if (g > 255.0f)
						g = 255.0f;
					if (b < 0.0f)
						b = 0.0f;
					else if (b > 255.0f)
						b = 255.0f;

					*pixel = 0xFF000000 | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;

					pixel++;
				}
			}

			wave_mtx.unlock();

			texture->UnlockRect(0);

			sprite->Draw(texture, NULL, NULL, NULL, 0xFFFFFFFF);

			sprite->End();
			d3ddev->EndScene();
			d3ddev->Present(NULL, NULL, NULL, NULL);
			
			Sleep(15);
		}
	}
}