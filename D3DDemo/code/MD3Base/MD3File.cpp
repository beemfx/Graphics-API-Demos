/*
	MD3File.c - Functions for reading an MD3 file.

	Copyright (c) 2003 Blaine Myers
*/

#include "Defines.h"
#include "MD3.h"
#include "MD3File.h"


/* Read and Create an MD3 Mesh (AKA MD3 Surface) in the MD3MESH structure. */
static md3_bool ReadMD3Mesh(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Finds the offset in the file in which the MD3 file begins. */
static md3_int32 FindMD3Header(
	HANDLE hFile,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3FRAME structure. */
static md3_bool ReadMD3Frame(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3HEADER structure. */
static md3_bool ReadMD3Header(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3MESHHEADER structure. */
static md3_bool ReadMD3MeshHeader(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3SHADER structure. */
static md3_bool ReadMD3Shader(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3TAG structure. */
static md3_bool ReadMD3Tag(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3TEXCOORD structure. */
static md3_bool ReadMD3TexCoords(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3TRIANGLE structure. */
static md3_bool ReadMD3Triangle(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3VECTOR structure. */
static md3_bool ReadMD3Vector(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3VERTEX structure. */
static md3_bool ReadMD3Vertex(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

md3_bool ReadMD3File(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_int32 lHeaderOffset = 0;
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3File* lpFile = nullptr;
	md3_int32 i = 0;

	*lpNumBytesRead = 0;

	lHeaderOffset = FindMD3Header(hFile, lpOverlapped);

	if (hFile == nullptr)return false;

	SetFilePointer(hFile, lHeaderOffset, 0, FILE_BEGIN);

	if (FAILED(GetLastError()))return false;

	lpFile = (md3File*)lpBuffer;

	/* Begin by reading the MD3 File Header. */
	bSuccess = ReadMD3Header(
		hFile,
		&(lpFile->Header),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	/* We have the header so lets make sure the information is correct. */

	if ((lpFile->Header.ID != MD3_ID))return false;
	if ((lpFile->Header.Version != MD3_VERSION))return false;


	/* Initialize and read frame data. */
	lpFile->Frames.resize(lpFile->Header.NumFrames);
	if (lpFile->Frames.size() != lpFile->Header.NumFrames)return false;

	/* Set file pointer to appropriate location, then read the data. */
	SetFilePointer(hFile, lpFile->Header.FrameOffset + lHeaderOffset, 0, FILE_BEGIN);
	for (i = 0; i < lpFile->Header.NumFrames; i++) {
		if (!ReadMD3Frame(hFile, &(lpFile->Frames[i]), &dwBytesRead, lpOverlapped)) {
			return false;
		}
		*lpNumBytesRead += dwBytesRead;
	}

	/* Initialize and read tag data. */
	lpFile->Tags.resize(lpFile->Header.NumTags * lpFile->Header.NumFrames);
	if (lpFile->Tags.size() != (lpFile->Header.NumTags * lpFile->Header.NumFrames))
	{
		return false;
	}

	/* Set file pointer to appropriate location. */
	SetFilePointer(hFile, lpFile->Header.TagOffset + lHeaderOffset, 0, FILE_BEGIN);
	for (i = 0; i < lpFile->Header.NumTags * lpFile->Header.NumFrames; i++) {
		if (!ReadMD3Tag(hFile, &(lpFile->Tags[i]), &dwBytesRead, lpOverlapped)) {
			return false;
		}
		*lpNumBytesRead += dwBytesRead;
	}

	/* Next up Meshes need to be read. */

	/* Allocate memory for meshes. */
	lpFile->Meshes.resize(lpFile->Header.NumMeshes);
	if (lpFile->Meshes.size() != lpFile->Header.NumMeshes) {
		return false;
	}

	/* Set file pointer to appropriate position, then read the meshes. */
	SetFilePointer(hFile, lpFile->Header.MeshOffset + lHeaderOffset, 0, FILE_BEGIN);
	for (i = 0; i < lpFile->Header.NumMeshes; i++) {
		if (!ReadMD3Mesh(hFile, /*&md3Mesh*/&(lpFile->Meshes[i]), &dwBytesRead, lpOverlapped)) {
			return false;
		}
		//DeleteMD3Mesh(&md3Mesh);
		*lpNumBytesRead += dwBytesRead;
	}

	return true;
}

md3_bool DeleteMD3File(
	LPVOID lpFile)
{
	md3_int32 i = 0;
	md3File* lpMd3File = (md3File*)lpFile;
	if (!lpFile)return false;

	/* Free frame and tag data. */
	lpMd3File->Frames.resize(0);
	lpMd3File->Frames.shrink_to_fit();
	lpMd3File->Tags.resize(0);
	lpMd3File->Tags.shrink_to_fit();
	lpMd3File->Meshes.resize(0);
	lpMd3File->Meshes.shrink_to_fit();

	return true;
}

static md3_bool ReadMD3Mesh(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_int32 lMeshOffset = 0;
	md3_int32 i = 0;
	md3Mesh* lpMesh = nullptr;
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	*lpNumBytesRead = dwBytesRead;

	lMeshOffset = SetFilePointer(hFile, 0, nullptr, FILE_CURRENT);
	lpMesh = (md3Mesh*)lpBuffer;

	/* First read the mesh header. */
	bSuccess = ReadMD3MeshHeader(
		hFile,
		&(lpMesh->MeshHeader),
		&dwBytesRead, lpOverlapped);
	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;
	if (lpMesh->MeshHeader.ID != MD3_ID)return false;

	/* Prepare shader data. */
	lpMesh->Shaders.resize(lpMesh->MeshHeader.NumShaders);
	if (lpMesh->Shaders.size() != lpMesh->MeshHeader.NumShaders)return false;
	/* Read shader. */
	SetFilePointer(hFile, lMeshOffset + lpMesh->MeshHeader.ShaderOffset, 0, FILE_BEGIN);
	for (i = 0; i < lpMesh->MeshHeader.NumShaders; i++) {
		bSuccess = ReadMD3Shader(
			hFile,
			&(lpMesh->Shaders[i]),
			&dwBytesRead,
			lpOverlapped);
		if ((!bSuccess) || (dwBytesRead == 0)) {
			return false;
		}
		*lpNumBytesRead += dwBytesRead;
	}
	/* Prepare triangle data. */
	lpMesh->Triangles.resize(lpMesh->MeshHeader.NumTriangles);
	if (lpMesh->Triangles.size() != lpMesh->MeshHeader.NumTriangles) {
		return false;
	}
	/* Read shader. */
	SetFilePointer(hFile, lMeshOffset + lpMesh->MeshHeader.TriangleOffset, 0, FILE_BEGIN);
	for (i = 0; i < lpMesh->MeshHeader.NumTriangles; i++) {
		bSuccess = ReadMD3Triangle(
			hFile,
			&(lpMesh->Triangles[i]),
			&dwBytesRead,
			lpOverlapped);
		if ((!bSuccess) || (dwBytesRead == 0)) {
			return false;
		}

		*lpNumBytesRead += dwBytesRead;
	}
	/* Prepare Texture coordinates. */
	lpMesh->TexCoords.resize(lpMesh->MeshHeader.NumVertices);
	if (lpMesh->TexCoords.size() != lpMesh->MeshHeader.NumVertices) {
		return false;
	}
	/* Read texture coordinates. */
	SetFilePointer(hFile, lMeshOffset + lpMesh->MeshHeader.TexCoordOffset, 0, FILE_BEGIN);
	for (i = 0; i < lpMesh->MeshHeader.NumVertices; i++) {
		bSuccess = ReadMD3TexCoords(
			hFile,
			&(lpMesh->TexCoords[i]),
			&dwBytesRead,
			lpOverlapped);
		if ((!bSuccess) || (dwBytesRead == 0)) {
			return false;
		}
		*lpNumBytesRead += dwBytesRead;
	}
	/* Prepare vertex data. */
	lpMesh->Vertexes.resize(lpMesh->MeshHeader.NumVertices * lpMesh->MeshHeader.NumFrames);
	if (lpMesh->Vertexes.size() != (lpMesh->MeshHeader.NumVertices * lpMesh->MeshHeader.NumFrames))
	{
		return false;
	}
	/* Read vertex data. */
	SetFilePointer(hFile, lMeshOffset + lpMesh->MeshHeader.VertexOffset, 0, FILE_BEGIN);
	for (i = 0; i < lpMesh->MeshHeader.NumVertices * lpMesh->MeshHeader.NumFrames; i++) {
		bSuccess = ReadMD3Vertex(
			hFile,
			&(lpMesh->Vertexes[i]),
			&dwBytesRead,
			lpOverlapped);
		if ((!bSuccess) || (dwBytesRead == 0))
		{
			return false;
		}

		*lpNumBytesRead += dwBytesRead;
	}

	/* Set the file pointer to the end of this surface. */
	SetFilePointer(hFile, lpMesh->MeshHeader.MeshDataSize + lMeshOffset, 0, FILE_BEGIN);

	return true;
}

static md3_int32 FindMD3Header(
	HANDLE hFile,
	LPOVERLAPPED lpOverlapped)
{
	/* This function should seach through the file byte
	by byte until "IDP3", and MD3_VERSION are found. */
	return 0;
}

static md3_bool ReadMD3Vertex(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3Vertex md3Vertex;

	ZeroMemory(&md3Vertex, sizeof(md3Vertex));
	*lpNumBytesRead = 0;

	if (hFile == nullptr)return false;

	bSuccess = ReadFile(
		hFile,
		&(md3Vertex.x),
		sizeof(md3_int16),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Vertex.y),
		sizeof(md3_int16),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Vertex.z),
		sizeof(md3_int16),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Vertex.Normal),
		sizeof(md3_int16),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	memcpy(lpBuffer, &md3Vertex, sizeof(md3Vertex));
	return true;
}


static md3_bool ReadMD3TexCoords(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3TexCoords md3TexCoords;

	ZeroMemory(&md3TexCoords, sizeof(md3TexCoords));
	*lpNumBytesRead = 0;

	if (hFile == nullptr)return false;

	bSuccess = ReadFile(
		hFile,
		&(md3TexCoords.tu),
		sizeof(FLOAT),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3TexCoords.tv),
		sizeof(FLOAT),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	memcpy(lpBuffer, &md3TexCoords, sizeof(md3TexCoords));
	return true;
}


static md3_bool ReadMD3Triangle(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3Triangle md3Triangle;

	ZeroMemory(&md3Triangle, sizeof(md3Triangle));
	*lpNumBytesRead = 0;

	if (hFile == nullptr)return false;

	bSuccess = ReadFile(
		hFile,
		&(md3Triangle.Indexes[0]),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Triangle.Indexes[1]),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Triangle.Indexes[2]),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;


	memcpy(lpBuffer, &md3Triangle, sizeof(md3Triangle));
	return true;
}

static md3_bool ReadMD3Shader(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3Shader md3Shader;

	ZeroMemory(&md3Shader, sizeof(md3Shader));
	*lpNumBytesRead = 0;

	if (hFile == nullptr)return false;

	bSuccess = ReadFile(
		hFile,
		&(md3Shader.ShaderName),
		MAX_QPATH,
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Shader.ShaderNum),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	memcpy(lpBuffer, &md3Shader, sizeof(md3Shader));
	return true;
}


static md3_bool ReadMD3MeshHeader(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3MeshHeader md3MeshHeader;

	ZeroMemory(&md3MeshHeader, sizeof(md3MeshHeader));
	*lpNumBytesRead = 0;

	if (hFile == nullptr)return false;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.ID),
		sizeof(DWORD),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.MeshName),
		MAX_QPATH,
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.Flags),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.NumFrames),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.NumShaders),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.NumVertices),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.NumTriangles),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.TriangleOffset),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.ShaderOffset),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.TexCoordOffset),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.VertexOffset),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3MeshHeader.MeshDataSize),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;


	memcpy(lpBuffer, &md3MeshHeader, sizeof(md3MeshHeader));
	return true;
}

