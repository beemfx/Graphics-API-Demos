#define D3D_MD3
#include "functions.h"
#include "md3.h"

#define PERR_FAIL           0x80000000l
#define PERR_NOLINE         0x80000001l
#define PERR_ANIMOUTOFRANGE 0x80000002l

////////////////////////////////
// Constructor and Destructor //
////////////////////////////////

CMD3Animation::CMD3Animation()
{
	m_nHeadOffset[0]=m_nHeadOffset[1]=m_nHeadOffset[2]=0;
	m_nSex=MD3SEX_DEFAULT;
	m_nFootStep=MD3FOOTSTEP_DEFAULT;
	m_lLegOffset=0;

	ZeroMemory(m_Animations, sizeof(m_Animations));
}

CMD3Animation::~CMD3Animation()
{

}

//////////////////////
// Public Functions //
//////////////////////

HRESULT CMD3Animation::LoadAnimationA(char szFilename[])
{
	HANDLE hFile=NULL;

	hFile=CreateFileA(
		szFilename,
		GENERIC_READ,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	if(hFile==INVALID_HANDLE_VALUE)
		return E_FAIL;


	DWORD dwNumLines=GetNumLinesInFile(hFile);
	ReadAnimations(hFile, dwNumLines);

	CloseHandle(hFile);
	//Get the leg animation offset.
	if(m_Animations[TORSO_GESTURE].lFirstFrame != m_Animations[LEGS_WALKCR].lFirstFrame)
	{
		m_lLegOffset=m_Animations[LEGS_WALKCR].lFirstFrame-m_Animations[TORSO_GESTURE].lFirstFrame;
	}
	else
		m_lLegOffset=0;

	return S_OK;
}

HRESULT CMD3Animation::LoadAnimationW(WCHAR szFilename[])
{
	HANDLE hFile=NULL;

	hFile=CreateFileW(
		szFilename,
		GENERIC_READ,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	if(hFile==INVALID_HANDLE_VALUE)
		return E_FAIL;


	DWORD dwNumLines=GetNumLinesInFile(hFile);
	ReadAnimations(hFile, dwNumLines);

	CloseHandle(hFile);
	if(m_Animations[TORSO_GESTURE].lFirstFrame != m_Animations[LEGS_WALKCR].lFirstFrame)
	{
		m_lLegOffset=m_Animations[LEGS_WALKCR].lFirstFrame-m_Animations[TORSO_GESTURE].lFirstFrame;
	}
	else
		m_lLegOffset=0;
	return S_OK;
}

HRESULT CMD3Animation::GetAnimation(DWORD dwRef, MD3ANIMATION * lpAnimation, DWORD dwFlags)
{
	if(dwRef >= MD3_NUM_ANIMS)
		return E_FAIL;

	lpAnimation->lFirstFrame=m_Animations[dwRef].lFirstFrame;
	lpAnimation->lNumFrames=m_Animations[dwRef].lNumFrames;
	lpAnimation->lLoopingFrames=m_Animations[dwRef].lLoopingFrames;
	lpAnimation->lFramesPerSecond=m_Animations[dwRef].lFramesPerSecond;

	if(
		(dwRef >= LEGS_WALKCR) && 
		(dwRef <= LEGS_TURN) && 
		((dwFlags&MD3ANIM_ADJUST)==MD3ANIM_ADJUST)
		)
	{
		//The animation legs offset should be adjusted.
		lpAnimation->lFirstFrame -= m_lLegOffset;
		
	}

	return S_OK;
}

///////////////////////
// Private Functions //
///////////////////////

HRESULT CMD3Animation::ReadAnimations(HANDLE hFile, DWORD dwNumLines)
{
	DWORD i=0;
	HRESULT hr=0;
	DWORD dwAnimRef=0;

	char szLine[100];
	
	for(i=0; i<dwNumLines; i++){
		hr=ReadLine(hFile, szLine);
		if(FAILED(hr))
			return hr;

		if(hr!=RLSUC_FINISHED){
			ParseLine(NULL, szLine, &dwAnimRef);
		}
	}
	return S_OK;
}

HRESULT CMD3Animation::ParseLine(LPVOID lpDataOut, LPSTR szLineIn, DWORD * lpAnimRef)
{
	size_t dwLen=0;
	DWORD i=0, j=0;
	char szTemp[100];
	char szFinal[100];
	
	strcpy(szFinal, szLineIn);
	dwLen=strlen(szFinal);
	//The first thing to do is remove anything found after the double slash.
	for(i=0; i<dwLen; i++){
		if(i < dwLen-1)
			if(szFinal[i]=='/' && szFinal[i+1]=='/')
				szFinal[i]=0;

	}
	dwLen=strlen(szFinal);

	if(dwLen==0)
		return PERR_NOLINE;
	//Convert all tabs into spaces.
	for(i=0; i<dwLen; i++){
		if(szFinal[i]=='\t')
			szFinal[i]=' ';
	}

	//Determine the type of line by geting the first word.

	ReadWordFromLine(szTemp, szFinal, 0, &i);


	//Check if the line is an assignment or a animation.
	if(_strnicmp(szTemp, "sex", 3)==0){
		//It's sex, so read second word to get the sex.
		ReadWordFromLine(szTemp, szFinal, i, &i);
		switch(szTemp[0])
		{
		case 'm': m_nSex=MD3SEX_MALE; break;
		case 'f': m_nSex=MD3SEX_FEMALE; break;
		default: m_nSex=MD3SEX_OTHER; break;
		};
		return S_OK;
	}else if(_strnicmp(szTemp, "headoffset", 10)==0){
		//It is headoffset so we read three additional words.
		ReadWordFromLine(szTemp, szFinal, i, &i);
		m_nHeadOffset[0]=atoi(szTemp);
		ReadWordFromLine(szTemp, szFinal, i, &i);
		m_nHeadOffset[1]=atoi(szTemp);
		ReadWordFromLine(szTemp, szFinal, i, &i);
		m_nHeadOffset[2]=atoi(szTemp);

		return S_OK;
	}else if(_strnicmp(szTemp, "footsteps", 9)==0){
		//It is footsteps so we read one more word.
		ReadWordFromLine(szTemp, szFinal, i, &i);
		if(_strnicmp(szTemp, "boot", 4)==0){
			m_nFootStep=MD3FOOTSTEP_BOOT;
		}else if(_strnicmp(szTemp, "energy", 6)==0){
			m_nFootStep=MD3FOOTSTEP_ENERGY;
		}else{
			m_nFootStep=MD3FOOTSTEP_DEFAULT;
		}

		return S_OK;
	}else{
		//We assume it is an animation configuration.

		if(*lpAnimRef>=MD3_NUM_ANIMS)return PERR_ANIMOUTOFRANGE;

		//Set the first frame to the current word.
		m_Animations[*lpAnimRef].lFirstFrame=atoi(szTemp);

		//Read the additional three words to get the rest of the info.
		ReadWordFromLine(szTemp, szFinal, i, &i);
		m_Animations[*lpAnimRef].lNumFrames=atoi(szTemp);
		ReadWordFromLine(szTemp, szFinal, i, &i);
		m_Animations[*lpAnimRef].lLoopingFrames=atoi(szTemp);
		ReadWordFromLine(szTemp, szFinal, i, &i);
		m_Animations[*lpAnimRef].lFramesPerSecond=atoi(szTemp);

		(*lpAnimRef)++;

		return S_OK;
	}
}
