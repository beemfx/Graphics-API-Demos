// (c) Beem Media. All rights reserved.

#pragma once

#include "MD3File.h"

#include <d3d9.h>

class CMD3SkinFile;

static constexpr DWORD D3DMD3VERTEX_TYPE = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

struct d3d_md3_vertex
{
	D3DVECTOR Postion = { }; /* Position of vertex. */
	D3DVECTOR Normal = { }; /* The normal of the vertex. */
	FLOAT tu = { }, tv = { }; /* Texture coordinates. */
};

enum class d3d_md3_detail
{
	High,
	Medium,
	Low,
};

class CD3D_MD3Mesh
{
public:
	static constexpr md3_uint32 MD3TEXRENDER_NOCULL = (1 << 0);

private:
	class CBuffers
	{
	public:
		IDirect3DVertexBuffer9* VB = nullptr;
		IDirect3DIndexBuffer9* IB = nullptr;

		CBuffers() = default;
		CBuffers(const CBuffers& Other);
		CBuffers(CBuffers&& Other);
		~CBuffers();

		CBuffers& operator = (const CBuffers& Other);
		CBuffers& operator = (CBuffers&& Other);
	};

	class CMeshData
	{
	public:
		CBuffers Buffers;
		std::vector<md3Vector> Normals;
		mutable std::vector<d3d_md3_vertex> TempVerts;
	};

protected:
	CMD3File m_md3File;
	std::vector<CMeshData> m_MeshDatas;
	IDirect3DDevice9* m_Dev = nullptr;

	bool m_bMD3Loaded = false; /* Whether or not MD3 is loaded. */
	bool m_bValid = false; /* Whether or not the MD3 is valid for D3D usage. */
	md3_uint32 m_dwNumSkins = 0; /* Total number of skins in the mesh. */
	D3DPOOL m_Pool = D3DPOOL_DEFAULT; /* The Pool Direct3D objects should be created in. */

	bool CreateVB();
	bool CreateIB();
	bool CreateNormals();

	md3_bool CreateModel();
	void DeleteModel();

public:
	CD3D_MD3Mesh();
	~CD3D_MD3Mesh();

	md3_bool LoadMD3(const std::filesystem::path& Filename, IDirect3DDevice9* lpDevice, D3DPOOL Pool);

	void ClearMD3();

	void Render(
		CMD3SkinFile* lpSkin,
		FLOAT fTime,
		LONG lFirstFrame,
		LONG lNextFrame,
		DWORD dwFlags);

	void RenderWithTexture(
		LPDIRECT3DTEXTURE9 lpTexture,
		LONG lMesh,
		FLOAT fTime,
		LONG lFirstFrame,
		LONG lNextFrame,
		DWORD dwFlags);

	void Invalidate();
	bool Validate();

	void SetSkinRefs(CMD3SkinFile& Skin);

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
