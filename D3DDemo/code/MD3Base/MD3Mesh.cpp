#define D3D_MD3
#include "MD3.h"
#include <d3dx9.h>
#include <stdio.h>
#include "defines.h"
#include "Functions.h"
#include "MD3Data.h"

///////////////////////////////////
///  Constructor and Destructor ///
///////////////////////////////////

CMD3Mesh::CMD3Mesh()
{
	m_md3File.Meshes=NULL;
	m_md3File.Frames=NULL;
	m_md3File.Tags=NULL;
	m_md3File.Header = { };

	m_lppIB=NULL;
	m_lppVB=NULL;
	m_lpVertices=NULL;

	m_bMD3Loaded=FALSE;
	m_bValid=FALSE;
	m_dwNumSkins=0;

	m_lppNormals=NULL;

	m_Pool=D3DPOOL_DEFAULT;

	m_lpDevice=NULL;
}

CMD3Mesh::~CMD3Mesh()
{
	ClearMD3();
}

///////////////////////////////
///  Public member methods  ///
///////////////////////////////

HRESULT CMD3Mesh::GetNumTags(
	LONG * lpNumTags)
{
	if(m_bMD3Loaded){
		*lpNumTags=m_md3File.Header.NumTags;
		return S_OK;
	}else{
		*lpNumTags=0;
		return E_FAIL;
	}
}
HRESULT CMD3Mesh::GetTagName(
	LONG lRef,
	char szTagName[MAX_QPATH])
{
	if(m_bMD3Loaded && (lRef>0) && (lRef<=m_md3File.Header.NumTags)){
		strcpy(szTagName, m_md3File.Tags[lRef-1].Name);
		return S_OK;
	}else{
		strcpy(szTagName, "");
		return E_FAIL;
	}
}

HRESULT CMD3Mesh::GetTagTranslation(
	DWORD dwTagRef,
	FLOAT fTime,
	LONG dwFirstFrame,
	LONG dwSecondFrame,
	D3DMATRIX * Translation)
{
	D3DXMATRIX FirstFrame, SecondFrame, Final;
	D3DXQUATERNION FirstQuat, SecondQuat, FinalQuat;
	LONG lTagRefFirst=0, lTagRefSecond=0;
	FLOAT x=0.0f, y=0.0f, z=0.0f;
	
	lTagRefFirst=dwFirstFrame*m_md3File.Header.NumTags+dwTagRef-1;
	lTagRefSecond=dwSecondFrame*m_md3File.Header.NumTags+dwTagRef-1;

	D3DXMatrixIdentity((D3DXMATRIX*)Translation);

	if( (dwTagRef<1) || (((LONG)dwTagRef) > m_md3File.Header.NumTags))
		return E_FAIL;

	if( (dwFirstFrame < 0) ||
		(dwSecondFrame < 0) ||
		(dwFirstFrame >= m_md3File.Header.NumFrames) ||
		(dwSecondFrame >= m_md3File.Header.NumFrames))
		return E_FAIL;
	
	FirstFrame._11=m_md3File.Tags[lTagRefFirst].Axis[0].x;
	FirstFrame._12=m_md3File.Tags[lTagRefFirst].Axis[0].y;
	FirstFrame._13=m_md3File.Tags[lTagRefFirst].Axis[0].z;
	FirstFrame._14=0;

	FirstFrame._21=m_md3File.Tags[lTagRefFirst].Axis[1].x;
	FirstFrame._22=m_md3File.Tags[lTagRefFirst].Axis[1].y;
	FirstFrame._23=m_md3File.Tags[lTagRefFirst].Axis[1].z;
	FirstFrame._24=0;

	FirstFrame._31=m_md3File.Tags[lTagRefFirst].Axis[2].x;
	FirstFrame._32=m_md3File.Tags[lTagRefFirst].Axis[2].y;
	FirstFrame._33=m_md3File.Tags[lTagRefFirst].Axis[2].z;
	FirstFrame._34=0;

	FirstFrame._41=0;
	FirstFrame._42=0;
	FirstFrame._43=0;
	FirstFrame._44=1;

	//If both frames are the same this is a much simpler proccess

	SecondFrame._11=m_md3File.Tags[lTagRefSecond].Axis[0].x;
	SecondFrame._12=m_md3File.Tags[lTagRefSecond].Axis[0].y;
	SecondFrame._13=m_md3File.Tags[lTagRefSecond].Axis[0].z;
	SecondFrame._14=0;

	SecondFrame._21=m_md3File.Tags[lTagRefSecond].Axis[1].x;
	SecondFrame._22=m_md3File.Tags[lTagRefSecond].Axis[1].y;
	SecondFrame._23=m_md3File.Tags[lTagRefSecond].Axis[1].z;
	SecondFrame._24=0;

	SecondFrame._31=m_md3File.Tags[lTagRefSecond].Axis[2].x;
	SecondFrame._32=m_md3File.Tags[lTagRefSecond].Axis[2].y;
	SecondFrame._33=m_md3File.Tags[lTagRefSecond].Axis[2].z;
	SecondFrame._34=0;

	SecondFrame._41=0;
	SecondFrame._42=0;
	SecondFrame._43=0;
	SecondFrame._44=1;

	D3DXQuaternionRotationMatrix(&FirstQuat, &FirstFrame);
	D3DXQuaternionRotationMatrix(&SecondQuat, &SecondFrame);
	D3DXQuaternionSlerp(&FinalQuat, &FirstQuat, &SecondQuat, fTime);
	D3DXMatrixRotationQuaternion(&Final, &FinalQuat);

	//Interpolate translation vector and stick it in the final matrix.
	x = m_md3File.Tags[lTagRefFirst].Position.x + 
			fTime * 
			(	m_md3File.Tags[lTagRefSecond].Position.x - 
				m_md3File.Tags[lTagRefFirst].Position.x);

	y = m_md3File.Tags[lTagRefFirst].Position.y + 
			fTime * 
			(	m_md3File.Tags[lTagRefSecond].Position.y - 
				m_md3File.Tags[lTagRefFirst].Position.y);

	z = m_md3File.Tags[lTagRefFirst].Position.z + 
			fTime * 
			(	m_md3File.Tags[lTagRefSecond].Position.z - 
				m_md3File.Tags[lTagRefFirst].Position.z);


	Final._41=x;
	Final._42=y;
	Final._43=z;
	Final._44=1;

	*Translation=Final;
	return S_OK;
}


