// (c) Beem Media. All rights reserved.

#pragma once

#include "MD3Types.h"

enum class md3_anim_sex
{
	Default,
	Male,
	Female,
	Machine,
	Other,
};

enum class md3_anim_footstep
{
	Default,
	Boot,
	Energy,
	Other,
};

struct md3AnimationConfig
{
	md3_int32 lFirstFrame = 0;
	md3_int32 lNumFrames = 0;
	md3_int32 lLoopingFrames = 0;
	md3_int32 lFramesPerSecond = 0;
};

// Animation types (Quake 3 Arena specific).
enum q3a_anims : md3_uint32
{
	BOTH_DEATH1   = 0,
	BOTH_DEAD1    = 1,
	BOTH_DEATH2   = 2,
	BOTH_DEAD2    = 3,
	BOTH_DEATH3   = 4,
	BOTH_DEAD3    = 5,

	TORSO_GESTURE = 6,
	TORSO_ATTACK  = 7,
	TORSO_ATTACK2 = 8,
	TORSO_DROP    = 9,
	TORSO_RAISE   = 10,
	TORSO_STAND   = 11,
	TORSO_STAND2  = 12,

	LEGS_WALKCR   = 13,
	LEGS_WALK     = 14,
	LEGS_RUN      = 15,
	LEGS_BACK     = 16,
	LEGS_SWIM     = 17,
	LEGS_JUMP     = 18,
	LEGS_LAND     = 19,
	LEGS_JUMPB    = 20,
	LEGS_LANDB    = 21,
	LEGS_IDLE     = 22,
	LEGS_IDLECR   = 23,
	LEGS_TURN     = 24,
};

// The number of MD3 animations (Quake 3 Arena specific).
static constexpr std::size_t MD3_NUM_ANIMS = 25;

static constexpr md3_uint32 MD3ANIM_ADJUST = (1 << 0);

class CMD3AnimConfig
{
public:
	CMD3AnimConfig();
	~CMD3AnimConfig();

	md3_bool LoadAnimation(const std::filesystem::path& Filename);
	md3AnimationConfig GetAnimation(md3_uint32 Ref, md3_uint32 Flags = MD3ANIM_ADJUST) const;

private:
	md3_anim_sex m_Sex = md3_anim_sex::Default;
	md3_anim_footstep m_Footstep = md3_anim_footstep::Default;
	md3_int16 m_HeadOffset[3] = { };
	md3_int32 m_LegOffset = 0;
	md3AnimationConfig m_Animations[MD3_NUM_ANIMS] = { };
	md3_uint32 m_NumAnims = 0;

private:

	void ReadAnimations(const std::vector<std::string>& Lines);
	void ParseLine(const std::string& Line);
};
