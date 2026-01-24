// (c) Beem Media. All rights reserved.

#pragma once

#include "MD3File.h"

#include <d3d9.h>

class CMD3SkinFile;

static constexpr DWORD D3DMD3VERTEX_TYPE = (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1);

struct d3d_md3_vertex
{
	D3DVECTOR Postion; /* Position of vertex. */
	D3DVECTOR Normal; /* The normal of the vertex. */
	FLOAT tu, tv; /* Texture coordinates. */
};

enum class d3d_md3_detail
{
	High,
	Medium,
	Low,
};

class CD3D_MD3Mesh
{
protected:
	CMD3File m_md3File;

	LPDIRECT3DVERTEXBUFFER9* m_lppVB;
	d3d_md3_vertex* m_lpVertices;
	LPDIRECT3DINDEXBUFFER9* m_lppIB;

	LPDIRECT3DDEVICE9 m_lpDevice;

	md3Vector** m_lppNormals;

	BOOL m_bMD3Loaded; /* Whether or not MD3 is loaded. */
	BOOL m_bValid; /* Whether or not the MD3 is valid for D3D usage. */
	DWORD m_dwNumSkins; /* Total number of skins in the mesh. */
	D3DPOOL m_Pool; /* The Pool Direct3D objects should be created in. */

	HRESULT CreateVB();
	HRESULT DeleteVB();

	HRESULT CreateIB();
	HRESULT DeleteIB();

	HRESULT CreateNormals();

	HRESULT CreateModel();
	HRESULT DeleteModel();

public:
	CD3D_MD3Mesh();
	~CD3D_MD3Mesh();

	HRESULT LoadMD3(
		const std::filesystem::path& Filename,
		LPDWORD lpBytesRead,
		LPDIRECT3DDEVICE9 lpDevice,
		D3DPOOL Pool);

	HRESULT ClearMD3();

	HRESULT Render(
		CMD3SkinFile* lpSkin,
		FLOAT fTime,
		LONG lFirstFrame,
		LONG lNextFrame,
		DWORD dwFlags);

	HRESULT RenderWithTexture(
		LPDIRECT3DTEXTURE9 lpTexture,
		LONG lMesh,
		FLOAT fTime,
		LONG lFirstFrame,
		LONG lNextFrame,
		DWORD dwFlags);

	HRESULT Invalidate();
	HRESULT Validate();

	HRESULT SetSkinRefs(
		CMD3SkinFile* lpSkin);

	HRESULT GetTagTranslation(
		DWORD dwTagRef,
		FLOAT fTime,
		LONG dwFirstFrame,
		LONG dwSecondFrame,
		D3DMATRIX* Translation);

	HRESULT GetNumTags(
		LONG* lpNumTags);
	HRESULT GetTagName(
		LONG lRef,
		char szTagName[MAX_QPATH]);
	HRESULT GetShader(
		LONG lMesh,
		LONG lShader,
		char szShaderName[MAX_QPATH],
		LONG* lpShaderNum);

	HRESULT GetNumMeshes(
		LONG* lpNumMeshes);

	HRESULT DumpDebug();

};
