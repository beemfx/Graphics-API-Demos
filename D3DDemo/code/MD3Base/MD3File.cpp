// (c) Beem Media. All rights reserved.

#include "MD3File.h"
#include "FileSystem/DataStream.h"

md3_bool CMD3File::Load(CDataStream& In)
{
	const md3_int32 HeaderOffset = FindMD3Header(In);
	In.SeekFromStart(HeaderOffset);

	if (!ReadMD3Header(Header, In))
	{
		return false;
	}

	if (Header.ID != MD3_ID || Header.Version != MD3_VERSION)
	{
		return false;
	}

	// Frames
	Frames.resize(Header.NumFrames);
	if (Frames.size() != Header.NumFrames)
	{
		return false;
	}

	In.SeekFromStart(HeaderOffset + Header.FrameOffset);
	
	for (md3_int32 i = 0; i < Header.NumFrames; i++)
	{
		if (!ReadMD3Frame(Frames[i], In))
		{
			return false;
		}
	}

	// Tags
	Tags.resize(Header.NumTags * Header.NumFrames);
	if (Tags.size() != (Header.NumTags * Header.NumFrames))
	{
		return false;
	}

	In.SeekFromStart(HeaderOffset + Header.TagOffset);

	for (md3_int32 i = 0; i < Header.NumTags * Header.NumFrames; i++)
	{
		if (!ReadMD3Tag(Tags[i], In))
		{
			return false;
		}
	}

	// Meshes
	Meshes.resize(Header.NumMeshes);
	if (Meshes.size() != Header.NumMeshes)
	{
		return false;
	}

	In.SeekFromStart(HeaderOffset + Header.MeshOffset);
	
	for (md3_int32 i = 0; i < Header.NumMeshes; i++)
	{
		if (!ReadMD3Mesh(Meshes[i], In))
		{
			return false;
		}
	}

	return true;
}

void CMD3File::Unload()
{
	Header = { };
	Frames.resize(0);
	Frames.shrink_to_fit();
	Tags.resize(0);
	Tags.shrink_to_fit();
	Meshes.resize(0);
	Meshes.shrink_to_fit();
}

md3Vector CMD3File::DecodeNormalVector(const md3Vertex& Vertex)
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

md3_bool CMD3File::ReadMD3Mesh(md3Mesh& Out, CDataStream& In)
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

md3_int32 CMD3File::FindMD3Header(CDataStream& In)
{
	/* This function should search through the file byte
	by byte until "IDP3", and MD3_VERSION are found. */
	return 0;
}

md3_bool CMD3File::ReadMD3Vertex(md3Vertex& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 8;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	return In.Read(&Out, ReadSize) == ReadSize;
}


md3_bool CMD3File::ReadMD3TexCoord(md3TexCoord& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 8;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	return In.Read(&Out, ReadSize) == ReadSize;
}


md3_bool CMD3File::ReadMD3Triangle(md3Triangle& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 12;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	return In.Read(&Out, ReadSize) == ReadSize;
}

md3_bool CMD3File::ReadMD3Shader(md3Shader& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = (MAX_QPATH + 4);
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	return In.Read(&Out, ReadSize) == ReadSize;
}

md3_bool CMD3File::ReadMD3MeshHeader(md3MeshHeader& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = (MAX_QPATH + (11 * 4));
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	return In.Read(&Out, ReadSize) == ReadSize;
}

md3_bool CMD3File::ReadMD3Tag(md3Tag& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = (MAX_QPATH + (12 * 4));
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	return In.Read(&Out, ReadSize) == ReadSize;
}

md3_bool CMD3File::ReadMD3Frame(md3Frame& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = ((12 * 3) + 4 + 16);
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	return In.Read(&Out, ReadSize) == ReadSize;
}

md3_bool CMD3File::ReadMD3Vector(md3Vector& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 3 * 4;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	return In.Read(&Out, ReadSize) == ReadSize;
}

md3_bool CMD3File::ReadMD3Header(md3Header& Out, CDataStream& In)
{
	static constexpr std::size_t ReadSize = 11 * 4 + MAX_QPATH;
	static_assert(sizeof(Out) == ReadSize, "Wrong Size, may need packing or manual read.");
	return In.Read(&Out, ReadSize) == ReadSize;
}
