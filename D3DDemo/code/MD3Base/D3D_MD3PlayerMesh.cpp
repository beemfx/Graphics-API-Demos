// (c) Beem Media. All rights reserved.

#include "D3D_MD3PlayerMesh.h"
#include "D3D_MD3WeaponMesh.h"
#include "D3D_MD3Skin.h"
#include "defines.h"
#include <d3dx9.h>

CD3D_MD3PlayerMesh::CD3D_MD3PlayerMesh()
{
	m_nLowerUpperTag = 0;
	m_nUpperHeadTag = 0;
	m_nUpperWeaponTag = 0;

	m_lpDevice = NULL;

	m_bLoaded = FALSE;
}

CD3D_MD3PlayerMesh::~CD3D_MD3PlayerMesh()
{
	Clear();
}

md3_uint32 CD3D_MD3PlayerMesh::GetSkinRef(const md3_char8* SkinName) const
{
	DWORD i = 0;

	for (md3_uint32 i = 0; i < m_SkinSets.size(); i++)
	{
		if (m_SkinSets[i].Name == SkinName)
		{
			return i + 1;
		}
	}

	return MD3_DEFAULT_INDEX;
}

md3AnimationConfig CD3D_MD3PlayerMesh::GetAnimation(md3_uint32 AnimRef) const
{
	return m_Animation.GetAnimation(AnimRef);
}

HRESULT CD3D_MD3PlayerMesh::GetLink(CD3D_MD3Mesh* lpFirst, const char szTagName[], WORD* lpTagRef)
{
	LONG i = 0;
	LONG lNumTags = 0;
	char szTemp[MAX_QPATH];

	lpFirst->GetNumTags(&lNumTags);

	for (i = 1; i <= lNumTags; i++) {
		lpFirst->GetTagName(i, szTemp);
		if (_strnicmp(szTemp, szTagName, strlen(szTagName)) == 0) {
			*lpTagRef = (WORD)i;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT CD3D_MD3PlayerMesh::Invalidate()
{
	if (m_bLoaded) {
		m_meshHead.Invalidate();
		m_meshUpper.Invalidate();
		m_meshLower.Invalidate();
	}
	return S_OK;
}

HRESULT CD3D_MD3PlayerMesh::Validate()
{
	if (m_bLoaded) {
		m_meshHead.Validate();
		m_meshUpper.Validate();
		m_meshLower.Validate();
	}
	return S_OK;
}

HRESULT CD3D_MD3PlayerMesh::Render(
	LONG lUpperFirstFrame,
	LONG lUpperSecondFrame,
	FLOAT fUpperTime,
	LONG lLowerFirstFrame,
	LONG lLowerSecondFrame,
	FLOAT fLowerTime,
	DWORD dwSkinRef,
	CD3D_MD3WeaponMesh* lpWeapon,
	const D3DMATRIX& SavedWorldMatrix)
{
	if (!m_bLoaded)
		return 0;

	D3DXMATRIX WorldMatrix, Translation, Temp;

	DWORD dwRenderFlags = 0;//MD3RENDER_WIREFRAME;

	//In order to render in the propper location it is necessary
	//to save the current World Matrix.  Then create a rotation
	//matrix to get the mesh properly oriented, then apply the
	//world matrix that was previously saved.

	if (!m_bLoaded)
		return S_FALSE;


	if (dwSkinRef == MD3_DEFAULT_INDEX)
		dwSkinRef = m_DefaultSkin;

	if ((dwSkinRef < 1) || (dwSkinRef > m_SkinSets.size()))
		return E_FAIL;

	if (fUpperTime > 1.0f)
		fUpperTime -= (LONG)fUpperTime;

	if (fLowerTime > 1.0f)
		fLowerTime -= (LONG)fLowerTime;


	D3DXMatrixIdentity(&WorldMatrix);

	//Orient the model in the right direction.
	D3DXMatrixRotationX(&Translation, 1.5f * D3DX_PI);
	WorldMatrix *= Translation;
	D3DXMatrixRotationY(&Translation, 0.5f * D3DX_PI);
	WorldMatrix *= Translation;
	WorldMatrix *= SavedWorldMatrix;
	m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);


	//Render the lower mesh.
	m_meshLower.Render(
		&m_SkinSets[dwSkinRef - 1].Lower,
		fLowerTime,
		lLowerFirstFrame,
		lLowerSecondFrame,
		dwRenderFlags);

	//Get and set the translation for the lower mesh.
	m_meshLower.GetTagTranslation(
		m_nLowerUpperTag,
		fLowerTime,
		lLowerFirstFrame,
		lLowerSecondFrame,
		&Translation);
	WorldMatrix = Translation * WorldMatrix;
	m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);
	D3DMATRIX TaggedMatrix = WorldMatrix;

	//Render the upper mesh.
	m_meshUpper.Render(
		&m_SkinSets[dwSkinRef - 1].Upper,
		fUpperTime,
		lUpperFirstFrame,
		lUpperSecondFrame,
		dwRenderFlags);

	//Render a weapon if one is set.
	if (lpWeapon)
	{
		Temp = TaggedMatrix;

		m_meshUpper.GetTagTranslation(
			m_nUpperWeaponTag,
			fUpperTime,
			lUpperFirstFrame,
			lUpperSecondFrame,
			&Translation);

		WorldMatrix = Translation * WorldMatrix;

		//We must derotate because the object will properly orient itself.
		D3DXMatrixRotationX(&Translation, 0.5f * D3DX_PI);
		WorldMatrix = Translation * WorldMatrix;
		D3DXMatrixRotationY(&Translation, 1.5f * D3DX_PI);
		WorldMatrix = Translation * WorldMatrix;

		m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);

		//Find out if the weapon should perform the flash.
		LONG lShoot = 0, lGauntlet = 0;
		md3AnimationConfig TorsoAttackAnim = m_Animation.GetAnimation(TORSO_ATTACK);
		lShoot = TorsoAttackAnim.lFirstFrame;
		md3AnimationConfig TorsoMeleeAnim = m_Animation.GetAnimation(TORSO_ATTACK2);
		lGauntlet = TorsoMeleeAnim.lFirstFrame;

		if (lUpperSecondFrame == (lShoot + 1) || lUpperSecondFrame == (lGauntlet + 2) || lUpperSecondFrame == (lGauntlet + 3))
			lpWeapon->Render(TRUE, WorldMatrix);
		else
			lpWeapon->Render(FALSE, WorldMatrix);

		WorldMatrix = Temp;
	}

	//Get and set translation for head mesh.
	m_meshUpper.GetTagTranslation(
		m_nUpperHeadTag,
		fUpperTime,
		lUpperFirstFrame,
		lUpperSecondFrame,
		&Translation);
	WorldMatrix = Translation * WorldMatrix;
	m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);

	//Render the head mesh (time, and frame must be set to zero).
	m_meshHead.Render(
		&m_SkinSets[dwSkinRef - 1].Head,
		0.0f,
		0,
		0,
		dwRenderFlags);


	//Restore the original world matrix.
	m_lpDevice->SetTransform(D3DTS_WORLD, &SavedWorldMatrix);
	return S_OK;
}

