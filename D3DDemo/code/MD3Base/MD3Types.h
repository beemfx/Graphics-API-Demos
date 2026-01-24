// (c) Beem Media. All rights reserved.

#pragma once

#include <cstdint>

using md3_bool = bool;

using md3_byte = std::uint8_t;

using md3_uint8 = std::uint8_t;
using md3_int8 = std::int8_t;
using md3_uint16 = std::uint16_t;
using md3_int16 = std::int16_t;
using md3_uint32 = std::uint32_t;
using md3_int32 = std::int32_t;
using md3_uint64 = std::uint64_t;
using md3_int64 = std::int64_t;

using md3_real32 = float;
using md3_real64 = double;

static_assert(sizeof(md3_byte) == 1, "Type is the wrong size.");
static_assert(sizeof(md3_uint8) == 1, "Type is the wrong size.");
static_assert(sizeof(md3_int8) == 1, "Type is the wrong size.");
static_assert(sizeof(md3_uint16) == 2, "Type is the wrong size.");
static_assert(sizeof(md3_int16) == 2, "Type is the wrong size.");
static_assert(sizeof(md3_uint32) == 4, "Type is the wrong size.");
static_assert(sizeof(md3_int32) == 4, "Type is the wrong size.");
static_assert(sizeof(md3_uint64) == 8, "Type is the wrong size.");
static_assert(sizeof(md3_int64) == 8, "Type is the wrong size.");
static_assert(sizeof(md3_real32) == 4, "Type is the wrong size.");
static_assert(sizeof(md3_real64) == 8, "Type is the wrong size.");
