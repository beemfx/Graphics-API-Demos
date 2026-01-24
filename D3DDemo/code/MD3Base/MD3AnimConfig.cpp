// (c) Beem Media. All rights reserved.

#include "MD3AnimConfig.h"
#include "Library/Functions.h"
#include "FileSystem/DataStream.h"

CMD3AnimConfig::CMD3AnimConfig()
{
	
}

CMD3AnimConfig::~CMD3AnimConfig()
{
	
}

md3_bool CMD3AnimConfig::LoadAnimation(const std::filesystem::path& Filename)
{
	CDataStream AnimDataStream(Filename);
	const std::vector<std::string> AnimDataLines = Functions::ReadLines(AnimDataStream);

	ReadAnimations(AnimDataLines);

	//Get the leg animation offset.
	if (m_Animations[TORSO_GESTURE].lFirstFrame != m_Animations[LEGS_WALKCR].lFirstFrame)
	{
		m_LegOffset = m_Animations[LEGS_WALKCR].lFirstFrame - m_Animations[TORSO_GESTURE].lFirstFrame;
	}
	else
	{
		m_LegOffset = 0;
	}

	return m_NumAnims == MD3_NUM_ANIMS;
}

md3AnimationConfig CMD3AnimConfig::GetAnimation(md3_uint32 Ref, md3_uint32 Flags /*= MD3ANIM_ADJUST*/)
{
	const bool bIsValidRef = 0 <= Ref && Ref < m_NumAnims;

	if (!bIsValidRef)
	{
		return md3AnimationConfig();
	}

	md3AnimationConfig Out;
	Out.lFirstFrame = m_Animations[Ref].lFirstFrame;
	Out.lNumFrames = m_Animations[Ref].lNumFrames;
	Out.lLoopingFrames = m_Animations[Ref].lLoopingFrames;
	Out.lFramesPerSecond = m_Animations[Ref].lFramesPerSecond;

	if (
		(Ref >= LEGS_WALKCR) &&
		(Ref <= LEGS_TURN) &&
		((Flags & MD3ANIM_ADJUST) == MD3ANIM_ADJUST)
	)
	{
		//The animation legs offset should be adjusted.
		Out.lFirstFrame -= m_LegOffset;

	}

	return Out;
}

///////////////////////
// Private Functions //
///////////////////////

void CMD3AnimConfig::ReadAnimations(const std::vector<std::string>& Lines)
{
	m_NumAnims = 0;

	for (auto& Line : Lines)
	{
		ParseLine(Line);
	}
}

void CMD3AnimConfig::ParseLine(const std::string& Line)
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
			m_Sex = md3_anim_sex::Male;
		}
		else if (SecondWord == "f")
		{
			m_Sex = md3_anim_sex::Female;
		}
		else
		{
			m_Sex = md3_anim_sex::Other;
		}
	}
	else if (FirstWord == "headoffset")
	{
		// It is head offset so we read three additional words.

		for (std::size_t i = 0; i < 3; i++)
		{
			m_HeadOffset[i] = std::atoi(Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart).c_str());
		}
	}
	else if (FirstWord == "footsteps")
	{
		//It is footsteps so we read one more word.
		const std::string SecondWord = Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart);

		if (SecondWord == "boot")
		{
			m_Footstep = md3_anim_footstep::Boot;
		}
		else if (SecondWord == "energy")
		{
			m_Footstep = md3_anim_footstep::Energy;
		}
		else
		{
			m_Footstep = md3_anim_footstep::Default;
		}

	}
	else if (FirstWord.size() > 0)
	{
		//We assume it is an animation configuration.
		assert(m_NumAnims < MD3_NUM_ANIMS);
		if (m_NumAnims < MD3_NUM_ANIMS)
		{
			//Set the first frame to the current word.
			m_Animations[m_NumAnims].lFirstFrame = atoi(FirstWord.c_str());

			//Read the additional three words to get the rest of the info.
			m_Animations[m_NumAnims].lNumFrames = std::atoi(Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart).c_str());
			m_Animations[m_NumAnims].lLoopingFrames = std::atoi(Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart).c_str());
			m_Animations[m_NumAnims].lFramesPerSecond = std::atoi(Functions::ReadWordFromLine(Final, NextWordStart, &NextWordStart).c_str());

			m_NumAnims++;
		}
	}
}
