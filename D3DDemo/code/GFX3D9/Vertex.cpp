/*
	Vertext.cpp - The Custom Vertext Format

	Copyright (c) 2003, Blaine Myers
*/

#include "GFX3D9.h"

tagCUSTOMVERTEX::tagCUSTOMVERTEX()
{
	Position.x=0;
	Position.y=0;
	Position.z=0;

	Normal.x=0;
	Normal.y=0;
	Normal.z=0;

	DiffuseColor=0;

	SpecularColor=0;

	tu=0;
	tv=0;
}

tagCUSTOMVERTEX::tagCUSTOMVERTEX(
	float px, 
	float py, 
	float pz, 
	float nx, 
	float ny, 
	float nz, 
	D3DCOLOR dwDiffuse, 
	D3DCOLOR dwSpecular, 
	float txu, 
	float txv)
{
	Position.x=px;
	Position.y=py;
	Position.z=pz;

	Normal.x=nx;
	Normal.y=ny;
	Normal.z=nz;

	DiffuseColor=dwDiffuse;

	SpecularColor=dwSpecular;

	tu=txu;
	tv=txv;
}

tagCUSTOMVERTEX tagCUSTOMVERTEX::operator = (const tagCUSTOMVERTEX & rhs)
{
	if(this == &rhs)
		return *this;
	
	Position.x=rhs.Position.x;
	Position.y=rhs.Position.y;
	Position.z=rhs.Position.z;

	Normal.x=rhs.Normal.x;
	Normal.y=rhs.Normal.y;
	Normal.z=rhs.Normal.z;

	DiffuseColor=rhs.DiffuseColor;

	SpecularColor=rhs.SpecularColor;

	tu=rhs.tu;
	tv=rhs.tv;

	return *this;
}

int tagCUSTOMVERTEX::operator == (const tagCUSTOMVERTEX & rhs)
{
	if(this == &rhs)
		return 1;

	if (
		(Position.x==rhs.Position.x) &&
		(Position.y==rhs.Position.y) &&
		(Position.z==rhs.Position.z) &&

		(Normal.x==rhs.Normal.x) &&
		(Normal.y==rhs.Normal.y) &&
		(Normal.z==rhs.Normal.z) &&

		(DiffuseColor==rhs.DiffuseColor) &&

		(SpecularColor==rhs.SpecularColor) &&

		(tu==rhs.tu) &&
		(tv==rhs.tv)
		)return 1;
	else return 0;
}

int tagCUSTOMVERTEX::operator != (const tagCUSTOMVERTEX & rhs)
{
	if(*this == rhs)return 0;
	else return 1;
}