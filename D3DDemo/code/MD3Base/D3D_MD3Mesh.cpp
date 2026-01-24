// (c) Beem Media. All rights reserved.

#include "D3D_MD3Mesh.h"
#include "D3D_MD3Skin.h"
#include "FileSystem/DataStream.h"
#include "MD3Data.h"
#include <d3dx9.h>

CD3D_MD3Mesh::CD3D_MD3Mesh()
{

}

CD3D_MD3Mesh::~CD3D_MD3Mesh()
{
	ClearMD3();
}

HRESULT CD3D_MD3Mesh::GetNumTags(
	LONG* lpNumTags)
{
	if (m_bMD3Loaded) {
		*lpNumTags = m_md3File.Header.NumTags;
		return S_OK;
	}
	else {
		*lpNumTags = 0;
		return E_FAIL;
	}
}
HRESULT CD3D_MD3Mesh::GetTagName(
	LONG lRef,
	char szTagName[MAX_QPATH])
{
	if (m_bMD3Loaded && (lRef > 0) && (lRef <= m_md3File.Header.NumTags)) {
		strcpy(szTagName, m_md3File.Tags[lRef - 1].Name);
		return S_OK;
	}
	else {
		strcpy(szTagName, "");
		return E_FAIL;
	}
}

HRESULT CD3D_MD3Mesh::GetTagTranslation(
	DWORD dwTagRef,
	FLOAT fTime,
	LONG dwFirstFrame,
	LONG dwSecondFrame,
	D3DMATRIX* Translation)
{
	D3DXMATRIX FirstFrame, SecondFrame, Final;
	D3DXQUATERNION FirstQuat, SecondQuat, FinalQuat;
	LONG lTagRefFirst = 0, lTagRefSecond = 0;
	FLOAT x = 0.0f, y = 0.0f, z = 0.0f;

	lTagRefFirst = dwFirstFrame * m_md3File.Header.NumTags + dwTagRef - 1;
	lTagRefSecond = dwSecondFrame * m_md3File.Header.NumTags + dwTagRef - 1;

	D3DXMatrixIdentity((D3DXMATRIX*)Translation);

	if ((dwTagRef < 1) || (((LONG)dwTagRef) > m_md3File.Header.NumTags))
		return E_FAIL;

	if ((dwFirstFrame < 0) ||
		(dwSecondFrame < 0) ||
		(dwFirstFrame >= m_md3File.Header.NumFrames) ||
		(dwSecondFrame >= m_md3File.Header.NumFrames))
		return E_FAIL;

	FirstFrame._11 = m_md3File.Tags[lTagRefFirst].Axis[0].x;
	FirstFrame._12 = m_md3File.Tags[lTagRefFirst].Axis[0].y;
	FirstFrame._13 = m_md3File.Tags[lTagRefFirst].Axis[0].z;
	FirstFrame._14 = 0;

	FirstFrame._21 = m_md3File.Tags[lTagRefFirst].Axis[1].x;
	FirstFrame._22 = m_md3File.Tags[lTagRefFirst].Axis[1].y;
	FirstFrame._23 = m_md3File.Tags[lTagRefFirst].Axis[1].z;
	FirstFrame._24 = 0;

	FirstFrame._31 = m_md3File.Tags[lTagRefFirst].Axis[2].x;
	FirstFrame._32 = m_md3File.Tags[lTagRefFirst].Axis[2].y;
	FirstFrame._33 = m_md3File.Tags[lTagRefFirst].Axis[2].z;
	FirstFrame._34 = 0;

	FirstFrame._41 = 0;
	FirstFrame._42 = 0;
	FirstFrame._43 = 0;
	FirstFrame._44 = 1;

	//If both frames are the same this is a much simpler proccess

	SecondFrame._11 = m_md3File.Tags[lTagRefSecond].Axis[0].x;
	SecondFrame._12 = m_md3File.Tags[lTagRefSecond].Axis[0].y;
	SecondFrame._13 = m_md3File.Tags[lTagRefSecond].Axis[0].z;
	SecondFrame._14 = 0;

	SecondFrame._21 = m_md3File.Tags[lTagRefSecond].Axis[1].x;
	SecondFrame._22 = m_md3File.Tags[lTagRefSecond].Axis[1].y;
	SecondFrame._23 = m_md3File.Tags[lTagRefSecond].Axis[1].z;
	SecondFrame._24 = 0;

	SecondFrame._31 = m_md3File.Tags[lTagRefSecond].Axis[2].x;
	SecondFrame._32 = m_md3File.Tags[lTagRefSecond].Axis[2].y;
	SecondFrame._33 = m_md3File.Tags[lTagRefSecond].Axis[2].z;
	SecondFrame._34 = 0;

	SecondFrame._41 = 0;
	SecondFrame._42 = 0;
	SecondFrame._43 = 0;
	SecondFrame._44 = 1;

	D3DXQuaternionRotationMatrix(&FirstQuat, &FirstFrame);
	D3DXQuaternionRotationMatrix(&SecondQuat, &SecondFrame);
	D3DXQuaternionSlerp(&FinalQuat, &FirstQuat, &SecondQuat, fTime);
	D3DXMatrixRotationQuaternion(&Final, &FinalQuat);

	//Interpolate translation vector and stick it in the final matrix.
	x = m_md3File.Tags[lTagRefFirst].Position.x +
		fTime *
		(m_md3File.Tags[lTagRefSecond].Position.x -
			m_md3File.Tags[lTagRefFirst].Position.x);

	y = m_md3File.Tags[lTagRefFirst].Position.y +
		fTime *
		(m_md3File.Tags[lTagRefSecond].Position.y -
			m_md3File.Tags[lTagRefFirst].Position.y);

	z = m_md3File.Tags[lTagRefFirst].Position.z +
		fTime *
		(m_md3File.Tags[lTagRefSecond].Position.z -
			m_md3File.Tags[lTagRefFirst].Position.z);


	Final._41 = x;
	Final._42 = y;
	Final._43 = z;
	Final._44 = 1;

	*Translation = Final;
	return S_OK;
}

