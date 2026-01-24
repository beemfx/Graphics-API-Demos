#define D3D_MD3
#include "Functions.h"
#include "MD3.h"
#include "FileSystem/DataStream.h"

#define PERR_FAIL           0x80000000l
#define PERR_NOLINE         0x80000001l
#define PERR_ANIMOUTOFRANGE 0x80000002l

////////////////////////////////
// Constructor and Destructor //
////////////////////////////////

CMD3Animation::CMD3Animation()
{
	m_nHeadOffset[0] = m_nHeadOffset[1] = m_nHeadOffset[2] = 0;
	m_nSex = MD3SEX_DEFAULT;
	m_nFootStep = MD3FOOTSTEP_DEFAULT;
	m_lLegOffset = 0;

	ZeroMemory(m_Animations, sizeof(m_Animations));
}

CMD3Animation::~CMD3Animation()
{

}

//////////////////////
// Public Functions //
//////////////////////

HRESULT CMD3Animation::LoadAnimation(const std::filesystem::path& Filename)
{
	CDataStream AnimDataStream(Filename);
	const std::vector<std::string> AnimDataLines = Functions::ReadLines(AnimDataStream);

	ReadAnimations(AnimDataLines);

	//Get the leg animation offset.
	if (m_Animations[TORSO_GESTURE].lFirstFrame != m_Animations[LEGS_WALKCR].lFirstFrame)
	{
		m_lLegOffset = m_Animations[LEGS_WALKCR].lFirstFrame - m_Animations[TORSO_GESTURE].lFirstFrame;
	}
	else
		m_lLegOffset = 0;

	return S_OK;
}

HRESULT CMD3Animation::GetAnimation(DWORD dwRef, MD3ANIMATION* lpAnimation, DWORD dwFlags)
{
	if (dwRef >= MD3_NUM_ANIMS)
		return E_FAIL;

	lpAnimation->lFirstFrame = m_Animations[dwRef].lFirstFrame;
	lpAnimation->lNumFrames = m_Animations[dwRef].lNumFrames;
	lpAnimation->lLoopingFrames = m_Animations[dwRef].lLoopingFrames;
	lpAnimation->lFramesPerSecond = m_Animations[dwRef].lFramesPerSecond;

	if (
		(dwRef >= LEGS_WALKCR) &&
		(dwRef <= LEGS_TURN) &&
		((dwFlags & MD3ANIM_ADJUST) == MD3ANIM_ADJUST)
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

void CMD3Animation::ReadAnimations(const std::vector<std::string>& Lines)
{
	m_CurReadAnim = 0;

	for (auto& Line : Lines)
	{
		ParseLine(Line);
	}
}

void CMD3Animation::ParseLine(const std::string& Line)
{
	std::string Final = Line;

	//The first thing to do is remove anything found after the double slash.
	for (std::size_t i = 0; i < Final.size(); i++)
	{
		if (Final[i] == '/' && Final[i+1] == '/')
		{
			Final.resize(i);
			break;
		}
	}

	if (Final.size() == 0)
	{
		// Blank line or comment only.
		return;
	}

	//Convert all tabs into spaces.
	for (std::size_t i = 0; i < Final.size(); i++)
	{
		if (Final[i] == '\t')
		{
			Final[i] = ' ';
		}
	}

	//Determine the type of line by getting the first word.
	std::size_t NextWordStart = 0;
	const std::string FirstWord = Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart);

	//Check if the line is an assignment or a animation.
	if (FirstWord == "sex")
	{
		// It is sex, so read second word to get the sex.
		const std::string SecondWord = Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart);
		if (SecondWord == "m")
		{
			m_nSex = MD3SEX_MALE;
		}
		else if (SecondWord == "f")
		{
			m_nSex = MD3SEX_FEMALE;
		}
		else
		{
			m_nSex = MD3SEX_OTHER;
		}
	}
	else if (FirstWord == "headoffset")
	{
		// It is head offset so we read three additional words.

		for (std::size_t i = 0; i < 3; i++)
		{
			m_nHeadOffset[i] = std::atoi(Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart).c_str());
		}
	}
	else if (FirstWord == "footsteps")
	{
		//It is footsteps so we read one more word.
		const std::string SecondWord = Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart);

		if (SecondWord == "boot")
		{
			m_nFootStep = MD3FOOTSTEP_BOOT;
		}
		else if (SecondWord == "energy")
		{
			m_nFootStep = MD3FOOTSTEP_ENERGY;
		}
		else
		{
			m_nFootStep = MD3FOOTSTEP_DEFAULT;
		}

	}
	else if (FirstWord.size() > 0)
	{
		//We assume it is an animation configuration.
		assert( m_CurReadAnim < MD3_NUM_ANIMS);
		if (m_CurReadAnim < MD3_NUM_ANIMS)
		{
			//Set the first frame to the current word.
			m_Animations[m_CurReadAnim].lFirstFrame = atoi(FirstWord.c_str());

			//Read the additional three words to get the rest of the info.
			m_Animations[m_CurReadAnim].lNumFrames = std::atoi(Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart).c_str());
			m_Animations[m_CurReadAnim].lLoopingFrames = std::atoi(Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart).c_str());
			m_Animations[m_CurReadAnim].lFramesPerSecond = std::atoi(Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart).c_str());

			m_CurReadAnim++;
		}
	}
}