HRESULT CMD3Mesh::LoadMD3A(
	char szFilename[MAX_PATH], 
	LPDWORD lpBytesRead, 
	LPDIRECT3DDEVICE9 lpDevice,
	D3DPOOL Pool)
{
	DWORD dwBytesRead=0;
	HANDLE hFile=NULL;
	HRESULT hr=0;

	// Clear any MD3 that might exist.
	ClearMD3();

	// Load the file.
	hFile=CreateFileA(
		szFilename,
		GENERIC_READ,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);


	//If the file failed to load we return the last error.
	if(hFile==INVALID_HANDLE_VALUE)
		return E_FAIL;


	//Read the MD3 File, insuring that it really is an MD3 file.
	if(!ReadMD3File(hFile, &m_md3File, &dwBytesRead, NULL)){
		CloseHandle(hFile);
		return E_FAIL;
	}

	//We have the data so we can close the file, and set the number
	//of bytes read.
	CloseHandle(hFile);
	if(lpBytesRead)
		*lpBytesRead=dwBytesRead;

	m_Pool=Pool;

	m_lpDevice=lpDevice;
	m_lpDevice->AddRef();

	hr=CreateModel();
	if(SUCCEEDED(hr)){
		m_bMD3Loaded=TRUE;
	}
	else
	{
		SAFE_RELEASE(m_lpDevice);
	}


	return hr;
}