md3_bool CD3D_MD3Mesh::LoadMD3(const std::filesystem::path& Filename, IDirect3DDevice9* lpDevice, D3DPOOL Pool)
{
	HRESULT hr = 0;

	// Clear any MD3 that might exist.
	ClearMD3();

	CDataStream MD3Stream(Filename);

	if (MD3Stream.GetSize() == 0)
	{
		return false;
	}

	//Read the MD3 File, insuring that it really is an MD3 file.
	if (!m_md3File.Load(MD3Stream))
	{
		return false;
	}

	m_Pool = Pool;

	m_Dev = lpDevice;
	if (!m_Dev)
	{
		return false;
	}

	m_Dev->AddRef();

	if (CreateModel())
	{
		m_bMD3Loaded = true;
	}
	else
	{
		m_Dev->Release();
		m_Dev = nullptr;
	}

	return m_bMD3Loaded;
}

void CD3D_MD3Mesh::ClearMD3()
{
	DeleteModel();
	m_md3File.Unload();

	if (m_Dev)
	{
		m_Dev->Release();
		m_Dev = nullptr;
	}

	m_bMD3Loaded = false;
}

void CD3D_MD3Mesh::RenderWithTexture(
	LPDIRECT3DTEXTURE9 lpTexture,
	LONG lMesh,
	FLOAT fTime,
	LONG lFirstFrame,
	LONG lNextFrame,
	DWORD dwFlags)
{
	LONG lNumVertices = 0;
	LONG i = 0, j = 0;
	FLOAT fFirstX = 0.0f, fFirstY = 0.0f, fFirstZ = 0.0f;
	FLOAT fNextX = 0.0f, fNextY = 0.0f, fNextZ = 0.0f;
	LPVOID lpBuffer = NULL;

	if (!m_Dev)
		return;

	if ((lMesh < 1) || (lMesh > m_md3File.Header.NumMeshes))
		return;

	//Make sure the frames are within the appropriate range.
	if ((lFirstFrame < 0) || (lFirstFrame >= m_md3File.Header.NumFrames))
		return;
	if ((lNextFrame < 0) || (lNextFrame >= m_md3File.Header.NumFrames))
		return;

	if (MD3TEXRENDER_NOCULL == (MD3TEXRENDER_NOCULL & dwFlags))
		m_Dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	else
		m_Dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	md3Mesh& Mesh = m_md3File.Meshes[lMesh - 1];
	CMeshData& MeshData = m_MeshDatas[lMesh - 1];

	lNumVertices = Mesh.MeshHeader.NumVertices;
	assert(MeshData.TempVerts.size() == lNumVertices);
	assert(MeshData.Normals.size() == lNumVertices*Mesh.MeshHeader.NumFrames);
	for (i = 0; i < lNumVertices; i++)
	{
		fFirstX = Mesh.Vertexes[i + lFirstFrame * lNumVertices].x * MD3_XYZ_SCALE;
		fFirstY = Mesh.Vertexes[i + lFirstFrame * lNumVertices].y * MD3_XYZ_SCALE;
		fFirstZ = Mesh.Vertexes[i + lFirstFrame * lNumVertices].z * MD3_XYZ_SCALE;

		fNextX = Mesh.Vertexes[i + lNextFrame * lNumVertices].x * MD3_XYZ_SCALE;
		fNextY = Mesh.Vertexes[i + lNextFrame * lNumVertices].y * MD3_XYZ_SCALE;
		fNextZ = Mesh.Vertexes[i + lNextFrame * lNumVertices].z * MD3_XYZ_SCALE;

		//Interpolate the first and second frames.
		MeshData.TempVerts[i].Postion.x = (fFirstX + fTime * (fNextX - fFirstX));
		MeshData.TempVerts[i].Postion.y = (fFirstY + fTime * (fNextY - fFirstY));
		MeshData.TempVerts[i].Postion.z = (fFirstZ + fTime * (fNextZ - fFirstZ));

		//Interpolate Normal vector.
		fFirstX = MeshData.Normals[i + lFirstFrame * lNumVertices].x;
		fFirstY = MeshData.Normals[i + lFirstFrame * lNumVertices].y;
		fFirstZ = MeshData.Normals[i + lFirstFrame * lNumVertices].z;

		fNextX = MeshData.Normals[i + lNextFrame * lNumVertices].x;
		fNextY = MeshData.Normals[i + lNextFrame * lNumVertices].y;
		fNextZ = MeshData.Normals[i + lNextFrame * lNumVertices].z;

		MeshData.TempVerts[i].Normal.x = (fFirstX + fTime * (fNextX - fFirstX));
		MeshData.TempVerts[i].Normal.y = (fFirstY + fTime * (fNextY - fFirstY));
		MeshData.TempVerts[i].Normal.z = (fFirstZ + fTime * (fNextZ - fFirstZ));

		//Get the texture coordinates.
		MeshData.TempVerts[i].tu = Mesh.TexCoords[i].tu;
		MeshData.TempVerts[i].tv = Mesh.TexCoords[i].tv;
	}

	IDirect3DVertexBuffer9_Lock(MeshData.Buffers.VB, 0, 0, &lpBuffer, 0);
	memcpy(lpBuffer, MeshData.TempVerts.data(), sizeof(d3d_md3_vertex) * lNumVertices);
	IDirect3DVertexBuffer9_Unlock(MeshData.Buffers.VB);

	IDirect3DDevice9_SetStreamSource(m_Dev, 0, MeshData.Buffers.VB, 0, sizeof(d3d_md3_vertex));
	IDirect3DDevice9_SetIndices(m_Dev, MeshData.Buffers.IB);

	m_Dev->SetTexture(0, lpTexture);
	IDirect3DDevice9_SetFVF(m_Dev, D3DMD3VERTEX_TYPE);

	IDirect3DDevice9_DrawIndexedPrimitive(
		m_Dev,
		D3DPT_TRIANGLELIST,
		0,
		0,
		Mesh.MeshHeader.NumVertices,
		0,
		Mesh.MeshHeader.NumTriangles);

	//#define DRAW_NORMAL_MESH
#ifdef DRAW_NORMAL_MESH
	FLOAT Line[6];
	m_Dev->SetTexture(0, NULL);
	for (int k = 0; k < Mesh.MeshHeader.NumVertices; k++)
	{
		Line[0] = m_Vertexes[k].Postion.x;
		Line[1] = m_Vertexes[k].Postion.y;
		Line[2] = m_Vertexes[k].Postion.z;
		Line[3] = m_Vertexes[k].Normal.x;
		Line[4] = m_Vertexes[k].Normal.y;
		Line[5] = m_Vertexes[k].Normal.z;
		m_Dev->SetFVF(D3DFVF_XYZ);
		m_Dev->DrawPrimitiveUP(
			D3DPT_LINELIST,
			1,
			&Line,
			sizeof(FLOAT) * 3);
		m_Dev->SetFVF(D3DMD3VERTEX_TYPE);
	}
#endif //DRAW_NORMAL_MESH
}


