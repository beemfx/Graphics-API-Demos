#include <stdio.h>
#include "MD3File.h"

BOOL DumpMD3MeshDebugData(FILE * fout, LPMD3MESH lpMesh, DWORD dwDumpFlags);

BOOL DumpMD3MeshDebugData(FILE * fout, LPMD3MESH lpMesh, DWORD dwDumpFlags){
	LONG i=0;

	if(!fout)return FALSE;
	fprintf(fout, "\n\n--- MD3 Mesh Header ---\n\n");
	fprintf(fout, "ID: %i\n", lpMesh->md3MeshHeader.dwID);
	fprintf(fout, "Name: %s\n", lpMesh->md3MeshHeader.szMeshName);
	fprintf(fout, "Flags: %i\n", lpMesh->md3MeshHeader.lFlags);
	fprintf(fout, "Num Frames: %i\n", lpMesh->md3MeshHeader.lNumFrames);
	fprintf(fout, "Num Shaders: %i\n", lpMesh->md3MeshHeader.lNumShaders);
	fprintf(fout, "Num Vertices: %i\n", lpMesh->md3MeshHeader.lNumVertices);
	fprintf(fout, "Num Triangles: %i\n", lpMesh->md3MeshHeader.lNumTriangles);
	fprintf(fout, "Triangle Offset: %i\n", lpMesh->md3MeshHeader.lTriangleOffset);
	fprintf(fout, "Shader offset: %i\n", lpMesh->md3MeshHeader.lShaderOffset);
	fprintf(fout, "Tex Coord Offset: %i\n", lpMesh->md3MeshHeader.lTexCoordOffset);
	fprintf(fout, "Vertex Offset: %i\n", lpMesh->md3MeshHeader.lVertexOffset);
	fprintf(fout, "Surface End: %i\n", lpMesh->md3MeshHeader.lMeshDataSize);
	fprintf(fout, "\n");

	if( (dwDumpFlags&MD3DUMP_MESHSHADER)==MD3DUMP_MESHSHADER){
		fprintf(fout, "- Shader Data -\n\n");
		for(i=0; i<lpMesh->md3MeshHeader.lNumShaders; i++){
			fprintf(fout, "Shader %i\n", i+1);
			fprintf(fout, "Shader #: %i\n", lpMesh->md3Shader[i].lShaderNum);
			fprintf(fout, "Shader name: %s\n", lpMesh->md3Shader[i].szShaderName);
		}
	}
	if( (dwDumpFlags&MD3DUMP_MESHTRI)==MD3DUMP_MESHTRI){
		fprintf(fout, "\n\n- Triangle Data -\n\n");
		for(i=0; i<lpMesh->md3MeshHeader.lNumTriangles; i++){
			fprintf(fout, "Triangle %i\n", i+1);
			fprintf(fout, "Indexes: (%i, %i, %i)\n", lpMesh->md3Triangle[i].nIndexes[0], lpMesh->md3Triangle[i].nIndexes[1], lpMesh->md3Triangle[i].nIndexes[2]);			fprintf(fout, "\n");
		}
	}
	if( (dwDumpFlags&MD3DUMP_MESHTEXCOORD)==MD3DUMP_MESHTEXCOORD){
		fprintf(fout, "\n\n- Texture Coordinates -\n\n");
		for(i=0; i<lpMesh->md3MeshHeader.lNumVertices; i++){
			fprintf(fout, "TexCoord %i\n", i+1);
			fprintf(fout, "Coordinates: (%f, %f)\n", lpMesh->md3TexCoords[i].tu, lpMesh->md3TexCoords[i].tv);
			fprintf(fout, "\n");
		}
	}
	if( (dwDumpFlags&MD3DUMP_MESHVERTEX)==MD3DUMP_MESHVERTEX){
		fprintf(fout, "\n\n- Vertices -\n\n");
		for(i=0; i<lpMesh->md3MeshHeader.lNumVertices*lpMesh->md3MeshHeader.lNumFrames; i++){
			fprintf(fout, "Vertex %i: ", i+1);
			fprintf(fout, "(%i, %i, %i, %i)\n", lpMesh->md3Vertex[i].x, lpMesh->md3Vertex[i].y,lpMesh->md3Vertex[i].z, lpMesh->md3Vertex[i].nNormal);
		}
	}
	return TRUE;
}

