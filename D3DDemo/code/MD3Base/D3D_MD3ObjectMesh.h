// (c) Beem Media. All rights reserved.

#pragma once

#include "D3D_MD3Mesh.h"
#include "GFX3D9/GFX3D9TextureDB.h"

class CD3D_MD3ObjectMesh
{
protected:
	CD3D_MD3Mesh m_meshObject;
	CGFX3D9TextureDB m_TexDB;
	LPDIRECT3DTEXTURE9* m_lppObjTex;

	BOOL m_bLoaded;

	HRESULT TextureExtension(LPDIRECT3DDEVICE9 lpDevice, char szShader[MAX_PATH]);
public:
	CD3D_MD3ObjectMesh();
	~CD3D_MD3ObjectMesh();

	HRESULT Render(LPDIRECT3DDEVICE9 lpDevice, const D3DMATRIX& WorldMatrix);

	HRESULT Load(LPDIRECT3DDEVICE9 lpDevice, char szFile[], d3d_md3_detail nDetail);

	HRESULT Clear();

	void Invalidate();
	bool Validate(LPDIRECT3DDEVICE9 lpDevice);
};