void CD3D_MD3Mesh::Render(
	CD3D_MD3Skin* lpSkin,
	FLOAT fTime,
	LONG lFirstFrame,
	LONG lNextFrame,
	DWORD dwFlags)
{
	LONG i = 0, j = 0;
	LONG lVertexOffset = 0;
	LONG lNumVertices = 0;

	FLOAT fFirstX = 0.0f;
	FLOAT fFirstY = 0.0f;
	FLOAT fFirstZ = 0.0f;

	FLOAT fNextX = 0.0f;
	FLOAT fNextY = 0.0f;
	FLOAT fNextZ = 0.0f;

	LPVOID lpBuffer = NULL;

	LPDIRECT3DTEXTURE9 lpTexture = NULL;

	//Bail if MD3 is not loaded is is currently not valid.
	if (!m_bMD3Loaded)
		return;
	if (!m_bValid)
		return;

	//Make sure the frames are within the appropriate range.
	if ((lFirstFrame < 0) || (lFirstFrame >= m_md3File.Header.NumFrames))
		return;
	if ((lNextFrame < 0) || (lNextFrame >= m_md3File.Header.NumFrames))
		return;

	//Insure that fTime is between 0.0f and 1.0f.
	if (fTime > 1.0f) {
		fTime -= (LONG)fTime;
	}


	//Loop for each mesh, interpolate the frames, and render the mesh.
	for (i = 0; i < m_md3File.Header.NumMeshes; i++) {
		lpTexture = lpSkin->GetTexturePointer(i);
		if (lpTexture)
		{
			RenderWithTexture(
				lpTexture,
				i + 1,
				fTime,
				lFirstFrame,
				lNextFrame,
				dwFlags);

			lpTexture->Release();
		}
	}

	//#define TESTBOX
#ifdef TESTBOX
	float box[2][3];
	box[0][0] = m_md3File.Frames[lFirstFrame].Min.x;
	box[0][1] = m_md3File.Frames[lFirstFrame].Min.y;
	box[0][2] = m_md3File.Frames[lFirstFrame].Max.z;
	box[1][0] = m_md3File.Frames[lFirstFrame].Max.x;
	box[1][1] = m_md3File.Frames[lFirstFrame].Max.y;
	box[1][2] = m_md3File.Frames[lFirstFrame].Max.z;

	m_Dev->SetFVF(D3DFVF_XYZ);
	m_Dev->DrawPrimitiveUP(
		D3DPT_LINELIST,
		1,
		&box,
		sizeof(FLOAT) * 3);
	m_Dev->SetFVF(D3DMD3VERTEX_TYPE);
#endif TESTBOX
}

