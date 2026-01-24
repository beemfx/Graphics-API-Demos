// (c) Beem Media. All rights reserved.

#include "D3D_MD3PlayerObject.h"
#include "D3D_MD3PlayerMesh.h"

CD3D_MD3PlayerObject::CD3D_MD3PlayerObject()
{
	m_lpPlayerMesh=NULL;
	m_dwSkinRef = MD3_DEFAULT_INDEX;
	m_dwUpperAnim=0;
	m_dwLowerAnim=0;

	m_fFPSUpper=1.0f;
	m_fFPSLower=1.0f;

	m_UpperTransition=TRANSITION_NONE;
	m_LowerTransition=TRANSITION_NONE;

	m_dwAnimNextUpper=0;
	m_dwAnimNextLower=0;

	m_dwAnimPrevLower=0;
	m_dwAnimPrevUpper=0;

	m_fAnimSpeedNextUpper=0.0f;
	m_fAnimSpeedNextLower=0.0f;

	m_lCurrentFrameLower=0;
	m_lCurrentFrameUpper=0;

	m_lLastSecondFrameUpper=0;
	m_lLastSecondFrameLower=0;

	m_dwTransitionCycle=100; //This can be changed for slower or faster transition (should create function).
	
	m_lpWeapon=NULL;

	m_dwLastCycleTimeLower=timeGetTime();
	m_dwLastCycleTimeUpper=timeGetTime();

	m_AnimationLower = { };
	m_AnimationUpper = { };
}

CD3D_MD3PlayerObject::~CD3D_MD3PlayerObject()
{
	
}

HRESULT CD3D_MD3PlayerObject::SetWeapon(CD3D_MD3WeaponMesh * lpWeapon)
{
	m_lpWeapon=lpWeapon;
	return S_OK;
}

HRESULT CD3D_MD3PlayerObject::GetAnimation(DWORD * lpUpper, DWORD * lpLower)
{
	if(lpUpper)
		*lpUpper=m_dwUpperAnim;
	if(lpLower)
		*lpLower=m_dwLowerAnim;

	return S_OK;
};

HRESULT CD3D_MD3PlayerObject::SetSkinByName(const md3_char8* SkinName)
{
	DWORD dwRef=0;
	if(!m_lpPlayerMesh)
		return E_FAIL;

	m_dwSkinRef = m_lpPlayerMesh->GetSkinRef(SkinName);
	return S_OK;
}

HRESULT CD3D_MD3PlayerObject::SetSkinByRef(DWORD dwSkinRef)
{
	m_dwSkinRef=dwSkinRef;

	return S_OK;
}

