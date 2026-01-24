// (c) Beem Media. All rights reserved.

#include "MD3Data.h"

static void DumpMD3MeshDebugData(FILE * fout, const md3Mesh* lpMesh, md3_uint32 dwDumpFlags)
{
	fprintf(fout, "\n\n--- MD3 Mesh Header ---\n\n");
	fprintf(fout, "ID: %i\n", lpMesh->MeshHeader.ID);
	fprintf(fout, "Name: %s\n", lpMesh->MeshHeader.MeshName);
	fprintf(fout, "Flags: %i\n", lpMesh->MeshHeader.Flags);
	fprintf(fout, "Num Frames: %i\n", lpMesh->MeshHeader.NumFrames);
	fprintf(fout, "Num Shaders: %i\n", lpMesh->MeshHeader.NumShaders);
	fprintf(fout, "Num Vertices: %i\n", lpMesh->MeshHeader.NumVertices);
	fprintf(fout, "Num Triangles: %i\n", lpMesh->MeshHeader.NumTriangles);
	fprintf(fout, "Triangle Offset: %i\n", lpMesh->MeshHeader.TriangleOffset);
	fprintf(fout, "Shader offset: %i\n", lpMesh->MeshHeader.ShaderOffset);
	fprintf(fout, "Tex Coord Offset: %i\n", lpMesh->MeshHeader.TexCoordOffset);
	fprintf(fout, "Vertex Offset: %i\n", lpMesh->MeshHeader.VertexOffset);
	fprintf(fout, "Surface End: %i\n", lpMesh->MeshHeader.MeshDataSize);
	fprintf(fout, "\n");

	if( (dwDumpFlags&MD3DUMP_MESHSHADER)==MD3DUMP_MESHSHADER){
		fprintf(fout, "- Shader Data -\n\n");
		for(md3_int32 i=0; i<lpMesh->MeshHeader.NumShaders; i++){
			fprintf(fout, "Shader %i\n", i+1);
			fprintf(fout, "Shader #: %i\n", lpMesh->Shaders[i].ShaderNum);
			fprintf(fout, "Shader name: %s\n", lpMesh->Shaders[i].ShaderName);
		}
	}
	if( (dwDumpFlags&MD3DUMP_MESHTRI)==MD3DUMP_MESHTRI){
		fprintf(fout, "\n\n- Triangle Data -\n\n");
		for(md3_int32 i=0; i<lpMesh->MeshHeader.NumTriangles; i++){
			fprintf(fout, "Triangle %i\n", i+1);
			fprintf(fout, "Indexes: (%i, %i, %i)\n", lpMesh->Triangles[i].Indexes[0], lpMesh->Triangles[i].Indexes[1], lpMesh->Triangles[i].Indexes[2]);			fprintf(fout, "\n");
		}
	}
	if( (dwDumpFlags&MD3DUMP_MESHTEXCOORD)==MD3DUMP_MESHTEXCOORD){
		fprintf(fout, "\n\n- Texture Coordinates -\n\n");
		for(md3_int32 i=0; i<lpMesh->MeshHeader.NumVertices; i++){
			fprintf(fout, "TexCoord %i\n", i+1);
			fprintf(fout, "Coordinates: (%f, %f)\n", lpMesh->TexCoords[i].tu, lpMesh->TexCoords[i].tv);
			fprintf(fout, "\n");
		}
	}
	if( (dwDumpFlags&MD3DUMP_MESHVERTEX)==MD3DUMP_MESHVERTEX){
		fprintf(fout, "\n\n- Vertices -\n\n");
		for(md3_int32 i=0; i<lpMesh->MeshHeader.NumVertices*lpMesh->MeshHeader.NumFrames; i++){
			fprintf(fout, "Vertex %i: ", i+1);
			fprintf(fout, "(%i, %i, %i, %i)\n", lpMesh->Vertexes[i].x, lpMesh->Vertexes[i].y,lpMesh->Vertexes[i].z, lpMesh->Vertexes[i].Normal);
		}
	}
}

