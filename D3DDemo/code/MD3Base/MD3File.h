// (c) Beem Media. All rights reserved.

#pragma once

#include "MD3Types.h"

static const md3_int32 MD3_VERSION = 15;
static const md3_int32 MD3_ID = (*(md3_uint32*)"IDP3");
static const md3_int32 MAX_QPATH = 64;
static const md3_int32 MD3_MAX_FRAMES = 1024;
static const md3_int32 MD3_MAX_SHADERS = 256;
static const md3_int32 MD3_MAX_SURFACES = 32;
static const md3_int32 MD3_MAX_TAGS = 16;
static const md3_int32 MD3_MAX_TRIANGLES = 8192;
static const md3_int32 MD3_MAX_VERTS = 4096;
static const md3_real32 MD3_XYZ_SCALE = (1.0f/64.0f);

struct md3Header
{
	md3_uint32 ID = 0; /* *(md3_uint32*)"IDP3" */
	md3_int32 Version = 0; /* MD3_VERSION */
	md3_char8 Filename[MAX_QPATH] = { }; /* Filename for PK3 usage */
	md3_int32 Flags = 0; /* Unused, ??? */
	md3_int32 NumFrames = 0; /* Number of frame objects */
	md3_int32 NumTags = 0; /* Number of tag objects */
	md3_int32 NumMeshes = 0; /* Number of meshes, not greater than MD3_MAX_SURFACES */
	md3_int32 NumSkins = 0; /* Number of skin objects, unused?*/
	md3_int32 FrameOffset = 0; /* File position where frame objects begin */
	md3_int32 TagOffset = 0; /* File position where tag objects start */
	md3_int32 MeshOffset = 0; /* File position where mesh objects start */
	md3_int32 FileSize = 0; /* File position where relative data ends */
};

struct md3Vector
{
	md3_real32 x = 0.f;
	md3_real32 y = 0.f;
	md3_real32 z = 0.f;
};

struct md3Frame
{
	md3Vector Min; /* First corner of bounding box */
	md3Vector Max; /* Second corner of bounding box */
	md3Vector Origin; /* local origin, usually (0, 0, 0) */
	md3_real32 Radius = 0.f; /* Radius of bounding sphere */
	md3_char8 Name[16] = { }; /* Name of frame ASCII character sting NULL-terminated */
};

struct md3Tag
{
	md3_char8 Name[MAX_QPATH] = { }; /* Name of tag object string, NULL-TERMINATED */
	md3Vector Position; /* Coordinates of tag objects relative to frame origin. */
	md3Vector Axis[3]; /* Orientation of tag object. */
};

struct md3MeshHeader
{
	md3_uint32 ID = 0; /* *(WORD*)"IDP3" */
	md3_char8 MeshName[MAX_QPATH] = { }; /* Name of mesh object. */
	md3_int32 Flags = 0; /* Flags, unused? */
	md3_int32 NumFrames = 0; /* Number of animation frames should match with MD3HEADER */
	md3_int32 NumShaders = 0; /* Number of shader objects, Textures. */
	md3_int32 NumVertices = 0; /* Number of vertices in object */
	md3_int32 NumTriangles = 0; /* Number of triangles defined in this mesh */
	md3_int32 TriangleOffset = 0; /* Relative offset from mesh start where triangle list begins */
	md3_int32 ShaderOffset = 0; /* Offset from start of mesh where shader data begins */
	md3_int32 TexCoordOffset = 0; /* offset from beginning of mesh where texture coordinates begin */
	md3_int32 VertexOffset = 0; /* offset form beginning of mesh where vertex objects starts */
	md3_int32 MeshDataSize = 0; /* offset from beginning of mesh where mesh ends */
};

struct md3Shader
{
	md3_char8 ShaderName[MAX_QPATH]; /* Name of shader NULL-terminated */
	md3_int32 ShaderNum = 0; /* Shade index number */
};

struct md3Triangle
{
	md3_int32 Indexes[3] = { }; /* References to vertex objects used to described a triangle in mesh */
};

struct md3TexCoords
{
	md3_real32 tu = 0.f;
	md3_real32 tv = 0.f;
};

struct md3Vertex
{
	md3_int16 x = 0; /* x-coordinate */
	md3_int16 y = 0; /* y-coordinate */
	md3_int16 z = 0; /* z-coordinate */
	md3_int16 Normal = 0; /* Encoded normal vector */
};

// Dynamic Size Mesh
struct md3Mesh
{
	md3MeshHeader MeshHeader; /* The Mesh Header */
	md3Shader* Shaders = nullptr; /* Shader list */
	md3Triangle* Triangles = nullptr; /* Triangle list */
	md3TexCoords* TexCoords = nullptr; /* Texture coordinate list */
	md3Vertex* Vertexes = nullptr; /* Vertex list */
};

// Dynamic Size File
struct md3File
{
	md3Header Header; /* File Header */
	md3Frame* Frames = nullptr; /* List of md3 frames */
	md3Tag* Tags = nullptr; /* List of md3 tag data */
	md3Mesh* Meshes = nullptr; /* List of md3 meshes */
};

// Fixed Size Mesh (Unused)
struct md3Mesh2
{
	md3MeshHeader MeshHeader;
	md3Shader Shaders[MD3_MAX_SHADERS];
	md3Triangle Triangles[MD3_MAX_TRIANGLES];
	md3TexCoords TexCoords[MD3_MAX_VERTS];
	md3Vertex Vertexes[MD3_MAX_VERTS];
};

// Fixed Size File (Unused)
struct md3File2
{
	md3Header Header;
	md3Frame Frames[MD3_MAX_FRAMES];
	md3Tag Tags[MD3_MAX_TAGS];
	md3Mesh Meshes[MD3_MAX_SURFACES];
};


/* MD3 File reader functions for Windows */

#include <windows.h>

/*
	ReadMD3File and ReadMD3Mesh are more advanced than
	simple file reading.  DeleteMD3File and DeleteMD3Mesh
	must be used if ReadMD3File or ReadMD3Mesh successfuly
	return.

	All other functions are used to correctly read the
	described data.
*/

/* Read and Create an MD3 File in the MD3FILE structure. */
md3_bool ReadMD3File(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Delete an MD3FILE that has been created. */
md3_bool DeleteMD3File(
	LPVOID lpFile);

md3Vector DecodeNormalVector(const md3Vertex& Vertex);