HRESULT CMD3Mesh::LoadMD3W(
	WCHAR szFilename[MAX_PATH], 
	LPDWORD lpBytesRead, 
	LPDIRECT3DDEVICE9 lpDevice,
	D3DPOOL Pool)
{
	DWORD dwBytesRead=0;
	HANDLE hFile=NULL;
	HRESULT hr=0;

	// Clear any MD3 that might exist.
	ClearMD3();

	// Load the file.
	hFile=CreateFileW(
		szFilename,
		GENERIC_READ,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	//If the file failed to load we return the last error.
	if(hFile==INVALID_HANDLE_VALUE)
		return E_FAIL;

	//Read the MD3 File, insuring that it really is an MD3 file.
	if(!ReadMD3File(hFile, &m_md3File, &dwBytesRead, NULL)){
		CloseHandle(hFile);
		return E_FAIL;
	}

	//We have the data so we can close the file, and set the number
	//of bytes read.
	CloseHandle(hFile);
	if(lpBytesRead)
		*lpBytesRead=dwBytesRead;

	m_Pool=Pool;

	m_lpDevice=lpDevice;
	m_lpDevice->AddRef();

	hr=CreateModel();
	if(SUCCEEDED(hr)){
		m_bMD3Loaded=TRUE;
	}
	else
	{
		SAFE_RELEASE(m_lpDevice);
	}

	return hr;
}

HRESULT CMD3Mesh::ClearMD3()
{
	//If an MD3 is loaded delete the model and the file.
	if(m_bMD3Loaded){
		DeleteModel();
		DeleteMD3File(&m_md3File);
		SAFE_RELEASE(m_lpDevice);
	}

	//Set loaded to false.
	m_bMD3Loaded=FALSE;
	return S_OK;
}

HRESULT CMD3Mesh::RenderWithTexture(
	LPDIRECT3DTEXTURE9 lpTexture,
	LONG lMesh,
	FLOAT fTime,
	LONG lFirstFrame,
	LONG lNextFrame,
	DWORD dwFlags)
{
	LONG lNumVertices=0;
	LONG i=0, j=0;
	FLOAT fFirstX=0.0f, fFirstY=0.0f, fFirstZ=0.0f;
	FLOAT fNextX=0.0f, fNextY=0.0f, fNextZ=0.0f;
	LPVOID lpBuffer=NULL;

	//D3D data that should be restored after the render.
	DWORD dwPrevCullMode=0;
	LPDIRECT3DTEXTURE9 lpPrevTexture=NULL;

	if(!m_lpDevice)
		return E_FAIL;

	if( (lMesh < 1) || (lMesh>m_md3File.Header.NumMeshes) )
		return E_FAIL;

	//Make sure the frames are within the appropriate range.
	if((lFirstFrame < 0) || (lFirstFrame>=m_md3File.Header.NumFrames))
		return S_FALSE;
	if((lNextFrame < 0) || (lNextFrame>=m_md3File.Header.NumFrames))
		return S_FALSE;

	//Get the D3D data that should be restored.
	m_lpDevice->GetRenderState(D3DRS_CULLMODE, &dwPrevCullMode);
	m_lpDevice->GetTexture(0, (IDirect3DBaseTexture9**)&lpPrevTexture);
	
	if(MD3TEXRENDER_NOCULL==(MD3TEXRENDER_NOCULL&dwFlags))
		m_lpDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	else
		m_lpDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	lNumVertices=m_md3File.Meshes[lMesh-1].MeshHeader.NumVertices;
	for(i=0; i<lNumVertices; i++){
		fFirstX=m_md3File.Meshes[lMesh-1].Vertexes[i+lFirstFrame*lNumVertices].x*MD3_XYZ_SCALE;
		fFirstY=m_md3File.Meshes[lMesh-1].Vertexes[i+lFirstFrame*lNumVertices].y*MD3_XYZ_SCALE;
		fFirstZ=m_md3File.Meshes[lMesh-1].Vertexes[i+lFirstFrame*lNumVertices].z*MD3_XYZ_SCALE;

		fNextX=m_md3File.Meshes[lMesh-1].Vertexes[i+lNextFrame*lNumVertices].x*MD3_XYZ_SCALE;
		fNextY=m_md3File.Meshes[lMesh-1].Vertexes[i+lNextFrame*lNumVertices].y*MD3_XYZ_SCALE;
		fNextZ=m_md3File.Meshes[lMesh-1].Vertexes[i+lNextFrame*lNumVertices].z*MD3_XYZ_SCALE;

		//Interpolate the first and second frames.
		m_lpVertices[i].Postion.x=(fFirstX + fTime*(fNextX-fFirstX));
		m_lpVertices[i].Postion.y=(fFirstY + fTime*(fNextY-fFirstY));
		m_lpVertices[i].Postion.z=(fFirstZ + fTime*(fNextZ-fFirstZ));
			
		//Interpolate Normal vector.
		fFirstX=m_lppNormals[lMesh-1][i+lFirstFrame*lNumVertices].x;
		fFirstY=m_lppNormals[lMesh-1][i+lFirstFrame*lNumVertices].y;
		fFirstZ=m_lppNormals[lMesh-1][i+lFirstFrame*lNumVertices].z;

		fNextX=m_lppNormals[lMesh-1][i+lNextFrame*lNumVertices].x;
		fNextY=m_lppNormals[lMesh-1][i+lNextFrame*lNumVertices].y;
		fNextZ=m_lppNormals[lMesh-1][i+lNextFrame*lNumVertices].z;
			
		m_lpVertices[i].Normal.x=(fFirstX + fTime*(fNextX-fFirstX));
		m_lpVertices[i].Normal.y=(fFirstY + fTime*(fNextY-fFirstY));
		m_lpVertices[i].Normal.z=(fFirstZ + fTime*(fNextZ-fFirstZ));

		//Get the texture coordinates.
		m_lpVertices[i].tu=m_md3File.Meshes[lMesh-1].TexCoords[i].tu;
		m_lpVertices[i].tv=m_md3File.Meshes[lMesh-1].TexCoords[i].tv;
	}

	IDirect3DVertexBuffer9_Lock(m_lppVB[lMesh-1], 0, 0, &lpBuffer, 0);
	memcpy(lpBuffer, m_lpVertices, sizeof(D3DMD3VERTEX)*lNumVertices);
	IDirect3DVertexBuffer9_Unlock(m_lppVB[lMesh-1]);

	IDirect3DDevice9_SetStreamSource(m_lpDevice, 0, m_lppVB[lMesh-1], 0, sizeof(D3DMD3VERTEX));
	IDirect3DDevice9_SetIndices(m_lpDevice, m_lppIB[lMesh-1]);
	
	m_lpDevice->SetTexture(0, lpTexture);
	IDirect3DDevice9_SetFVF(m_lpDevice, D3DMD3VERTEX_TYPE);

	IDirect3DDevice9_DrawIndexedPrimitive(
		m_lpDevice,
		D3DPT_TRIANGLELIST,
		0,
		0,
		m_md3File.Meshes[lMesh-1].MeshHeader.NumVertices,
		0,
		m_md3File.Meshes[lMesh-1].MeshHeader.NumTriangles);

	//#define DRAW_NORMAL_MESH
	#ifdef DRAW_NORMAL_MESH
	FLOAT Line[6];
	m_lpDevice->SetTexture(0, NULL);
	for(int k=0; k<m_md3File.Meshes[lMesh-1].MeshHeader.NumVertices; k++)
	{
		Line[0]=m_lpVertices[k].Postion.x;
		Line[1]=m_lpVertices[k].Postion.y;
		Line[2]=m_lpVertices[k].Postion.z;
		Line[3]=m_lpVertices[k].Normal.x;
		Line[4]=m_lpVertices[k].Normal.y;
		Line[5]=m_lpVertices[k].Normal.z;
		m_lpDevice->SetFVF(D3DFVF_XYZ);
		m_lpDevice->DrawPrimitiveUP(
			D3DPT_LINELIST,
			1,
			&Line,
			sizeof(FLOAT)*3);
		m_lpDevice->SetFVF(D3DMD3VERTEX_TYPE);
	}
	#endif //DRAW_NORMAL_MESH

	//Restore saved D3D data.
	m_lpDevice->SetRenderState(D3DRS_CULLMODE, dwPrevCullMode);
	m_lpDevice->SetTexture(0, lpPrevTexture);
	SAFE_RELEASE(lpPrevTexture);
	return S_OK;
}


HRESULT CMD3Mesh::Render(
	CMD3SkinFile * lpSkin,
	FLOAT fTime,
	LONG lFirstFrame,
	LONG lNextFrame,
	DWORD dwFlags)
{
	LONG i=0, j=0;
	LONG lVertexOffset=0;
	LONG lNumVertices=0;

	FLOAT fFirstX=0.0f;
	FLOAT fFirstY=0.0f;
	FLOAT fFirstZ=0.0f;

	FLOAT fNextX=0.0f;
	FLOAT fNextY=0.0f;
	FLOAT fNextZ=0.0f;

	LPVOID lpBuffer=NULL;

	LPDIRECT3DTEXTURE9 lpTexture=NULL;
	
	//Bail if MD3 is not loaded is is currently not valid.
	if(!m_bMD3Loaded)
		return S_FALSE;
	if(!m_bValid)
		return S_FALSE;

	//Make sure the frames are within the appropriate range.
	if((lFirstFrame < 0) || (lFirstFrame>=m_md3File.Header.NumFrames))
		return S_FALSE;
	if((lNextFrame < 0) || (lNextFrame>=m_md3File.Header.NumFrames))
		return S_FALSE;

	//Insure that fTime is between 0.0f and 1.0f.
	if(fTime>1.0f){
		fTime-=(LONG)fTime;
	}
	

	//Loop for each mesh, interpolate the frames, and render the mesh.
	for(i=0; i<m_md3File.Header.NumMeshes; i++){
		lpSkin->GetTexturePointer(i, &lpTexture);
		if(lpTexture)
		{
			RenderWithTexture(
				lpTexture,
				i+1,
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
	box[0][0]=m_md3File.Frames[lFirstFrame].Min.x;
	box[0][1]=m_md3File.Frames[lFirstFrame].Min.y;
	box[0][2]=m_md3File.Frames[lFirstFrame].Max.z;
	box[1][0]=m_md3File.Frames[lFirstFrame].Max.x;
	box[1][1]=m_md3File.Frames[lFirstFrame].Max.y;
	box[1][2]=m_md3File.Frames[lFirstFrame].Max.z;

	m_lpDevice->SetFVF(D3DFVF_XYZ);
		m_lpDevice->DrawPrimitiveUP(
			D3DPT_LINELIST,
			1,
			&box,
			sizeof(FLOAT)*3);
	m_lpDevice->SetFVF(D3DMD3VERTEX_TYPE);
	#endif TESTBOX

	return S_OK;
}

HRESULT CMD3Mesh::Invalidate()
{
	HRESULT hr=S_OK;
	HRESULT hrReturn=S_OK;

	//If MD3 is loaded silently return.
	if(!m_bMD3Loaded)
		return S_FALSE;

	//If MD3 is currently not valid we return.
	if(!m_bValid)
		return S_FALSE;

	// Delete Index Buffer and Vertex Buffer.
	hr=DeleteIB();
	if(FAILED(hr)){
		//Should set more specifict error message.
		hrReturn=E_FAIL;
	}

	hr=DeleteVB();
	if(FAILED(hr)){
		// Should send more specific error message. 
		hrReturn=E_FAIL;
	}
	m_bValid=FALSE;
	return hrReturn;
}

HRESULT CMD3Mesh::Validate()
{
	HRESULT hr=0;

	//If already validated we silently return.
	if(m_bValid)
		return S_FALSE;

	// Create Vertex Buffer, and Index Buffer.
	hr=CreateVB();
	if(FAILED(hr))
		return E_FAIL;

	hr=CreateIB();
	if(FAILED(hr)){
		DeleteVB();
		return E_FAIL;
	}

	m_bValid=TRUE;
	return S_OK;
}

HRESULT CMD3Mesh::GetNumMeshes(
	LONG * lpNumMeshes)
{
	if(!m_bMD3Loaded)
		return E_FAIL;

	if(!lpNumMeshes)
		return E_FAIL;

	*lpNumMeshes=m_md3File.Header.NumMeshes;
	return S_OK;
}

HRESULT CMD3Mesh::GetShader(
	LONG lMesh,
	LONG lShader,
	char szShaderName[MAX_QPATH],
	LONG * lpShaderNum)
{
	if(!m_bMD3Loaded)
		return E_FAIL;

	if( (lMesh < 1) || (lMesh > m_md3File.Header.NumMeshes) )
		return E_FAIL;

	if( (lShader <1) || (lShader > m_md3File.Meshes[lMesh-1].MeshHeader.NumShaders) )
		return E_FAIL;

	if(lpShaderNum)
		*lpShaderNum=m_md3File.Meshes[lMesh-1].Shaders[lShader-1].ShaderNum;

	strcpy(szShaderName, m_md3File.Meshes[lMesh-1].Shaders[lShader-1].ShaderName);
	return S_OK;
}

HRESULT CMD3Mesh::DumpDebug()
{
	FILE * fout=fopen("md3dump.txt", "w");
	DumpMD3DebugData(fout, &m_md3File, MD3DUMP_ALL);
	fclose(fout);
	return S_OK;
}

HRESULT CMD3Mesh::SetSkinRefs(
	CMD3SkinFile * lpSkin)
{
	LONG i=0;
	if(!m_bMD3Loaded)
		return E_FAIL;

	for(i=0; i<m_md3File.Header.NumMeshes; i++){
		lpSkin->SetSkinRef(m_md3File.Meshes[i].MeshHeader.MeshName, i);
	}
	return S_OK;
}

////////////////////////////////
///  Private Member Methods  ///
////////////////////////////////


HRESULT CMD3Mesh::CreateVB()
{
	LONG i=0, j=0;
	DWORD dwVerticeSize=0;
	
	//Set each vertex buffer to NULL.
	for(i=0; i<m_md3File.Header.NumMeshes; i++){
		m_lppVB[i]=NULL;
	}

	//Create a vertex buffer for each mesh.
	for(i=0; i<m_md3File.Header.NumMeshes; i++){
		dwVerticeSize=m_md3File.Meshes[i].MeshHeader.NumVertices * sizeof(D3DMD3VERTEX);

		//Create the vertex buffer.  We don't fill it with data because
		//we will be interpolating at render time.
		if(FAILED(IDirect3DDevice9_CreateVertexBuffer(
			m_lpDevice,
			dwVerticeSize,
			D3DUSAGE_WRITEONLY,
			D3DMD3VERTEX_TYPE,
			m_Pool,
			&(m_lppVB[i]),
			NULL)))
		{
			for(j=0; j<i; j++){
				SAFE_RELEASE(m_lppVB[j]);
			}	
			return E_FAIL;
		}
			
	}
	return S_OK;
}

HRESULT CMD3Mesh::DeleteVB()
{
	LONG i=0;
	for(i=0; i<m_md3File.Header.NumMeshes; i++){
		SAFE_RELEASE((m_lppVB[i]));
	}
	return S_OK;
}

HRESULT CMD3Mesh::CreateIB()
{
	//This function should check to make sure everything worked properly
	//not just assume it did.
	LONG i=0, j=0, k=0;
	SHORT * pIndices=NULL;
	LPVOID pBuffer=NULL;
	DWORD dwIndexSize=0;

	for(i=0; i<m_md3File.Header.NumMeshes; i++){
		m_lppIB[i]=NULL;
	}
	//Load data from MD3 file into the index buffer.
	for(i=0; i<m_md3File.Header.NumMeshes; i++){
		//Create each mesh's index buffer in turn.
		// Allocate memory for temp buffer.
		dwIndexSize=m_md3File.Meshes[i].MeshHeader.NumTriangles * 3 * sizeof(SHORT);
		pIndices=(SHORT*)malloc( dwIndexSize );

		if(pIndices==NULL){
			for(j=0; j<i; j++){
				SAFE_RELEASE(m_lppIB[j]);
			}
			return E_FAIL;
		}

		//Put each index in temp buffer.
		for(j=0, k=0; j<m_md3File.Meshes[i].MeshHeader.NumTriangles; j++, k+=3){
			pIndices[k]=(SHORT)m_md3File.Meshes[i].Triangles[j].Indexes[0];
			pIndices[k+1]=(SHORT)m_md3File.Meshes[i].Triangles[j].Indexes[1];
			pIndices[k+2]=(SHORT)m_md3File.Meshes[i].Triangles[j].Indexes[2];
		}

		//Create the index buffer.
		if(FAILED(IDirect3DDevice9_CreateIndexBuffer(
			m_lpDevice,
			dwIndexSize,
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			m_Pool,
			&(m_lppIB[i]),
			NULL)))
		{
			for(j=0; j<i; j++){
				SAFE_RELEASE(m_lppIB[j]);
				SAFE_FREE(pIndices);
			}
			return E_FAIL;
		}


		//Load each index into index buffer.
		IDirect3DIndexBuffer9_Lock(m_lppIB[i], 0, 0, &pBuffer, 0);
		memcpy(pBuffer, pIndices, dwIndexSize);
		IDirect3DIndexBuffer9_Unlock(m_lppIB[i]);

		//Clear the temp index buffer.
		SAFE_FREE(pIndices);
	}
	return S_OK;
}

HRESULT CMD3Mesh::DeleteIB()
{
	LONG i=0;
	for(i=0; i<m_md3File.Header.NumMeshes; i++){
		SAFE_RELEASE((m_lppIB[i]));
	}
	return S_OK;
}

HRESULT CMD3Mesh::CreateNormals()
{
	LONG i=0, j=0;
	
	//Get the normals.
	m_lppNormals=(md3Vector**)malloc(sizeof(md3Vector*) * m_md3File.Header.NumMeshes);
	if(m_lppNormals==NULL)
		return E_FAIL;

	
	for(i=0; i<m_md3File.Header.NumMeshes; i++)
	{
		m_lppNormals[i]=(md3Vector*)malloc(sizeof(md3Vector) * m_md3File.Meshes[i].MeshHeader.NumVertices * m_md3File.Meshes[i].MeshHeader.NumFrames);
		if(m_lppNormals[i]==NULL)
		{
			for(j=0; j<i; j++)
			{
				SAFE_FREE(m_lppNormals[j]);
			}
			SAFE_FREE(m_lppNormals);
			return E_FAIL;
		}

		for(j=0; j<(m_md3File.Meshes[i].MeshHeader.NumVertices*m_md3File.Meshes[i].MeshHeader.NumFrames); j++)
		{
			m_lppNormals[i][j] = MD3_DecodeNormalVector(m_md3File.Meshes[i].Vertexes[j]);
		}
	}
	return S_OK;
}

HRESULT CMD3Mesh::CreateModel()
{
	LONG i=0, j=0;
	DWORD dwSizeVB=0;
	// Allocate memory for the Direct3D objects
	m_lppVB=(LPDIRECT3DVERTEXBUFFER9*)malloc( sizeof(LPDIRECT3DVERTEXBUFFER9) * (m_md3File.Header.NumMeshes) );
	if(m_lppVB == NULL){
		return E_FAIL;
	}

	m_lppIB=(LPDIRECT3DINDEXBUFFER9*)malloc( sizeof(LPDIRECT3DINDEXBUFFER9) * (m_md3File.Header.NumMeshes) );
	if(m_lppIB == NULL){
		SAFE_FREE(m_lppVB);
		return E_FAIL;
	}

	//I should probably only create on3 interpolated vertex buffer
	//the size of the larges vertex buffer.
	for(i=0; i<m_md3File.Header.NumMeshes; i++){
		dwSizeVB = (m_md3File.Meshes[i].MeshHeader.NumVertices > (LONG)dwSizeVB) ? m_md3File.Meshes[i].MeshHeader.NumVertices : dwSizeVB;
	}

	//We also create the interpolated vertex buffer.  It
	//should be the size of the largest meshes vertex buffer.
	m_lpVertices=(D3DMD3VERTEX*)malloc(sizeof(D3DMD3VERTEX) * dwSizeVB);
	if(m_lpVertices==NULL){
		SAFE_FREE(m_lppVB);
		SAFE_FREE(m_lppIB);
		return E_FAIL;
	}

	if(FAILED(CreateNormals()))
	{
		SAFE_FREE(m_lppVB);
		SAFE_FREE(m_lppIB);
		SAFE_FREE(m_lpVertices);
		return E_FAIL;
	}

	// i will load the index buffers and vertex buffers.
	if(FAILED(Validate())){
		SAFE_FREE(m_lppIB);
		SAFE_FREE(m_lppVB);
		for(i=0; i<m_md3File.Header.NumMeshes; i++)
		{
			SAFE_FREE(m_lppNormals[i]);
		}
		SAFE_FREE(m_lppNormals);
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CMD3Mesh::DeleteModel()
{
	HRESULT hr=S_OK;
	LONG i=0;
	/// Invalidate objects.
	hr=Invalidate();

	//Delete the interpolated vertex buffer.
	SAFE_FREE( (m_lpVertices) );

	// Free space allocated for D3D Pool Objects.
	SAFE_FREE(m_lppIB);
	SAFE_FREE(m_lppVB);

	//Free space allocated to normals.
	for(i=0; i<m_md3File.Header.NumMeshes; i++)
	{
		SAFE_FREE(m_lppNormals[i]);
	}
	SAFE_FREE(m_lppNormals);

	return hr;
}