﻿#pragma once

enum race_flags8 : uint32_t {
    RF8_WILD_ONLY = 0x00000001,
    RF8_WILD_TOWN = 0x00000002,
    RF8_NO_QUEST = 0x00000004,
    RF8_WILD_SHORE = 0x00000008,
    RF8_WILD_OCEAN = 0x00000010,
    RF8_WILD_WASTE = 0x00000020,
    RF8_WILD_WOOD = 0x00000040,
    RF8_WILD_VOLCANO = 0x00000080,
    RF8_XXX8X08 = 0x00000100,
    RF8_WILD_MOUNTAIN = 0x00000200,
    RF8_WILD_GRASS = 0x00000400,
    RF8_WILD_SWAMP = 0x00000800, //!< 沼地に生息(未使用)
    RF8_WILD_ALL = 0x80000000,
};
