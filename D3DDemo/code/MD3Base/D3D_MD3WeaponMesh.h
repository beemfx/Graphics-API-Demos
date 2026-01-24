// (c) Beem Media. All rights reserved.

#pragma once

#include "D3D_MD3Mesh.h"
#include "GFX3D9/GFX3D9TextureDB.h"

class CMD3WeaponMesh
{
protected:
	CD3D_MD3Mesh m_meshWeapon;
	CD3D_MD3Mesh m_meshBarrel;
	CD3D_MD3Mesh m_meshFlash;
	CD3D_MD3Mesh m_meshHand;

	CGFX3D9TextureDB m_TexDB;

	LPDIRECT3DTEXTURE9* m_lpFlashTex;
	LPDIRECT3DTEXTURE9* m_lpWeaponTex;
	LPDIRECT3DTEXTURE9* m_lpBarrelTex;

	LPDIRECT3DDEVICE9 m_lpDevice;

	BOOL m_bBarrel;

	WORD m_nTagWeapon;
	WORD m_nTagBarrel;
	WORD m_nTagFlash;

	BOOL m_bLoaded;

	HRESULT GetLink(CD3D_MD3Mesh* lpFirst, const char szTagName[], WORD* lpTagRef);
	HRESULT TextureExtension(char szShader[MAX_PATH]);

public:
	CMD3WeaponMesh();
	~CMD3WeaponMesh();

	HRESULT Clear();

	HRESULT Render(BOOL bFlash, const D3DMATRIX& WorldMatrix);

	HRESULT Load(LPDIRECT3DDEVICE9 lpDevice, char szDir[], d3d_md3_detail nDetail);
	HRESULT Invalidate();
	HRESULT Validate();
};
