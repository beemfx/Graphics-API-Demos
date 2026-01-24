// (c) Beem Media. All rights reserved.

#pragma once

#include "MD3File.h"
#include <stdio.h>

/*
	Generic MD3 Functions.
*/
/* Dumps copy of MD3 data to a file. */
static const md3_uint32 MD3DUMP_BONEFRAME = (1 << 0);
static const md3_uint32 MD3DUMP_TAG = (1 << 1);
static const md3_uint32 MD3DUMP_MESH = (1 << 2);
static const md3_uint32 MD3DUMP_MESHSHADER = (1 << 3);
static const md3_uint32 MD3DUMP_MESHTRI = (1 << 4);
static const md3_uint32 MD3DUMP_MESHTEXCOORD = (1 << 5);
static const md3_uint32 MD3DUMP_MESHVERTEX = (1 << 6);
static const md3_uint32 MD3DUMP_ALL = 0xFFFFFFFF;

void DumpMD3DebugData(FILE* fout, const md3File* lpFile, md3_uint32 dwDumpFlags);