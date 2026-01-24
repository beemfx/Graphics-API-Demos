// (c) Beem Media. All rights reserved.

#pragma once

#include "D3D_MD3Mesh.h"
#include "D3D_MD3Skin.h"
#include "MD3AnimConfig.h"
#include "GFX3D9/GFX3D9TextureDB.h"

class CD3D_MD3Skin;
class CD3D_MD3WeaponMesh;

class CD3D_MD3PlayerMesh
{
public:
	CD3D_MD3PlayerMesh();
	~CD3D_MD3PlayerMesh();

	md3AnimationConfig GetAnimation(md3_uint32 AnimRef) const;
	md3_uint32 GetSkinRef(const md3_char8* SkinName) const;

	HRESULT Render(
		LONG lUpperFirstFrame,
		LONG lUpperSecondFrame,
		FLOAT fUpperTime,
		LONG lLowerFirstFrame,
		LONG lLowerSecondFrame,
		FLOAT fLowerTime,
		DWORD dwSkinRef,
		CD3D_MD3WeaponMesh* lpWeapon,
		const D3DMATRIX& SavedWorldMatrix);

	bool Load(LPDIRECT3DDEVICE9 lpDevice, const std::filesystem::path& Dir, d3d_md3_detail nDetail);

	HRESULT Clear();

	HRESULT Invalidate();
	HRESULT Validate();

private:
	struct CSkinSet
	{
		std::string Name;
		CD3D_MD3Skin Head;
		CD3D_MD3Skin Upper;
		CD3D_MD3Skin Lower;
	};

protected:
	CD3D_MD3Mesh m_meshHead;
	CD3D_MD3Mesh m_meshUpper;
	CD3D_MD3Mesh m_meshLower;

	std::vector<CSkinSet> m_SkinSets;
	md3_uint32 m_DefaultSkin = 0;

	CMD3AnimConfig m_Animation;

	CGFX3D9TextureDB m_TexDB;

	WORD m_nLowerUpperTag;
	WORD m_nUpperHeadTag;
	WORD m_nUpperWeaponTag;

	LPDIRECT3DDEVICE9 m_lpDevice;

	BOOL m_bLoaded;

	HRESULT GetLink(CD3D_MD3Mesh* lpFirst, const char szTagName[], WORD* lpTagRef);

	bool GetSkins(const std::filesystem::path& Dir);
};