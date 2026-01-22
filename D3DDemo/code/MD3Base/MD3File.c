/*
	MD3File.c - Functions for reading an MD3 file.

	Copyright (c) 2003 Blaine Myers
*/

#include "Defines.h"
#include "MD3.h"
#include "MD3File.h"

BOOL ReadMD3File(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	LONG lHeaderOffset=0;
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	LPMD3FILE lpFile=NULL;
	LONG i=0;

	*lpNumBytesRead=0;
	
	lHeaderOffset=FindMD3Header(hFile, lpOverlapped);

	if(hFile==NULL)return FALSE;

	SetFilePointer(hFile, lHeaderOffset, 0, FILE_BEGIN);

	if(FAILED(GetLastError()))return FALSE;

	lpFile=(LPMD3FILE)lpBuffer;

	/* Begin by reading the MD3 File Header. */
	bSuccess=ReadMD3Header(
		hFile,
		&(lpFile->md3Header),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	/* We have the header so lets make sure the information is correct. */

	if( ( lpFile->md3Header.dwID != MD3_ID) )return FALSE;
	if( ( lpFile->md3Header.lVersion != MD3_VERSION) )return FALSE;


	/* Initialize and read frame data. */
	lpFile->md3Frame=malloc(lpFile->md3Header.lNumFrames * sizeof(MD3FRAME));
	if(lpFile->md3Frame == NULL)return FALSE;

	/* Set file pointer to appropriate location, then read the data. */
	SetFilePointer(hFile, lpFile->md3Header.lFrameOffset+lHeaderOffset, 0, FILE_BEGIN);
	for(i=0; i<lpFile->md3Header.lNumFrames; i++){
		if(!ReadMD3Frame(hFile, &(lpFile->md3Frame[i]), &dwBytesRead, lpOverlapped)){
			SAFE_FREE(lpFile->md3Frame);
			return FALSE;
		}
		*lpNumBytesRead+=dwBytesRead;
	}

	/* Initialzie and read tag data. */
	lpFile->md3Tag=malloc(lpFile->md3Header.lNumTags * lpFile->md3Header.lNumFrames * sizeof(MD3TAG) );
	if(lpFile->md3Tag == NULL){
		SAFE_FREE(lpFile->md3Frame);
		return FALSE;
	}

	/* Set file pointer to appropriate location. */
	SetFilePointer(hFile, lpFile->md3Header.lTagOffset + lHeaderOffset, 0, FILE_BEGIN);
	for(i=0; i<lpFile->md3Header.lNumTags * lpFile->md3Header.lNumFrames; i++){
		if(!ReadMD3Tag(hFile, &(lpFile->md3Tag[i]), &dwBytesRead, lpOverlapped)){
			SAFE_FREE(lpFile->md3Frame);
			SAFE_FREE(lpFile->md3Tag);
			return FALSE;
		}
		*lpNumBytesRead+=dwBytesRead;
	}

	/* Next up Meshes need to be read. */

	/* Allocate memory for meshes. */
	lpFile->md3Mesh=malloc(lpFile->md3Header.lNumMeshes * sizeof(MD3MESH));
	if(lpFile->md3Mesh == NULL){
		SAFE_FREE(lpFile->md3Frame);
		SAFE_FREE(lpFile->md3Tag);
		return FALSE;
	}

	/* Set file pointer to appropriate position, then read the meshes. */
	SetFilePointer(hFile, lpFile->md3Header.lMeshOffset + lHeaderOffset, 0, FILE_BEGIN);
	for(i=0; i<lpFile->md3Header.lNumMeshes; i++){
		if(!ReadMD3Mesh(hFile, /*&md3Mesh*/&(lpFile->md3Mesh[i]), &dwBytesRead, lpOverlapped)){
			SAFE_FREE(lpFile->md3Frame);
			SAFE_FREE(lpFile->md3Tag);
			/* Here I need to release any meshes that were created. */
			for(i= (i-1); i>=0; i--){
				DeleteMD3Mesh(&(lpFile->md3Mesh[i]));
			}
			SAFE_FREE((lpFile->md3Mesh));
			return FALSE;
		}
		//DeleteMD3Mesh(&md3Mesh);
		*lpNumBytesRead+=dwBytesRead;
	}

	return TRUE;
}

BOOL DeleteMD3File(
	LPVOID lpFile)
{
	LONG i=0;
	LPMD3FILE lpMd3File = (LPMD3FILE)lpFile;
	if(!lpFile)return FALSE;

	/* Free frame and tag data. */
	SAFE_FREE( ( ((LPMD3FILE)lpFile)->md3Frame ) );
	SAFE_FREE( ( ((LPMD3FILE)lpFile)->md3Tag ) );
	

	/* Free each of the meshes. */
	if( ((LPMD3FILE)lpFile)->md3Mesh){
		for(i=0; i<((LPMD3FILE)lpFile)->md3Header.lNumMeshes; i++){
			DeleteMD3Mesh( &( ((LPMD3FILE)lpFile)->md3Mesh[i] ) );
		}
		/* Free the mesh array. */
		SAFE_FREE( ( ((LPMD3FILE)lpFile)->md3Mesh ) );
	}
	return TRUE;
}

BOOL ReadMD3Mesh(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	LONG lMeshOffset=0;
	LONG i=0;
	LPMD3MESH lpMesh=NULL;
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	*lpNumBytesRead=dwBytesRead;

	lMeshOffset=SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
	lpMesh=(LPMD3MESH)lpBuffer;

	/* First read the mesh header. */
	bSuccess=ReadMD3MeshHeader(
		hFile,
		&(lpMesh->md3MeshHeader),
		&dwBytesRead, lpOverlapped);
	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;
	if(lpMesh->md3MeshHeader.dwID != MD3_ID)return FALSE;

	/* Prepare shader data. */
	lpMesh->md3Shader=malloc(lpMesh->md3MeshHeader.lNumShaders * sizeof(MD3SHADER));
	if(lpMesh->md3Shader == NULL)return FALSE;
	/* Read shader. */
	SetFilePointer(hFile, lMeshOffset+lpMesh->md3MeshHeader.lShaderOffset, 0, FILE_BEGIN);
	for(i=0; i<lpMesh->md3MeshHeader.lNumShaders; i++){
		bSuccess=ReadMD3Shader(
			hFile,
			&(lpMesh->md3Shader[i]),
			&dwBytesRead,
			lpOverlapped);
		if( (!bSuccess) || (dwBytesRead==0) ){
			SAFE_FREE(lpMesh->md3Shader);
			return FALSE;
		}
		*lpNumBytesRead+=dwBytesRead;
	}
	/* Prepare triangle data. */
	lpMesh->md3Triangle=malloc(lpMesh->md3MeshHeader.lNumTriangles * sizeof(MD3TRIANGLE));
	if(lpMesh->md3Triangle == NULL){
		SAFE_FREE(lpMesh->md3Shader);
		return FALSE;
	}
	/* Read shader. */
	SetFilePointer(hFile, lMeshOffset+lpMesh->md3MeshHeader.lTriangleOffset, 0, FILE_BEGIN);
	for(i=0; i<lpMesh->md3MeshHeader.lNumTriangles; i++){
		bSuccess=ReadMD3Triangle(
			hFile,
			&(lpMesh->md3Triangle[i]),
			&dwBytesRead,
			lpOverlapped);
		if( (!bSuccess) || (dwBytesRead==0)){
			SAFE_FREE(lpMesh->md3Shader);
			SAFE_FREE(lpMesh->md3Triangle);
			return FALSE;
		}

		*lpNumBytesRead+=dwBytesRead;
	}
	/* Prepare Texture coordinates. */
	lpMesh->md3TexCoords=malloc(lpMesh->md3MeshHeader.lNumVertices * sizeof(MD3TEXCOORDS));
	if(lpMesh->md3TexCoords == NULL){
		SAFE_FREE(lpMesh->md3Shader);
		SAFE_FREE(lpMesh->md3Triangle);
		return FALSE;
	}
	/* Read texture coordinates. */
	SetFilePointer(hFile, lMeshOffset+lpMesh->md3MeshHeader.lTexCoordOffset, 0, FILE_BEGIN);
	for(i=0; i<lpMesh->md3MeshHeader.lNumVertices; i++){
		bSuccess=ReadMD3TexCoords(
			hFile,
			&(lpMesh->md3TexCoords[i]),
			&dwBytesRead,
			lpOverlapped);
		if( (!bSuccess) || (dwBytesRead==0) ){
			SAFE_FREE(lpMesh->md3Shader);
			SAFE_FREE(lpMesh->md3Triangle);
			SAFE_FREE(lpMesh->md3TexCoords);
			return FALSE;
		}
		*lpNumBytesRead+=dwBytesRead;
	}
	/* Prepare vertex data. */
	lpMesh->md3Vertex=malloc(lpMesh->md3MeshHeader.lNumVertices * lpMesh->md3MeshHeader.lNumFrames * sizeof(MD3VERTEX));
	if(lpMesh->md3Vertex == NULL){
		SAFE_FREE(lpMesh->md3Shader);
		SAFE_FREE(lpMesh->md3Triangle);
		SAFE_FREE(lpMesh->md3TexCoords);
		return FALSE;
	}
	/* Read vertex data. */
	SetFilePointer(hFile, lMeshOffset+lpMesh->md3MeshHeader.lVertexOffset, 0, FILE_BEGIN);
	for(i=0; i<lpMesh->md3MeshHeader.lNumVertices * lpMesh->md3MeshHeader.lNumFrames; i++){
		bSuccess=ReadMD3Vertex(
			hFile,
			&(lpMesh->md3Vertex[i]),
			&dwBytesRead,
			lpOverlapped);
		if( (!bSuccess) || (dwBytesRead==0) ){
			SAFE_FREE(lpMesh->md3Shader);
			SAFE_FREE(lpMesh->md3Triangle);
			SAFE_FREE(lpMesh->md3TexCoords);
			SAFE_FREE(lpMesh->md3Vertex);
			return FALSE;
		}

		*lpNumBytesRead+=dwBytesRead;
	}

	/* Set the file pointer to the end of this surface. */
	SetFilePointer(hFile, lpMesh->md3MeshHeader.lMeshDataSize+lMeshOffset, 0, FILE_BEGIN);

	return TRUE;
}

BOOL DeleteMD3Mesh(
	LPVOID lpMesh)
{
	if(lpMesh==NULL)return FALSE;
	/* Free shader, vertex, texcoords, and triangle data. */
	SAFE_FREE( ( ((LPMD3MESH)lpMesh)->md3Shader ) );
	SAFE_FREE( ( ((LPMD3MESH)lpMesh)->md3Triangle ) );
	SAFE_FREE( ( ((LPMD3MESH)lpMesh)->md3TexCoords ) );
	SAFE_FREE( ( ((LPMD3MESH)lpMesh)->md3Vertex ) );

	return TRUE;
}

LONG FindMD3Header(
	HANDLE hFile,
	LPOVERLAPPED lpOverlapped)
{
	/* This function should seach through the file byte
	by byte until "IDP3", and MD3_VERSION are found. */
	return 0;
}

BOOL ReadMD3Vertex(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	MD3VERTEX md3Vertex;

	ZeroMemory(&md3Vertex, sizeof(MD3VERTEX));
	*lpNumBytesRead=0;

	if(hFile==NULL)return FALSE;

	bSuccess=ReadFile(
		hFile,
		&(md3Vertex.x),
		sizeof(SHORT),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3Vertex.y),
		sizeof(SHORT),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3Vertex.z),
		sizeof(SHORT),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3Vertex.nNormal),
		sizeof(SHORT),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	memcpy(lpBuffer, &md3Vertex, sizeof(MD3VERTEX));
	return TRUE;
}