void CD3D_MD3Mesh::Invalidate()
{
	m_MeshDatas.resize(0);
	m_bValid = false;
}

bool CD3D_MD3Mesh::Validate()
{
	if (m_bValid)
	{
		// Already valid
		return true;
	}

	m_MeshDatas.resize(m_md3File.Header.NumMeshes);
	if (m_MeshDatas.size() != m_md3File.Header.NumMeshes)
	{
		Invalidate();
		return false;
	}

	if (!CreateNormals())
	{
		return false;
	}

	// Create Vertex Buffer, and Index Buffer.
	if (!CreateVB())
	{
		Invalidate();
		return false;
	}

	if (!CreateIB())
	{
		Invalidate();
		return false;
	}

	m_bValid = true;
	return true;
}

HRESULT CD3D_MD3Mesh::GetNumMeshes(
	LONG* lpNumMeshes)
{
	if (!m_bMD3Loaded)
		return E_FAIL;

	if (!lpNumMeshes)
		return E_FAIL;

	*lpNumMeshes = m_md3File.Header.NumMeshes;
	return S_OK;
}

HRESULT CD3D_MD3Mesh::GetShader(
	LONG lMesh,
	LONG lShader,
	char szShaderName[MAX_QPATH],
	LONG* lpShaderNum)
{
	if (!m_bMD3Loaded)
		return E_FAIL;

	if ((lMesh < 1) || (lMesh > m_md3File.Header.NumMeshes))
		return E_FAIL;

	md3Mesh& Mesh = m_md3File.Meshes[lMesh - 1];

	if ((lShader < 1) || (lShader > Mesh.MeshHeader.NumShaders))
		return E_FAIL;

	if (lpShaderNum)
		*lpShaderNum = Mesh.Shaders[lShader - 1].ShaderNum;

	strcpy(szShaderName, Mesh.Shaders[lShader - 1].ShaderName);
	return S_OK;
}

