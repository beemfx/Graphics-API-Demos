#ifndef __MD3FILE_H__
#define __MD3FILE_H__

#include <windows.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#if (defined(WIN32) || defined(WIN64))
/* MD3 and Q3 Definitions */
#define MD3_VERSION       15
#define MD3_ID            (*(DWORD*)"IDP3")
#define MAX_QPATH         64
#define MD3_MAX_FRAMES    1024
#define MD3_MAX_SHADERS   256
#define MD3_MAX_SURFACES  32
#define MD3_MAX_TAGS      16
#define MD3_MAX_TRIANGLES 8192
#define MD3_MAX_VERTS     4096
#define MD3_XYZ_SCALE     (1.0f/64.0f)


/* MD3 File Header */
typedef struct tagMD3HEADER{
	DWORD dwID; /* *(DWORD*)"IDP3" */
	LONG lVersion; /* MD3_VERSION */
	char szFileName[MAX_QPATH]; /* Filename for PK3 usage */
	LONG lFlags; /* Unused, ??? */
	LONG lNumFrames; /* Number of frame objects */
	LONG lNumTags; /* Number of tag objects */
	LONG lNumMeshes; /* Number of meshes, not greater than MD3_MAX_SURFACES */
	LONG lNumSkins; /* Number of skin objects, unused?*/
	LONG lFrameOffset; /* File position where frame objects begin */
	LONG lTagOffset; /* File position where tag objects start */
	LONG lMeshOffset; /* File position where mesh objects start */
	LONG lFileSize; /* File position where relative data ends */
}MD3HEADER, *LPMD3HEADER;

/* MD3 Vector */
typedef struct tagMD3VECTOR{
	FLOAT x;
	FLOAT y;
	FLOAT z;
}MD3VECTOR, *LPMD3VECTOR;

/* MD3 Frame */
typedef struct tagMD3FRAME{
	MD3VECTOR vMin; /* First corner of bounding box */
	MD3VECTOR vMax; /* Second corner of bounding box */
	MD3VECTOR vOrigin; /* local origin, usually (0, 0, 0) */
	FLOAT fRadius; /* Radius of bounding sphere */
	char szName[16]; /* Name of frame ASCII character sting NULL-terminated */
}MD3FRAME, *LPMD3FRAME;

/* MD3 Tag */
typedef struct tagMD3TAG{
	char szName[MAX_QPATH]; /* Name of tag object stirng, NULL-TERMINATED */
	MD3VECTOR vPosition; /* Coordinates of tag objects relative to frame origin. */
	MD3VECTOR vAxis[3]; /* Orientation of tag object. */
}MD3TAG, *LPMD3TAG;

/* MD3 Mesh Header */
typedef struct tagMD3MESHHEADER{
	DWORD dwID; /* *(WORD*)"IDP3" */
	char szMeshName[MAX_QPATH]; /* Name of mesh object. */
	LONG lFlags; /* Flags, unused? */
	LONG lNumFrames; /* Number of animation frames should match with MD3HEADER */
	LONG lNumShaders; /* Number of shader objects, Textures. */
	LONG lNumVertices; /* Number of vertices in object */
	LONG lNumTriangles; /* Number of traingles defined in this mesh */
	LONG lTriangleOffset; /* Relative offset from mesh start where triangle list begins */
	LONG lShaderOffset; /* Offset from start of mesh where shader data begins */
	LONG lTexCoordOffset; /* offset from begining of mesh where texture coordinates begin */
	LONG lVertexOffset; /* offset form begining of mesh wehre vertext objecs starts */
	LONG lMeshDataSize; /* offet from begining of mesh where mesh ends */
}MD3MESHHEADER, *LPMD3MESHHEADER;

/* MD3 Shader */
typedef struct tagMD3SHADER{
	char szShaderName[MAX_QPATH]; /* Name of shader NULL-terminated */
	LONG lShaderNum; /* Shade index number */
}MD3SHADER, *LPMD3SHADER;

/* MD3 Triangle */
typedef struct tagMD3TRIANGLE{
	LONG nIndexes[3]; /* References to vertex objects used to described a triangle in mesh */
}MD3TRIANGLE, *LPMD3TRIANGLE;

/* MD3 Texture Coordinates */
typedef struct tagMD3TEXCOORDS{
	FLOAT tu;
	FLOAT tv;
}MD3TEXCOORDS, *LPMD3TEXCOORDS;

/* MD3 Vertex */
typedef struct tagMD3VERTEX{
	SHORT x; /* x-coordinate */
	SHORT y; /* y-coordinate */
	SHORT z; /* z-coordinate */
	SHORT nNormal; /* Encoded normal vector */
}MD3VERTEX, *LPMD3VERTEX;

