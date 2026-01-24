// (c) Beem Media. All rights reserved.

#include "MD3.h"
#include "MD3File.h"
#include "FileSystem/DataStream.h"

static md3_bool ReadMD3Mesh(md3Mesh& Out, CDataStream& In);
static md3_int32 FindMD3Header(CDataStream& In);
static md3_bool ReadMD3Frame(md3Frame& Out, CDataStream& In);
static md3_bool ReadMD3Header(md3Header& Out, CDataStream& In);
static md3_bool ReadMD3MeshHeader(md3MeshHeader& Out, CDataStream& In);
static md3_bool ReadMD3Shader(md3Shader& Out, CDataStream& In);
static md3_bool ReadMD3Tag(md3Tag& Out, CDataStream& In);
static md3_bool ReadMD3TexCoord(md3TexCoord& Out, CDataStream& In);
static md3_bool ReadMD3Triangle(md3Triangle& Out, CDataStream& In);
static md3_bool ReadMD3Vector(md3Vector& Out, CDataStream& In);
static md3_bool ReadMD3Vertex(md3Vertex& Out, CDataStream& In);

md3_bool ReadMD3File(md3File& Out, CDataStream& In)
{
	md3_bool bSuccess = false;
	std::size_t dwBytesRead = 0;
	md3_int32 i = 0;

	const md3_int32 HeaderOffset = FindMD3Header(In);
	In.SeekFromStart(HeaderOffset);

	if (!ReadMD3Header(Out.Header, In))
	{
		return false;
	}

	if (Out.Header.ID != MD3_ID || Out.Header.Version != MD3_VERSION)
	{
		return false;
	}

	// Frames
	Out.Frames.resize(Out.Header.NumFrames);
	if (Out.Frames.size() != Out.Header.NumFrames)
	{
		return false;
	}

	In.SeekFromStart(HeaderOffset + Out.Header.FrameOffset);
	
	for (i = 0; i < Out.Header.NumFrames; i++)
	{
		if (!ReadMD3Frame(Out.Frames[i], In))
		{
			return false;
		}
	}

	// Tags
	Out.Tags.resize(Out.Header.NumTags * Out.Header.NumFrames);
	if (Out.Tags.size() != (Out.Header.NumTags * Out.Header.NumFrames))
	{
		return false;
	}

	In.SeekFromStart(HeaderOffset + Out.Header.TagOffset);

	for (i = 0; i < Out.Header.NumTags * Out.Header.NumFrames; i++)
	{
		if (!ReadMD3Tag(Out.Tags[i], In))
		{
			return false;
		}
	}

	// Meshes
	Out.Meshes.resize(Out.Header.NumMeshes);
	if (Out.Meshes.size() != Out.Header.NumMeshes)
	{
		return false;
	}

	In.SeekFromStart(HeaderOffset + Out.Header.MeshOffset);
	
	for (i = 0; i < Out.Header.NumMeshes; i++)
	{
		if (!ReadMD3Mesh(Out.Meshes[i], In))
		{
			return false;
		}
	}

	return true;
}

void DeleteMD3File(md3File& In)
{
	/* Free frame and tag data. */
	In.Frames.resize(0);
	In.Frames.shrink_to_fit();
	In.Tags.resize(0);
	In.Tags.shrink_to_fit();
	In.Meshes.resize(0);
	In.Meshes.shrink_to_fit();
}

static md3_bool ReadMD3Mesh(md3Mesh& Out, CDataStream& In)
{
	const md3_int32 MeshOffset = static_cast<md3_int32>(In.Tell());

	if (!ReadMD3MeshHeader(Out.MeshHeader, In))
	{
		return false;
	}

	if (Out.MeshHeader.ID != MD3_ID)
	{
		return false;
	}

	// Shaders
	Out.Shaders.resize(Out.MeshHeader.NumShaders);
	if (Out.Shaders.size() != Out.MeshHeader.NumShaders)
	{
		return false;
	}

	In.SeekFromStart(MeshOffset + Out.MeshHeader.ShaderOffset);
	
	for (md3_int32 i = 0; i < Out.MeshHeader.NumShaders; i++)
	{
		if (!ReadMD3Shader(Out.Shaders[i], In))
		{
			return false;
		}
	}

	// Triangles
	Out.Triangles.resize(Out.MeshHeader.NumTriangles);
	if (Out.Triangles.size() != Out.MeshHeader.NumTriangles)
	{
		return false;
	}
	
	In.SeekFromStart(MeshOffset + Out.MeshHeader.TriangleOffset);
	
	for (md3_int32 i = 0; i < Out.MeshHeader.NumTriangles; i++)
	{
		if (!ReadMD3Triangle(Out.Triangles[i], In))
		{
			return false;
		}
	}
	
	// Tex Coords
	Out.TexCoords.resize(Out.MeshHeader.NumVertices);
	if (Out.TexCoords.size() != Out.MeshHeader.NumVertices)
	{
		return false;
	}
	
	In.SeekFromStart(MeshOffset + Out.MeshHeader.TexCoordOffset);
	
	for (md3_int32 i = 0; i < Out.MeshHeader.NumVertices; i++)
	{
		if (!ReadMD3TexCoord(Out.TexCoords[i], In))
		{
			return false;
		}
	}
	
	// Vertexes
	Out.Vertexes.resize(Out.MeshHeader.NumVertices * Out.MeshHeader.NumFrames);
	if (Out.Vertexes.size() != (Out.MeshHeader.NumVertices * Out.MeshHeader.NumFrames))
	{
		return false;
	}
	
	In.SeekFromStart(MeshOffset + Out.MeshHeader.VertexOffset);

	for (md3_int32 i = 0; i < Out.MeshHeader.NumVertices * Out.MeshHeader.NumFrames; i++)
	{
		if (!ReadMD3Vertex(Out.Vertexes[i], In))
		{
			return false;
		}
	}

	// Seek to end of Mesh (should already be there, but just in case).
	In.SeekFromStart(MeshOffset + Out.MeshHeader.MeshDataSize);
	
	return true;
}

static md3_int32 FindMD3Header(CDataStream& In)
{
	/* This function should search through the file byte
	by byte until "IDP3", and MD3_VERSION are found. */
	return 0;
}

static md3_bool ReadMD3Vertex(md3Vertex& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 8;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	In.Read(&Out, ReadSize);
	return true;
}


static md3_bool ReadMD3TexCoord(md3TexCoord& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 8;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	In.Read(&Out, ReadSize);
	return true;
}


static md3_bool ReadMD3Triangle(md3Triangle& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 12;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	In.Read(&Out, ReadSize);
	return true;
}

static md3_bool ReadMD3Shader(md3Shader& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = (MAX_QPATH + 4);
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	In.Read(&Out, ReadSize);
	return true;
}

static md3_bool ReadMD3MeshHeader(md3MeshHeader& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = (MAX_QPATH + (11 * 4));
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	In.Read(&Out, ReadSize);
	return true;
}

static md3_bool ReadMD3Tag(md3Tag& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = (MAX_QPATH + (12 * 4));
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	In.Read(&Out, ReadSize);
	return true;
}


static md3_bool ReadMD3Frame(md3Frame& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = ((12 * 3) + 4 + 16);
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	In.Read(&Out, ReadSize);
	return true;
}

static md3_bool ReadMD3Vector(md3Vector& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 3 * 4;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	In.Read(&Out, ReadSize);
	return true;
}

static md3_bool ReadMD3Header(md3Header& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 11 * 4 + MAX_QPATH;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	In.Read(&Out, ReadSize);
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