HRESULT CD3D_MD3Mesh::DumpDebug()
{
	FILE* fout = fopen("md3dump.txt", "w");
	DumpMD3DebugData(fout, m_md3File, MD3DUMP_ALL);
	fclose(fout);
	return S_OK;
}

void CD3D_MD3Mesh::SetSkinRefs(CD3D_MD3Skin& Skin)
{
	if (!m_bMD3Loaded)
		return;

	for (md3_int32 i = 0; i < m_md3File.Header.NumMeshes; i++)
	{
		md3Mesh& Mesh = m_md3File.Meshes[i];
		Skin.SetSkinRef(Mesh.MeshHeader.MeshName, i);
	}
}

bool CD3D_MD3Mesh::CreateVB()
{
	assert(m_MeshDatas.size() == m_md3File.Header.NumMeshes);

	//Create a vertex buffer for each mesh.
	for (md3_int32 i = 0; i < m_md3File.Header.NumMeshes; i++)
	{
		const md3Mesh& Mesh = m_md3File.Meshes[i];
		CMeshData& MeshData = m_MeshDatas[i];

		assert(MeshData.Buffers.VB == nullptr);

		MeshData.TempVerts.resize(Mesh.MeshHeader.NumVertices);

		const md3_uint32 dwVerticeSize = Mesh.MeshHeader.NumVertices * sizeof(d3d_md3_vertex);

		//Create the vertex buffer.  We don't fill it with data because
		//we will be interpolating at render time.
		if (FAILED(IDirect3DDevice9_CreateVertexBuffer(
			m_Dev,
			dwVerticeSize,
			D3DUSAGE_WRITEONLY,
			D3DMD3VERTEX_TYPE,
			m_Pool,
			&MeshData.Buffers.VB,
			NULL)))
		{
			return false;
		}

	}

	return true;
}