void DumpMD3DebugData(FILE* fout, const CMD3File& lpFile, md3_uint32 dwDumpFlags)
{
	/* Write the header data. */
	fprintf(fout, "--- MD3 Header ---\n\n");
	fprintf(fout, "MD3 ID: %i\n", lpFile.Header.ID);
	fprintf(fout, "MD3 Version: %i\n", lpFile.Header.Version);
	fprintf(fout, "MD3 Filename: %s\n", lpFile.Header.Filename);
	fprintf(fout, "Flags: %i\n", lpFile.Header.Flags);
	fprintf(fout, "Number of frames: %i\n", lpFile.Header.NumFrames);
	fprintf(fout, "Number of tags: %i\n", lpFile.Header.NumTags);
	fprintf(fout, "Number of meshes: %i\n",lpFile.Header.NumMeshes);
	fprintf(fout, "Number of skins: %i\n", lpFile.Header.NumSkins);
	fprintf(fout, "Frame Offset: %i\n", lpFile.Header.FrameOffset);
	fprintf(fout, "Tag Offset: %i\n", lpFile.Header.TagOffset);
	fprintf(fout, "Mesh Offset: %i\n", lpFile.Header.MeshOffset);
	fprintf(fout, "MD3 File Size: %i bytes\n", lpFile.Header.FileSize);
	fprintf(fout, "\n");

	/* Write the frame data. */
	if( (dwDumpFlags&MD3DUMP_BONEFRAME)==MD3DUMP_BONEFRAME){
		fprintf(fout, "\n\n\n--- Frame Data ---\n\n");
		for(md3_int32 i=0; i<lpFile.Header.NumFrames; i++){
			fprintf(fout, "- Frame %i -\n", (i+1));
			fprintf(fout, "Min Bounds: (%f, %f, %f)\n", lpFile.Frames[i].Min.x, lpFile.Frames[i].Min.y, lpFile.Frames[i].Min.z);
			fprintf(fout, "Max Bounds: (%f, %f, %f)\n", lpFile.Frames[i].Max.x, lpFile.Frames[i].Max.y, lpFile.Frames[i].Max.z);
			fprintf(fout, "Local Origin: (%f, %f, %f)\n", lpFile.Frames[i].Origin.x, lpFile.Frames[i].Origin.y, lpFile.Frames[i].Origin.z);
			fprintf(fout, "Boudning Radius: %f\n", lpFile.Frames[i].Radius);
			fprintf(fout, "Name: %s\n", lpFile.Frames[i].Name);
			fprintf(fout, "\n");
		}
	}
	/* Write the tag data. */
	if( (dwDumpFlags&MD3DUMP_TAG)==MD3DUMP_TAG){
		fprintf(fout, "\n\n\n--- Tag Data ---\n\n");
		for(md3_int32 i=0; i<lpFile.Header.NumTags*lpFile.Header.NumFrames; i++){
			fprintf(fout, "- Tag %i -\n", (i+1));
			fprintf(fout, "Name: %s\n", lpFile.Tags[i].Name);
			fprintf(fout, "Origin: (%f, %f, %f)\n", lpFile.Tags[i].Position.x, lpFile.Tags[i].Position.y, lpFile.Tags[i].Position.z);
			fprintf(fout, "1st Axis: (%f, %f, %f)\n", lpFile.Tags[i].Axis[0].x, lpFile.Tags[i].Axis[0].y, lpFile.Tags[i].Axis[0].z);
			fprintf(fout, "2nd Axis: (%f, %f, %f)\n", lpFile.Tags[i].Axis[1].x, lpFile.Tags[i].Axis[1].y, lpFile.Tags[i].Axis[1].z);
			fprintf(fout, "3rd Axis: (%f, %f, %f)\n", lpFile.Tags[i].Axis[2].x, lpFile.Tags[i].Axis[2].y, lpFile.Tags[i].Axis[2].z);
			fprintf(fout, "\n");
		}
	}
	/* Write the Mesh data. */
	if( (dwDumpFlags&MD3DUMP_MESH)==MD3DUMP_MESH){
		for(md3_int32 i=0; i<lpFile.Header.NumMeshes; i++){
			fprintf(fout, "\n=== Mesh %i ===\n", i+1);
			DumpMD3MeshDebugData(fout, &(lpFile.Meshes[i]), dwDumpFlags);
		}
	}
}
