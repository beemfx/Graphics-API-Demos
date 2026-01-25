// (c) Beem Media. All rights reserved.

#include "D3D_MD3ObjectMesh.h"
#include <d3dx9.h>
#include <stdio.h>
#include "defines.h"
#include "Library/Functions.h"

CD3D_MD3ObjectMesh::CD3D_MD3ObjectMesh()
{
	m_lppObjTex = NULL;

	m_bLoaded = FALSE;
}

CD3D_MD3ObjectMesh::~CD3D_MD3ObjectMesh()
{
	Clear();
}

HRESULT CD3D_MD3ObjectMesh::Clear()
{
	LONG lNumMesh = 0;
	DWORD i = 0;

	if (!m_bLoaded)
		return S_FALSE;

	m_meshObject.GetNumMeshes(&lNumMesh);

	for (i = 0; i < (DWORD)lNumMesh; i++) {
		SAFE_RELEASE(m_lppObjTex[i]);
	}
	SAFE_FREE(m_lppObjTex);

	m_TexDB.ClearDB();
	m_meshObject.ClearMD3();

	return S_OK;
}

HRESULT CD3D_MD3ObjectMesh::Load(LPDIRECT3DDEVICE9 lpDevice, const std::filesystem::path& Filename, d3d_md3_detail nDetail)
{
	assert(false); // CD3D_MD3ObjectMesh is untested, make sure it works before removing this assert.

	const std::string Ext = [&]() -> std::string
		{
			if (nDetail == d3d_md3_detail::Low)
				return "_2.md3";
			else if (nDetail == d3d_md3_detail::Medium)
				return "_1.md3";
			else
				return ".md3";
		}();

	const std::filesystem::path AdjFilename = Filename.parent_path() / (Filename.stem().string() + Ext);

	if (!m_meshObject.LoadMD3(AdjFilename, lpDevice, D3DPOOL_DEFAULT))
	{
		if (nDetail == d3d_md3_detail::Medium || nDetail == d3d_md3_detail::Low)
		{
			return Load(lpDevice, Filename, d3d_md3_detail::High);
		}

		return E_FAIL;
	}

	LONG lNumMesh = 0;
	m_meshObject.GetNumMeshes(&lNumMesh);

	m_lppObjTex = (LPDIRECT3DTEXTURE9*)malloc(sizeof(LPDIRECT3DTEXTURE9) * lNumMesh);
	if (m_lppObjTex == NULL) {
		m_meshObject.ClearMD3();
		return E_FAIL;
	}
	//Get the textures.
	for (LONG i = 0; i < lNumMesh; i++)
	{
		md3_char8 szShader[MAX_QPATH];
		m_meshObject.GetShader(i + 1, 1, szShader, NULL);
		std::filesystem::path ShaderPath = Functions::RemoveDirectoryFromString(szShader);
		std::filesystem::path ShaderFile = Filename.root_directory() / ShaderPath;
		if (m_TexDB.AddTexture(lpDevice, ShaderFile))
		{
			m_lppObjTex[i] = m_TexDB.GetTexture(ShaderFile);
		}
		else
		{
			m_lppObjTex[i] = NULL;
		}
	}
	m_bLoaded = TRUE;
	return S_OK;
}

void CD3D_MD3ObjectMesh::Invalidate()
{
	if (!m_bLoaded)
		return;

	m_meshObject.Invalidate();
}

bool CD3D_MD3ObjectMesh::Validate(LPDIRECT3DDEVICE9 lpDevice)
{
	if (!m_bLoaded)
		return false;

	return m_meshObject.Validate();
}

HRESULT CD3D_MD3ObjectMesh::Render(LPDIRECT3DDEVICE9 lpDevice, const D3DMATRIX& SavedWorldMatrix)
{
	LONG lNumMesh = 0;
	DWORD i = 0;

	D3DXMATRIX WorldMatrix, Orientation, Translation;

	if (!m_bLoaded)
		return S_FALSE;
	D3DXMatrixIdentity(&WorldMatrix);
	D3DXMatrixIdentity(&Orientation);


	D3DXMatrixRotationX(&Translation, 1.5f * D3DX_PI);
	Orientation *= Translation;
	D3DXMatrixRotationY(&Translation, 0.5f * D3DX_PI);
	Orientation *= Translation;

	Orientation *= SavedWorldMatrix;
	WorldMatrix *= Orientation;

	lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);

	m_meshObject.GetNumMeshes(&lNumMesh);

	for (i = 0; i < (DWORD)lNumMesh; i++) {
		m_meshObject.RenderWithTexture(
			m_lppObjTex[i],
			i + 1,
			0.0f,
			0,
			0,
			0);
	}

	lpDevice->SetTransform(D3DTS_WORLD, &SavedWorldMatrix);
	return S_OK;
}