bool CD3D_MD3Mesh::CreateIB()
{
	assert(m_MeshDatas.size() == m_md3File.Header.NumMeshes);

	std::vector<md3_uint16> TempIndexes;

	//Load data from MD3 file into the index buffer.
	for (md3_int32 i = 0; i < m_md3File.Header.NumMeshes; i++)
	{
		const md3Mesh& Mesh = m_md3File.Meshes[i];
		CMeshData& MeshData = m_MeshDatas[i];

		assert(MeshData.Buffers.IB == nullptr);

		//Create each mesh's index buffer in turn.
		// Allocate memory for temp buffer.
		const md3_uint32 NumIndexes = Mesh.MeshHeader.NumTriangles * 3;
		TempIndexes.resize(NumIndexes);
		if (TempIndexes.size() != NumIndexes)
		{
			return false;
		}

		//Put each index in temp buffer.
		for (md3_int32 j = 0, k = 0; j < Mesh.MeshHeader.NumTriangles; j++, k += 3)
		{
			TempIndexes[k] = static_cast<md3_uint16>(Mesh.Triangles[j].Indexes[0]);
			TempIndexes[k + 1] = static_cast<md3_uint16>(Mesh.Triangles[j].Indexes[1]);
			TempIndexes[k + 2] = static_cast<md3_uint16>(Mesh.Triangles[j].Indexes[2]);
		}

		//Create the index buffer.
		if (FAILED(IDirect3DDevice9_CreateIndexBuffer(
			m_Dev,
			NumIndexes * sizeof(md3_uint16),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			m_Pool,
			&MeshData.Buffers.IB,
			NULL)))
		{	
			return false;
		}


		//Load each index into index buffer.
		void* pBuffer = nullptr;
		IDirect3DIndexBuffer9_Lock(MeshData.Buffers.IB, 0, 0, &pBuffer, 0);
		memcpy(pBuffer, TempIndexes.data(), NumIndexes * sizeof(md3_uint16));
		IDirect3DIndexBuffer9_Unlock(MeshData.Buffers.IB);
	}

	return true;
}

bool CD3D_MD3Mesh::CreateNormals()
{
	assert(m_MeshDatas.size() == m_md3File.Header.NumMeshes);

	for (md3_int32 i = 0; i < m_md3File.Header.NumMeshes; i++)
	{
		const md3Mesh& Mesh = m_md3File.Meshes[i];
		CMeshData& MeshData = m_MeshDatas[i];
		MeshData.Normals.resize(Mesh.MeshHeader.NumVertices * Mesh.MeshHeader.NumFrames);
		if (MeshData.Normals.size() != Mesh.MeshHeader.NumVertices * Mesh.MeshHeader.NumFrames)
		{
			return false;
		}

		for (md3_int32 j = 0; j < (Mesh.MeshHeader.NumVertices * Mesh.MeshHeader.NumFrames); j++)
		{
			MeshData.Normals[j] = CMD3File::DecodeNormalVector(Mesh.Vertexes[j]);
		}
	}
	return true;
}

md3_bool CD3D_MD3Mesh::CreateModel()
{
	m_MeshDatas.resize(m_md3File.Header.NumMeshes);
	if (m_MeshDatas.size() != m_md3File.Header.NumMeshes)
	{
		return false;
	}

	return Validate();
}

void CD3D_MD3Mesh::DeleteModel()
{
	Invalidate();
}

CD3D_MD3Mesh::CBuffers::CBuffers(const CBuffers& Other)
{
	*this = Other;
}

CD3D_MD3Mesh::CBuffers::CBuffers(CBuffers&& Other)
{
	*this = std::move(Other);
}

CD3D_MD3Mesh::CBuffers::~CBuffers()
{
	if (VB)
	{
		VB->Release();
	}

	if (IB)
	{
		IB->Release();
	}
}

CD3D_MD3Mesh::CBuffers& CD3D_MD3Mesh::CBuffers::operator=(const CBuffers& Other)
{
	VB = Other.VB;
	if (VB)
	{
		VB->AddRef();
	}

	IB = Other.IB;
	if (IB)
	{
		IB->AddRef();
	}

	return *this;
}

CD3D_MD3Mesh::CBuffers& CD3D_MD3Mesh::CBuffers::operator=(CBuffers&& Other)
{
	VB = Other.VB;
	Other.VB = nullptr;

	IB = Other.IB;
	Other.IB = nullptr;

	return *this;
}