static md3_bool ReadMD3Tag(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3Tag md3Tag;

	ZeroMemory(&md3Tag, sizeof(md3Tag));
	*lpNumBytesRead = 0;

	if (hFile == nullptr)return false;

	bSuccess = ReadFile(
		hFile,
		&(md3Tag.Name),
		MAX_QPATH,
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadMD3Vector(
		hFile,
		&(md3Tag.Position),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadMD3Vector(
		hFile,
		&(md3Tag.Axis[0]),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadMD3Vector(
		hFile,
		&(md3Tag.Axis[1]),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadMD3Vector(
		hFile,
		&(md3Tag.Axis[2]),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;


	memcpy(lpBuffer, &md3Tag, sizeof(md3Tag));
	return true;
}


static md3_bool ReadMD3Frame(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3Frame md3Frame;

	ZeroMemory(&md3Frame, sizeof(md3Frame));
	*lpNumBytesRead = 0;

	if (hFile == nullptr)return false;

	bSuccess = ReadMD3Vector(
		hFile,
		&(md3Frame.Min),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadMD3Vector(
		hFile,
		&(md3Frame.Max),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadMD3Vector(
		hFile,
		&(md3Frame.Origin),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Frame.Radius),
		sizeof(FLOAT),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Frame.Name),
		16,
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;


	memcpy(lpBuffer, &md3Frame, sizeof(md3Frame));
	return true;
}

static md3_bool ReadMD3Vector(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3Vector md3Vector;

	ZeroMemory(&md3Vector, sizeof(md3Vector));
	*lpNumBytesRead = 0;

	if (hFile == nullptr)return false;

	bSuccess = ReadFile(
		hFile,
		&(md3Vector.x),
		sizeof(FLOAT),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Vector.y),
		sizeof(FLOAT),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(md3Vector.z),
		sizeof(FLOAT),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;
	memcpy(lpBuffer, &md3Vector, sizeof(md3Vector));

	return true;
}

static md3_bool ReadMD3Header(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	md3_bool bSuccess = false;
	DWORD dwBytesRead = 0;
	md3Header Header;

	ZeroMemory(&Header, sizeof(md3Header));
	*lpNumBytesRead = 0;

	if (hFile == nullptr)return false;

	bSuccess = ReadFile(
		hFile,
		&(Header.ID),
		sizeof(DWORD),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.Version),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.Filename),
		MAX_QPATH,
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.Flags),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.NumFrames),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.NumTags),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.NumMeshes),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.NumSkins),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.FrameOffset),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.TagOffset),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.MeshOffset),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	bSuccess = ReadFile(
		hFile,
		&(Header.FileSize),
		sizeof(md3_int32),
		&dwBytesRead,
		lpOverlapped);

	if ((!bSuccess) || (dwBytesRead == 0))return false;
	*lpNumBytesRead += dwBytesRead;

	memcpy(lpBuffer, &Header, sizeof(md3Header));
	return true;
}

md3Vector MD3_DecodeNormalVector(const md3Vertex& Vertex)
{
	md3Vector Out;

	md3_real32 lat = 0, lng = 0;

	// Get the latitude and longitude.
	lat = (Vertex.Normal & 0x00FF) * (2.0f * 3.141592654f) / 255.0f;
	lng = ((Vertex.Normal & 0xFF00) >> 8) * (2.0f * 3.141592654f) / 255.0f;
	// Get the x, y, z values.
	Out.x = static_cast<md3_real32>(std::cos(lat) * std::sin(lng));
	Out.y = static_cast<md3_real32>(std::sin(lat) * std::sin(lng));
	Out.z = static_cast<md3_real32>(std::cos(lng));

	return Out;
}