bool CD3D_MD3PlayerMesh::GetSkins(const std::filesystem::path& Dir)
{
	std::vector<std::string> SkinFiles;

	{
		WIN32_FIND_DATAA FindData = { };

		//Basically my intent is to find out how many skins there
		//are. This can be done by finding out how many files with names
		//skin file beginning with upper_ exist.  We could use any bone
		//of the body, but upper will do nicely.
		std::filesystem::path SearchPath = Dir / "upper_*.skin";

		HANDLE hFind = FindFirstFileA(
			SearchPath.string().c_str(),
			&FindData);

		if (hFind == INVALID_HANDLE_VALUE)
			return false;

		do
		{
			SkinFiles.push_back(FindData.cFileName);
		} while (FindNextFile(hFind, &FindData));

		FindClose(hFind);
	}

	m_SkinSets.reserve(SkinFiles.size());

	std::vector<md3_char8> Temp;
	for (auto& UpperName : SkinFiles)
	{
		auto& NewSkin = m_SkinSets.emplace_back();
		// The name is everything after the first '_' and before ".skin":
		Temp.clear();
		bool bFoundUnderscore = false;
		for (md3_uint32 i = 0; i <UpperName.size(); i++)
		{
			const md3_char8 c = UpperName[i];
			if (!bFoundUnderscore)
			{
				if (c == '_')
				{
					bFoundUnderscore = true;
				}
			}
			else
			{
				if (c == '.')
				{
					break;
				}
				else
				{
					Temp.push_back(c);
				}
			}
		}
		Temp.push_back('\0');
		NewSkin.Name = Temp.data();
		if (NewSkin.Name == "default")
		{
			m_DefaultSkin = static_cast<md3_uint32>(m_SkinSets.size());
		}
	}

	//We'll now go ahead and load all the skins.
	for (md3_uint32 i = 0; i < m_SkinSets.size(); i++)
	{
		const std::string HeadFilename = "head_" + m_SkinSets[i].Name + ".skin";
		const std::string UpperFilename = "upper_" + m_SkinSets[i].Name + ".skin";
		const std::string LowerFilename = "lower_" + m_SkinSets[i].Name + ".skin";

		const std::filesystem::path HeadPath = Dir / HeadFilename;
		const std::filesystem::path UpperPath = Dir / UpperFilename;
		const std::filesystem::path LowerPath = Dir / LowerFilename;


		const bool bSkinLoaded 
			=
			m_SkinSets[i].Head.LoadSkin(m_lpDevice, HeadPath, m_TexDB)
			&&
			m_SkinSets[i].Upper.LoadSkin(m_lpDevice, UpperPath, m_TexDB)
			&&
			m_SkinSets[i].Lower.LoadSkin(m_lpDevice, LowerPath, m_TexDB);

		if (!bSkinLoaded)
		{
			m_SkinSets.resize(0);

			return false;
		}
	}


	//Set all the skin references for the model.
	for (md3_uint32 i = 0; i < m_SkinSets.size(); i++)
	{
		m_meshHead.SetSkinRefs(m_SkinSets[i].Head);
		m_meshUpper.SetSkinRefs(m_SkinSets[i].Upper);
		m_meshLower.SetSkinRefs(m_SkinSets[i].Lower);
	}

	return true;
}