BOOL ReadMD3TexCoords(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	MD3TEXCOORDS md3TexCoords;

	ZeroMemory(&md3TexCoords, sizeof(MD3TEXCOORDS));
	*lpNumBytesRead=0;

	if(hFile==NULL)return FALSE;

	bSuccess=ReadFile(
		hFile,
		&(md3TexCoords.tu),
		sizeof(FLOAT),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3TexCoords.tv),
		sizeof(FLOAT),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	memcpy(lpBuffer, &md3TexCoords, sizeof(MD3TEXCOORDS));
	return TRUE;
}


BOOL ReadMD3Triangle(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	MD3TRIANGLE md3Triangle;

	ZeroMemory(&md3Triangle, sizeof(MD3TRIANGLE));
	*lpNumBytesRead=0;

	if(hFile==NULL)return FALSE;

	bSuccess=ReadFile(
		hFile,
		&(md3Triangle.nIndexes[0]),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3Triangle.nIndexes[1]),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3Triangle.nIndexes[2]),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;


	memcpy(lpBuffer, &md3Triangle, sizeof(MD3TRIANGLE));
	return TRUE;
}

BOOL ReadMD3Shader(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	MD3SHADER md3Shader;

	ZeroMemory(&md3Shader, sizeof(MD3SHADER));
	*lpNumBytesRead=0;

	if(hFile==NULL)return FALSE;

	bSuccess=ReadFile(
		hFile,
		&(md3Shader.szShaderName),
		MAX_QPATH,
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3Shader.lShaderNum),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	memcpy(lpBuffer, &md3Shader, sizeof(MD3SHADER));
	return TRUE;
}


