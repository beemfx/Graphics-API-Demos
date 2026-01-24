// (c) Beem Media. All rights reserved.

#pragma once

#include "D3D_MD3Mesh.h"
#include "MD3AnimConfig.h"
#include "GFX3D9/GFX3D9TextureDB.h"

class CD3D_MD3Skin;
class CMD3WeaponMesh;

class CD3D_MD3PlayerMesh
{
protected:
	CD3D_MD3Mesh m_meshHead;
	CD3D_MD3Skin* m_skinHead;

	CD3D_MD3Mesh m_meshUpper;
	CD3D_MD3Skin* m_skinUpper;

	CD3D_MD3Mesh m_meshLower;
	CD3D_MD3Skin* m_skinLower;

	CMD3AnimConfig m_Animation;

	CGFX3D9TextureDB m_TexDB;

	WORD m_nLowerUpperTag;
	WORD m_nUpperHeadTag;
	WORD m_nUpperWeaponTag;

	DWORD m_dwNumSkins;
	DWORD m_dwDefaultSkin;
	char** m_szSkinName;

	LPDIRECT3DDEVICE9 m_lpDevice;

	BOOL m_bLoaded;

	HRESULT GetLink(CD3D_MD3Mesh* lpFirst, const char szTagName[], WORD* lpTagRef);

	HRESULT GetSkinsA(char szDir[]);
	HRESULT GetSkinsW(WCHAR szDir[]);
public:
	CD3D_MD3PlayerMesh();
	~CD3D_MD3PlayerMesh();

	HRESULT GetAnimation(DWORD dwAnimRef, md3AnimationConfig* lpAnimation);

	HRESULT GetSkinRef(DWORD* lpRef, char szSkinName[]);


	HRESULT Render(
		LONG lUpperFirstFrame,
		LONG lUpperSecondFrame,
		FLOAT fUpperTime,
		LONG lLowerFirstFrame,
		LONG lLowerSecondFrame,
		FLOAT fLowerTime,
		DWORD dwSkinRef,
		CMD3WeaponMesh* lpWeapon,
		const D3DMATRIX& SavedWorldMatrix);

	HRESULT LoadA(LPDIRECT3DDEVICE9 lpDevice, char szDir[], d3d_md3_detail nDetail);
	HRESULT LoadW(LPDIRECT3DDEVICE9 lpDevice, WCHAR szDir[], d3d_md3_detail nDetail);

	HRESULT Clear();

	HRESULT Invalidate();
	HRESULT Validate();
};