BOOL DumpMD3DebugData(LPVOID fout, LPMD3FILE lpFile, DWORD dwDumpFlags){
	LONG i=0;
	/* Write the header data. */
	fprintf(fout, "--- MD3 Header ---\n\n");
	fprintf(fout, "MD3 ID: %i\n", lpFile->md3Header.dwID);
	fprintf(fout, "MD3 Version: %i\n", lpFile->md3Header.lVersion);
	fprintf(fout, "MD3 Filename: %s\n", lpFile->md3Header.szFileName);
	fprintf(fout, "Flags: %i\n", lpFile->md3Header.lFlags);
	fprintf(fout, "Number of frames: %i\n", lpFile->md3Header.lNumFrames);
	fprintf(fout, "Number of tags: %i\n", lpFile->md3Header.lNumTags);
	fprintf(fout, "Number of meshes: %i\n",lpFile->md3Header.lNumMeshes);
	fprintf(fout, "Number of skins: %i\n", lpFile->md3Header.lNumSkins);
	fprintf(fout, "Frame Offset: %i\n", lpFile->md3Header.lFrameOffset);
	fprintf(fout, "Tag Offset: %i\n", lpFile->md3Header.lTagOffset);
	fprintf(fout, "Mesh Offset: %i\n", lpFile->md3Header.lMeshOffset);
	fprintf(fout, "MD3 File Size: %i bytes\n", lpFile->md3Header.lFileSize);
	fprintf(fout, "\n");

	/* Write the frame data. */
	if( (dwDumpFlags&MD3DUMP_BONEFRAME)==MD3DUMP_BONEFRAME){
		fprintf(fout, "\n\n\n--- Frame Data ---\n\n");
		for(i=0; i<lpFile->md3Header.lNumFrames; i++){
			fprintf(fout, "- Frame %i -\n", (i+1));
			fprintf(fout, "Min Bounds: (%f, %f, %f)\n", lpFile->md3Frame[i].vMin.x, lpFile->md3Frame[i].vMin.y, lpFile->md3Frame[i].vMin.z);
			fprintf(fout, "Max Bounds: (%f, %f, %f)\n", lpFile->md3Frame[i].vMax.x, lpFile->md3Frame[i].vMax.y, lpFile->md3Frame[i].vMax.z);
			fprintf(fout, "Local Origin: (%f, %f, %f)\n", lpFile->md3Frame[i].vOrigin.x, lpFile->md3Frame[i].vOrigin.y, lpFile->md3Frame[i].vOrigin.z);
			fprintf(fout, "Boudning Radius: %f\n", lpFile->md3Frame[i].fRadius);
			fprintf(fout, "Name: %s\n", lpFile->md3Frame[i].szName);
			fprintf(fout, "\n");
		}
	}
	/* Write the tag data. */
	if( (dwDumpFlags&MD3DUMP_TAG)==MD3DUMP_TAG){
		fprintf(fout, "\n\n\n--- Tag Data ---\n\n");
		for(i=0; i<lpFile->md3Header.lNumTags*lpFile->md3Header.lNumFrames; i++){
			fprintf(fout, "- Tag %i -\n", (i+1));
			fprintf(fout, "Name: %s\n", lpFile->md3Tag[i].szName);
			fprintf(fout, "Origin: (%f, %f, %f)\n", lpFile->md3Tag[i].vPosition.x, lpFile->md3Tag[i].vPosition.y, lpFile->md3Tag[i].vPosition.z);
			fprintf(fout, "1st Axis: (%f, %f, %f)\n", lpFile->md3Tag[i].vAxis[0].x, lpFile->md3Tag[i].vAxis[0].y, lpFile->md3Tag[i].vAxis[0].z);
			fprintf(fout, "2nd Axis: (%f, %f, %f)\n", lpFile->md3Tag[i].vAxis[1].x, lpFile->md3Tag[i].vAxis[1].y, lpFile->md3Tag[i].vAxis[1].z);
			fprintf(fout, "3rd Axis: (%f, %f, %f)\n", lpFile->md3Tag[i].vAxis[2].x, lpFile->md3Tag[i].vAxis[2].y, lpFile->md3Tag[i].vAxis[2].z);
			fprintf(fout, "\n");
		}
	}
	/* Write the Mesh data. */
	if( (dwDumpFlags&MD3DUMP_MESH)==MD3DUMP_MESH){
		for(i=0; i<lpFile->md3Header.lNumMeshes; i++){
			fprintf(fout, "\n=== Mesh %i ===\n", i+1);
			DumpMD3MeshDebugData(fout, &(lpFile->md3Mesh[i]), dwDumpFlags);
		}
	}
	return TRUE;
}