BOOL ReadMD3MeshHeader(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	MD3MESHHEADER md3MeshHeader;

	ZeroMemory(&md3MeshHeader, sizeof(MD3MESHHEADER));
	*lpNumBytesRead=0;

	if(hFile==NULL)return FALSE;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.dwID),
		sizeof(DWORD),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.szMeshName),
		MAX_QPATH,
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lFlags),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lNumFrames),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lNumShaders),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lNumVertices),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lNumTriangles),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lTriangleOffset),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lShaderOffset),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lTexCoordOffset),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lVertexOffset),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3MeshHeader.lMeshDataSize),
		sizeof(LONG),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;


	memcpy(lpBuffer, &md3MeshHeader, sizeof(MD3MESHHEADER));
	return TRUE;
}

BOOL ReadMD3Tag(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	MD3TAG md3Tag;

	ZeroMemory(&md3Tag, sizeof(MD3TAG));
	*lpNumBytesRead=0;

	if(hFile==NULL)return FALSE;

	bSuccess=ReadFile(
		hFile,
		&(md3Tag.szName),
		MAX_QPATH,
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadMD3Vector(
		hFile,
		&(md3Tag.vPosition),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadMD3Vector(
		hFile,
		&(md3Tag.vAxis[0]),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadMD3Vector(
		hFile,
		&(md3Tag.vAxis[1]),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadMD3Vector(
		hFile,
		&(md3Tag.vAxis[2]),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;


	memcpy(lpBuffer, &md3Tag, sizeof(MD3TAG));
	return TRUE;
}


BOOL ReadMD3Frame(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	MD3FRAME md3Frame;

	ZeroMemory(&md3Frame, sizeof(MD3FRAME));
	*lpNumBytesRead=0;

	if(hFile==NULL)return FALSE;
	
	bSuccess=ReadMD3Vector(
		hFile, 
		&(md3Frame.vMin), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadMD3Vector(
		hFile, 
		&(md3Frame.vMax), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadMD3Vector(
		hFile, 
		&(md3Frame.vOrigin), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3Frame.fRadius),
		sizeof(FLOAT),
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile,
		&(md3Frame.szName),
		16,
		&dwBytesRead,
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;


	memcpy(lpBuffer, &md3Frame, sizeof(MD3FRAME));
	return TRUE;
}

BOOL ReadMD3Vector(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	MD3VECTOR md3Vector;

	ZeroMemory(&md3Vector, sizeof(MD3VECTOR));
	*lpNumBytesRead=0;

	if(hFile==NULL)return FALSE;

	bSuccess=ReadFile(
		hFile, 
		&(md3Vector.x), 
		sizeof(FLOAT), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Vector.y), 
		sizeof(FLOAT), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Vector.z), 
		sizeof(FLOAT), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0))return FALSE;
	*lpNumBytesRead+=dwBytesRead;
	memcpy(lpBuffer, &md3Vector, sizeof(MD3VECTOR));

	return TRUE;
}

BOOL ReadMD3Header(
	HANDLE hFile, 
	LPVOID lpBuffer, 
	LPDWORD lpNumBytesRead, 
	LPOVERLAPPED lpOverlapped)
{
	BOOL bSuccess=FALSE;
	DWORD dwBytesRead=0;
	MD3HEADER md3Header;

	ZeroMemory(&md3Header, sizeof(MD3HEADER));
	*lpNumBytesRead=0;

	if(hFile==NULL)return FALSE;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.dwID), 
		sizeof(DWORD), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lVersion), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.szFileName), 
		MAX_QPATH, 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lFlags), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lNumFrames), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lNumTags), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lNumMeshes), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lNumSkins), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lFrameOffset), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lTagOffset), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lMeshOffset), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	bSuccess=ReadFile(
		hFile, 
		&(md3Header.lFileSize), 
		sizeof(LONG), 
		&dwBytesRead, 
		lpOverlapped);

	if( (!bSuccess) || (dwBytesRead==0) )return FALSE;
	*lpNumBytesRead+=dwBytesRead;

	memcpy(lpBuffer, &md3Header, sizeof(MD3HEADER));
	return TRUE;
}