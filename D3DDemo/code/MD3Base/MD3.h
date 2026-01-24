/*
	MD3.h - Header for MD3 File Format

	Copyright (c) 2003 Blaine Myers
	Portions Copyright (c) 1999 ID Software
*/

#pragma once

#include "MD3File.h"
#include "MD3AnimConfig.h"
#include "MD3SkinConfig.h"
#include "D3D_MD3Mesh.h"
#include "GFX3D9/GFX3D9TextureDB.h"

#include <d3d9.h>

class CD3D_MD3Skin;
class CMD3WeaponMesh;
class CD3D_MD3PlayerMesh;


/*
	The player object manages animations.
*/

/* The flags for the SetAnimation function. */
#define MD3SETANIM_WAIT      0x00000010l /* Wait for the current animation to finish. */
#define MD3SETANIM_FRAME     0x00000020l /* Wait for the next key-frame. */
#define MD3SETANIM_FORCELOOP 0x00000040l

typedef enum tagFRAMETRANSITIONTYPE{
	TRANSITION_NONE=0x00000000l,
	TRANSITION_WAITFORANIMATION,
	TRANSITION_WAITFORKEYFRAME,
	TRANSITION_FRAMETRANSITION,
}FRAMETRANSITIONTYPE;


#define MD3APPLYANIM_UPPER 0x00000001l
#define MD3APPLYANIM_LOWER 0x00000002l

class CMD3PlayerObject
{
protected:
	CD3D_MD3PlayerMesh * m_lpPlayerMesh; /* Pointer to the player mesh. */
	DWORD m_dwLastCycleTimeLower; /* The last time the lower cycle was completed. */
	DWORD m_dwLastCycleTimeUpper; /* The last time the upper cycle was completed. */

	FLOAT m_fFPSUpper; /* Speed adjust for the upper animation. */
	FLOAT m_fFPSLower; /* Speed adjust for the lower animation. */

	DWORD m_dwSkinRef; /* The current skin reference. */

	DWORD m_dwUpperAnim; /* The reference for the torso animation. */
	md3AnimationConfig m_AnimationUpper; /* The data for the torso animation. */
	DWORD m_dwLowerAnim; /* The reference for the legs animation. */
	md3AnimationConfig m_AnimationLower; /* The data for the legs animation. */

	CMD3WeaponMesh * m_lpWeapon;

	FRAMETRANSITIONTYPE m_LowerTransition;
	FRAMETRANSITIONTYPE m_UpperTransition;

	LONG m_lPrevFrameUpper;
	LONG m_lPrevFrameLower;

	DWORD m_dwAnimNextLower;
	DWORD m_dwAnimNextUpper;

	DWORD m_dwAnimPrevLower;
	DWORD m_dwAnimPrevUpper;

	FLOAT m_fAnimSpeedNextLower;
	FLOAT m_fAnimSpeedNextUpper;

	LONG m_lCurrentFrameLower;
	LONG m_lCurrentFrameUpper;

	LONG m_lLastSecondFrameLower;
	LONG m_lLastSecondFrameUpper;

	DWORD m_dwTransitionCycle;

	__inline HRESULT FrameTransitionAdjust(
		FRAMETRANSITIONTYPE * lpTransition,
		md3AnimationConfig* lpAnimation,
		LONG * lpFirstFrame,
		LONG * lpSecondFrame,
		FLOAT * lpTime,
		LONG * lpPrevFrame,
		DWORD * dwLastCycleTime,
		DWORD dwCycleTime,
		DWORD dwFirstAnim,
		DWORD dwSecondAnim,
		FLOAT fSpeed,
		BOOL bDone,
		DWORD dwFlags);


	HRESULT GetFrames(
		LONG * lpFirstFrame, 
		LONG * lpSecondFrame,
		FLOAT * lpTime,
		DWORD dwTimeElapsed,
		DWORD dwFrameTime,
		const md3AnimationConfig& Animation); /* Gets the correct frame and time data for animation. */

	HRESULT ApplyAnimation(
		DWORD dwAnimRef,
		FLOAT fSpeed,
		DWORD dwFlags);
public:
	CMD3PlayerObject(); /* Constructor. */
	~CMD3PlayerObject(); /* Destructor. */

	HRESULT SetAnimation(DWORD dwAnimRef, DWORD dwFlags, FLOAT fSpeed); /* Sets the current animation. */
	HRESULT GetAnimation(DWORD * lpUpper, DWORD * lpLower); /* Gets the animation. */

	HRESULT SetPlayerMesh(CD3D_MD3PlayerMesh * lpPlayerMesh); /* Sets the current player mesh. */

	/* Should remove weapon from render and have a setweapon option. */
	HRESULT Render( const D3DMATRIX& WorldMatrix ); /* Renders the mesh with appropriate animation. */

	HRESULT SetSkinByRef(DWORD dwSkinRef); /* Sets the skin. */
	HRESULT SetSkinByName(char szSkinName[MAX_QPATH]); /* Sets the skin based on it's name. */

	HRESULT SetWeapon(CMD3WeaponMesh * lpWeapon);
};

/*
	The Weapon Mesh Class.
*/