bool CD3D_MD3PlayerMesh::Load(LPDIRECT3DDEVICE9 lpDevice, const std::filesystem::path& Dir, d3d_md3_detail nDetail)
{
	if (!lpDevice)
		return false;

	Clear();

	m_lpDevice = lpDevice;
	m_lpDevice->AddRef();

	std::filesystem::path HeadPath;
	std::filesystem::path UpperPath;
	std::filesystem::path LowerPath;

	switch (nDetail)
	{
	case d3d_md3_detail::High:
		HeadPath = Dir / "head.md3";
		UpperPath = Dir / "upper.md3";
		LowerPath = Dir / "lower.md3";
		break;
	case d3d_md3_detail::Medium:
		HeadPath = Dir / "head_1.md3";
		UpperPath = Dir / "upper_1.md3";
		LowerPath = Dir / "lower_1.md3";
		break;
	case d3d_md3_detail::Low:
		HeadPath = Dir / "head_2.md3";
		UpperPath = Dir / "upper_2.md3";
		LowerPath = Dir / "lower_2.md3";
		break;
	};

	const md3_bool bLoadedMeshes
	=
	m_meshHead.LoadMD3(HeadPath, lpDevice, D3DPOOL_DEFAULT)
	&&
	m_meshUpper.LoadMD3(UpperPath, lpDevice, D3DPOOL_DEFAULT)
	&&
	m_meshLower.LoadMD3(LowerPath, lpDevice, D3DPOOL_DEFAULT);

	if (!bLoadedMeshes)
	{
		m_meshHead.ClearMD3();
		m_meshUpper.ClearMD3();
		m_meshLower.ClearMD3();

		if ((nDetail == d3d_md3_detail::Medium) || (nDetail == d3d_md3_detail::Low))
		{
			//Lower details may not exist of this model so
			//attempt to load at high detail.
			SAFE_RELEASE(m_lpDevice);
			return Load(lpDevice, Dir, d3d_md3_detail::High);
		}
		SAFE_RELEASE(m_lpDevice);
		return false;
	}


	//Attempt to load the animation.
	const std::filesystem::path AnimConfigPath = Dir / "animation.cfg";

	if (!m_Animation.LoadAnimation(AnimConfigPath))
	{
		m_meshHead.ClearMD3();
		m_meshUpper.ClearMD3();
		m_meshLower.ClearMD3();
		SAFE_RELEASE(m_lpDevice);
		return false;
	}

	if (!GetSkins(Dir))
	{
		m_SkinSets.resize(0);

		m_meshHead.ClearMD3();
		m_meshUpper.ClearMD3();
		m_meshLower.ClearMD3();
		SAFE_RELEASE(m_lpDevice);
		return false;
	}

	//Get the link reference for all the md3 mesh's.
	GetLink(&m_meshLower, "tag_torso", &m_nLowerUpperTag);
	GetLink(&m_meshUpper, "tag_head", &m_nUpperHeadTag);
	GetLink(&m_meshUpper, "tag_weapon", &m_nUpperWeaponTag);

	m_bLoaded = TRUE;

	return true;
}

HRESULT CD3D_MD3PlayerMesh::Clear()
{
	DWORD i = 0;

	m_meshHead.ClearMD3();
	m_meshUpper.ClearMD3();
	m_meshLower.ClearMD3();
	m_TexDB.ClearDB();
	m_SkinSets.resize(0);
	m_DefaultSkin = 0;
	
	SAFE_RELEASE(m_lpDevice);

	m_bLoaded = FALSE;
	return S_OK;
}