HRESULT CD3D_MD3PlayerObject::ApplyAnimation(DWORD dwAnimRef, FLOAT fSpeed, DWORD dwFlags)
{
	if(!m_lpPlayerMesh)
		return E_FAIL;

	if( (dwAnimRef >=BOTH_DEATH1) && (dwAnimRef <= BOTH_DEAD3)){
		if((dwFlags&MD3APPLYANIM_UPPER)==MD3APPLYANIM_UPPER)
		{
			m_dwAnimPrevUpper=m_dwUpperAnim;
			m_dwUpperAnim=dwAnimRef;
			m_lpPlayerMesh->GetAnimation(dwAnimRef, &m_AnimationUpper);
			m_dwLastCycleTimeUpper=timeGetTime();
			m_fFPSUpper=fSpeed;
		}
		else if((dwFlags&MD3APPLYANIM_LOWER)==MD3APPLYANIM_LOWER)
		{
			m_dwAnimPrevLower=m_dwLowerAnim;
			m_dwLowerAnim=dwAnimRef;
			m_lpPlayerMesh->GetAnimation(dwAnimRef, &m_AnimationLower);
			m_dwLastCycleTimeLower=timeGetTime();
			m_fFPSLower=fSpeed;
		}
		else
		{
			m_dwAnimPrevUpper=m_dwUpperAnim;
			m_dwAnimPrevLower=m_dwLowerAnim;
			m_dwUpperAnim=dwAnimRef;
			m_dwLowerAnim=dwAnimRef;
			m_lpPlayerMesh->GetAnimation(dwAnimRef, &m_AnimationUpper);
			m_lpPlayerMesh->GetAnimation(dwAnimRef, &m_AnimationLower);
			m_dwLastCycleTimeUpper=m_dwLastCycleTimeLower=timeGetTime();
			m_fFPSUpper=m_fFPSLower=fSpeed;
		}
	}else if( (dwAnimRef >= TORSO_GESTURE) && (dwAnimRef <= TORSO_STAND2)){
		m_dwAnimPrevUpper=m_dwUpperAnim;
		m_dwUpperAnim=dwAnimRef;
		m_lpPlayerMesh->GetAnimation(dwAnimRef, &m_AnimationUpper);
		m_dwLastCycleTimeUpper=timeGetTime();
		m_fFPSUpper=fSpeed;
	}else if( (dwAnimRef >= LEGS_WALKCR) && (dwAnimRef <= LEGS_TURN)){
		m_dwAnimPrevLower=m_dwLowerAnim;
		m_dwLowerAnim=dwAnimRef;
		m_dwLastCycleTimeLower=timeGetTime();
		m_lpPlayerMesh->GetAnimation(dwAnimRef, &m_AnimationLower);
		m_fFPSLower=fSpeed;
	}else{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CD3D_MD3PlayerObject::SetAnimation(DWORD dwAnimRef, DWORD dwFlags, FLOAT fSpeed)
{
	BOOL bLower=FALSE;
	BOOL bUpper=FALSE;
	BOOL bAlreadyDeath=FALSE;

	if( (dwAnimRef >=BOTH_DEATH1) && (dwAnimRef <= BOTH_DEAD3)){
		bLower=bUpper=TRUE;
		m_dwAnimNextUpper=m_dwAnimNextLower=dwAnimRef;
		m_fAnimSpeedNextUpper=m_fAnimSpeedNextLower=fSpeed;
		if( !((m_dwLowerAnim >=BOTH_DEATH1) && (m_dwLowerAnim <= BOTH_DEAD3)) ){
			//If we weren't in a death animation we do a frame transition to a death animation.
			//Otherwise we fall through and do the chosen transition-type.
			m_LowerTransition=m_UpperTransition=TRANSITION_FRAMETRANSITION;
			m_lPrevFrameLower=m_lLastSecondFrameLower;
			m_lPrevFrameUpper=m_lLastSecondFrameUpper;
			ApplyAnimation(dwAnimRef, fSpeed, 0);
			return S_OK;
		}
	}else if( (dwAnimRef >= TORSO_GESTURE) && (dwAnimRef <= TORSO_STAND2)){
		m_fAnimSpeedNextUpper=fSpeed;
		m_dwAnimNextUpper=dwAnimRef;
		bUpper=TRUE;
	}else if( (dwAnimRef >= LEGS_WALKCR) && (dwAnimRef <= LEGS_TURN)){
		//ApplyAnimation(dwAnimRef, fSpeed);
		m_fAnimSpeedNextLower=fSpeed;
		m_dwAnimNextLower=dwAnimRef;
		bLower=TRUE;
	}else{
		return E_FAIL;
	}


	if(MD3SETANIM_WAIT==(dwFlags&MD3SETANIM_WAIT)){
		if(bLower)
			m_LowerTransition=TRANSITION_WAITFORANIMATION;

		if(bUpper)
			m_UpperTransition=TRANSITION_WAITFORANIMATION;

	}else if(MD3SETANIM_FRAME==(dwFlags&MD3SETANIM_FRAME)){
		if(bLower){
			m_LowerTransition=TRANSITION_WAITFORKEYFRAME;
			m_lPrevFrameLower=m_lLastSecondFrameLower;
		}

		if(bUpper){
			m_UpperTransition=TRANSITION_WAITFORKEYFRAME;
			m_lPrevFrameUpper=m_lLastSecondFrameUpper;
		}
	}else{
		//If no transition was specified we do a closest match interpolation.
		if(bLower){
			m_LowerTransition=TRANSITION_FRAMETRANSITION;
			m_lPrevFrameLower=m_lLastSecondFrameLower;
			ApplyAnimation(dwAnimRef, fSpeed, 0);
		}

		if(bUpper){
			m_UpperTransition=TRANSITION_FRAMETRANSITION;
			m_lPrevFrameUpper=m_lLastSecondFrameUpper;
			ApplyAnimation(dwAnimRef, fSpeed, 0);
		}
	}
	return S_OK;
}

HRESULT CD3D_MD3PlayerObject::SetPlayerMesh(CD3D_MD3PlayerMesh * lpPlayerMesh)
{
	//Apply the animation.
	m_lpPlayerMesh=lpPlayerMesh;

	//Apply default animations.
	ApplyAnimation(LEGS_IDLE, 1.0f, 0);
	ApplyAnimation(TORSO_STAND, 1.0f, 0);

	SetSkinByRef(MD3_DEFAULT_INDEX);

	return S_OK;
}

HRESULT CD3D_MD3PlayerObject::GetFrames(
	LONG * lpFirstFrame,
	LONG * lpSecondFrame,
	FLOAT * lpTime,
	DWORD dwTimeElapsed,
	DWORD dwFrameTime,
	const md3AnimationConfig& Animation)
{
	//We get the first frame by dividing the
	//elapsed time by the frames per second.
	//And then add the first frame.
	*lpFirstFrame=dwTimeElapsed/dwFrameTime + Animation.lFirstFrame;
	//We make sure the frame is within bounds
	if( (*lpFirstFrame) >= (Animation.lFirstFrame + Animation.lNumFrames)){
		(*lpFirstFrame)=Animation.lFirstFrame;
		//(*dwLastCycle)=timeGetTime();
	}
	

	//Get the time in between frames.
	(*lpTime)=(float)( (float)dwTimeElapsed/(float)dwFrameTime );
	(*lpTime)-=(LONG)(*lpTime);

	//Get the second frame.
	(*lpSecondFrame)=(*lpFirstFrame)+1;

	//Make sure the second frame is within bounds.
	if( (*lpSecondFrame)>=(Animation.lFirstFrame + Animation.lNumFrames)){
		if(Animation.lLoopingFrames > 0)
			(*lpSecondFrame)=(*lpFirstFrame)-Animation.lLoopingFrames+1;
		else
			(*lpSecondFrame)=Animation.lFirstFrame;//(*lpFirstFrame)-Animation.lLoopingFrames;
	}

	return S_OK;
}

__inline HRESULT CD3D_MD3PlayerObject::FrameTransitionAdjust(
	FRAMETRANSITIONTYPE * lpTransition,
	md3AnimationConfig * lpAnimation,
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
	DWORD dwFlags)
{

	switch(*lpTransition)
	{
	case TRANSITION_NONE:
		break;
	case TRANSITION_WAITFORANIMATION:
		if(
			(*lpFirstFrame == (lpAnimation->lFirstFrame+lpAnimation->lNumFrames-2)) ||
			(lpAnimation->lNumFrames==1) )
		{
			(*lpPrevFrame)=(*lpSecondFrame);
			*lpTransition=TRANSITION_WAITFORKEYFRAME;
		}
		break;
	case TRANSITION_WAITFORKEYFRAME:
		if((*lpFirstFrame) != (*lpPrevFrame))
			break;
		//Otherwise fall through.
	case TRANSITION_FRAMETRANSITION:
		if(*lpTransition==TRANSITION_WAITFORKEYFRAME)
		{
			*lpTransition=TRANSITION_FRAMETRANSITION;
			*dwLastCycleTime=timeGetTime();

			ApplyAnimation(dwSecondAnim, fSpeed, dwFlags);

			dwCycleTime=0;	
		}

		GetFrames(
			lpFirstFrame,
			lpSecondFrame,
			lpTime,
			dwCycleTime,
			m_dwTransitionCycle,
			*lpAnimation);

		*lpSecondFrame=*lpFirstFrame;
		*lpFirstFrame=*lpPrevFrame;

		if(*lpSecondFrame > lpAnimation->lFirstFrame)
		{
			*lpSecondFrame=lpAnimation->lFirstFrame;
			*lpTime=1.0f;
			*lpTransition=TRANSITION_NONE;
			*dwLastCycleTime=timeGetTime();
		}
		else if(lpAnimation->lNumFrames <= 1)
		{
			if(bDone)
			{
				*lpTime=1.0f;
				*lpTransition=TRANSITION_NONE;
				*dwLastCycleTime=timeGetTime();
			}
		}
		break;
	}
	return S_OK;
}

HRESULT CD3D_MD3PlayerObject::Render( const D3DMATRIX& SavedWorldMatrix )
{
	//We need a speed adjust to display animation at correct speed.
	if(!this->m_lpPlayerMesh)
		return 0;
	LONG lFirstFrameLower=0;
	LONG lSecondFrameLower=0;
	FLOAT fTimeLower=0.0f;

	LONG lFirstFrameUpper=0;
	LONG lSecondFrameUpper=0;
	FLOAT fTimeUpper=0.0f;
	
	DWORD dwCycleTime=0;
	DWORD dwAnimCycle=0;

	BOOL bFrameDone=FALSE;

	/////////////////////////////
	/// Get the lower frames. ///
	/////////////////////////////
	bFrameDone=FALSE;
	//Get the cycle time for the lower animation.
	dwCycleTime=timeGetTime()-m_dwLastCycleTimeLower;
	//Get the animation cycle for the lower frames.
	dwAnimCycle=(DWORD)(( (FLOAT)( (m_AnimationLower.lNumFrames*1000)/m_AnimationLower.lFramesPerSecond) )*m_fFPSLower);

	//Insure that the time elapsed is within the animation cycle.
	//If it isn't reset the animation time.
	if(m_LowerTransition==TRANSITION_FRAMETRANSITION)
		dwAnimCycle=m_dwTransitionCycle*m_AnimationLower.lNumFrames;
	

	if( dwCycleTime >= dwAnimCycle ){
		m_dwLastCycleTimeLower=timeGetTime();
		dwCycleTime-=dwAnimCycle;
		if(m_LowerTransition==TRANSITION_FRAMETRANSITION)
			bFrameDone=TRUE;	
	}
	
	//Get the frames and time according to the animation cycle.
	GetFrames(
		&lFirstFrameLower, 
		&lSecondFrameLower, 
		&fTimeLower,
		dwCycleTime,
		dwAnimCycle/m_AnimationLower.lNumFrames, 
		m_AnimationLower);

	FrameTransitionAdjust(
		&m_LowerTransition,
		&m_AnimationLower,
		&lFirstFrameLower,
		&lSecondFrameLower,
		&fTimeLower,
		&m_lPrevFrameLower,
		&m_dwLastCycleTimeLower,
		dwCycleTime,
		m_dwLowerAnim,
		m_dwAnimNextLower,
		m_fAnimSpeedNextLower,
		bFrameDone,
		MD3APPLYANIM_LOWER);

	/////////////////////////////
	/// Get the Upper frames. ///
	/////////////////////////////
	bFrameDone=FALSE;
	//Get the cycle time for the upper animation.
	dwCycleTime=timeGetTime()-m_dwLastCycleTimeUpper;

	//Get the animation cycle for the upper frames.
	dwAnimCycle=(DWORD)((FLOAT)(((m_AnimationUpper.lNumFrames)*1000)/m_AnimationUpper.lFramesPerSecond)*m_fFPSUpper);

	//Insure that the time elapsed is within the animation cycle.
	if(m_UpperTransition==TRANSITION_FRAMETRANSITION)
		dwAnimCycle=m_dwTransitionCycle*m_AnimationUpper.lNumFrames;

	if(dwCycleTime >= dwAnimCycle){
		m_dwLastCycleTimeUpper=timeGetTime();
		dwCycleTime-=dwAnimCycle;
		if(m_UpperTransition==TRANSITION_FRAMETRANSITION)
			bFrameDone=TRUE;
	}

	GetFrames(
		&lFirstFrameUpper,
		&lSecondFrameUpper,
		&fTimeUpper,
		dwCycleTime,
		dwAnimCycle/m_AnimationUpper.lNumFrames,
		m_AnimationUpper);

	FrameTransitionAdjust(
		&m_UpperTransition,
		&m_AnimationUpper,
		&lFirstFrameUpper,
		&lSecondFrameUpper,
		&fTimeUpper,
		&m_lPrevFrameUpper,
		&m_dwLastCycleTimeUpper,
		dwCycleTime,
		m_dwUpperAnim,
		m_dwAnimNextUpper,
		m_fAnimSpeedNextUpper,
		bFrameDone,
		MD3APPLYANIM_UPPER);

	//Save the last second frame (for transition purposes).
	m_lLastSecondFrameUpper=lSecondFrameUpper;
	m_lLastSecondFrameLower=lSecondFrameLower;


	//Render the player mesh.
	if(m_lpPlayerMesh)
		m_lpPlayerMesh->Render(
			lFirstFrameUpper,
			lSecondFrameUpper,
			fTimeUpper,
			lFirstFrameLower,
			lSecondFrameLower,
			fTimeLower,
			m_dwSkinRef,
			m_lpWeapon,
			SavedWorldMatrix);


	return S_OK;
}