/* MD3 Mesh (dynamic size) */
typedef struct tagMD3MESH{
	MD3MESHHEADER md3MeshHeader; /* The Mesh Header */
	MD3SHADER * md3Shader; /* Shader list */
	MD3TRIANGLE * md3Triangle; /* Triangle list */
	MD3TEXCOORDS * md3TexCoords; /* Texture coordinate list */
	MD3VERTEX * md3Vertex; /* Vertex list */
}MD3MESH, *LPMD3MESH;

/* MD3MESH2 is fixed in size. */
typedef struct tagMD3MESH2{
	MD3MESHHEADER md3MeshHeader;
	MD3SHADER md3Shader[MD3_MAX_SHADERS];
	MD3TRIANGLE md3Triangle[MD3_MAX_TRIANGLES];
	MD3TEXCOORDS md3TexCoords[MD3_MAX_VERTS];
	MD3VERTEX md3Vertex[MD3_MAX_VERTS];
}MD3MESH2, *LPMD3MESH2;

/* MD3 File (dynamic size) */
typedef struct tagMD3FILE{
	MD3HEADER md3Header; /* File Header */
	MD3FRAME * md3Frame; /* List of md3 frames */
	MD3TAG * md3Tag; /* List of md3 tag data */
	MD3MESH * md3Mesh; /* List of md3 meshes */
}MD3FILE, *LPMD3FILE;

/* MD3FILE2 is a fixed size structure for md3's. */
typedef struct tagMD3FILE2{
	MD3HEADER md3Header;
	MD3FRAME md3Frame[MD3_MAX_FRAMES];
	MD3TAG md3Tag[MD3_MAX_TAGS];
	MD3MESH md3Mesh[MD3_MAX_SURFACES];
}MD3FILE2, *LPMD3FILE2;

/* MD3 File reader functions for Windows */

/*
	ReadMD3File and ReadMD3Mesh are more advanced than
	simple file reading.  DeleteMD3File and DeleteMD3Mesh
	must be used if ReadMD3File or ReadMD3Mesh successfuly
	return.

	All other functions are used to correctly read the
	described data.
*/

/* Read and Create an MD3 File in the MD3FILE structure. */
BOOL ReadMD3File(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Delete an MD3FILE that has been created. */
BOOL DeleteMD3File(
	LPVOID lpFile);

/* Read and Create an MD3 Mesh (AKA MD3 Surface) in the MD3MESH structure. */
BOOL ReadMD3Mesh(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Delete an MD3MESH that has been created. */
BOOL DeleteMD3Mesh(
	LPVOID lpMesh);

/* Finds the offset in the file in which the MD3 file begins. */
LONG FindMD3Header(
	HANDLE hFile,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3FRAME structure. */
BOOL ReadMD3Frame(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3HEADER structure. */
BOOL ReadMD3Header(
	HANDLE hFile, 
	LPVOID lpBuffer, 
	LPDWORD lpNumBytesRead, 
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3MESHHEADER structure. */
BOOL ReadMD3MeshHeader(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3SHADER structure. */
BOOL ReadMD3Shader(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3TAG structure. */
BOOL ReadMD3Tag(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3TEXCOORD structure. */
BOOL ReadMD3TexCoords(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3TRIANGLE structure. */
BOOL ReadMD3Triangle(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3VECTOR structure. */
BOOL ReadMD3Vector(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);

/* Read data from a file into MD3VERTEX structure. */
BOOL ReadMD3Vertex(
	HANDLE hFile,
	LPVOID lpBuffer,
	LPDWORD lpNumBytesRead,
	LPOVERLAPPED lpOverlapped);


#endif /* (defined(WIN32) || defined(WIN64)) */
/*
	Generic MD3 Functions.
*/
/* Dumps copy of MD3 data to a file. */
#define MD3DUMP_BONEFRAME    0x00000001l
#define MD3DUMP_TAG          0x00000002l
#define MD3DUMP_MESH         0x00000004l
#define MD3DUMP_MESHSHADER   0x00000008l
#define MD3DUMP_MESHTRI      0x00000010l
#define MD3DUMP_MESHTEXCOORD 0x00000020l
#define MD3DUMP_MESHVERTEX   0x00000040l
#define MD3DUMP_ALL          0xFFFFFFFFl

BOOL DumpMD3DebugData(LPVOID fout, LPMD3FILE lpFile, DWORD dwDumpFlags);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __MD3FILE_H__ */