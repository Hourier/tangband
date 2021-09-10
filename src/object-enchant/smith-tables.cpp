﻿#include "object-enchant/smith-tables.h"
#include "artifact/random-art-effects.h"
#include "object-enchant/object-smith.h"
#include "object-enchant/smith-info.h"
#include "object-enchant/smith-types.h"
#include "object-enchant/tr-flags.h"
#include "object-enchant/tr-types.h"

#include <memory>
#include <unordered_map>
#include <vector>

/*!
 * @brief エッセンスの順序リスト。エッセンスの表示順等で使用する。
 */
const std::vector<SmithEssence> Smith::essence_list_order = {
    SmithEssence::STR,
    SmithEssence::INT,
    SmithEssence::WIS,
    SmithEssence::DEX,
    SmithEssence::CON,
    SmithEssence::CHR,

    SmithEssence::SUST_STATUS,

    SmithEssence::MAGIC_MASTERY,
    SmithEssence::STEALTH,
    SmithEssence::SEARCH,
    SmithEssence::INFRA,
    SmithEssence::TUNNEL,
    SmithEssence::SPEED,
    SmithEssence::BLOWS,

    SmithEssence::CHAOTIC,
    SmithEssence::VAMPIRIC,
    SmithEssence::EATHQUAKE,
    SmithEssence::BRAND_POIS,
    SmithEssence::BRAND_ACID,
    SmithEssence::BRAND_ELEC,
    SmithEssence::BRAND_FIRE,
    SmithEssence::BRAND_COLD,

    SmithEssence::IMMUNITY,
    SmithEssence::REFLECT,

    SmithEssence::RES_ACID,
    SmithEssence::RES_ELEC,
    SmithEssence::RES_FIRE,
    SmithEssence::RES_COLD,
    SmithEssence::RES_POIS,
    SmithEssence::RES_FEAR,
    SmithEssence::RES_LITE,
    SmithEssence::RES_DARK,
    SmithEssence::RES_BLIND,
    SmithEssence::RES_CONF,
    SmithEssence::RES_SOUND,
    SmithEssence::RES_SHARDS,
    SmithEssence::RES_NETHER,
    SmithEssence::RES_NEXUS,
    SmithEssence::RES_CHAOS,
    SmithEssence::RES_DISEN,

    SmithEssence::HOLD_EXP,
    SmithEssence::FREE_ACT,
    SmithEssence::WARNING,
    SmithEssence::LEVITATION,
    SmithEssence::SEE_INVIS,
    SmithEssence::SLOW_DIGEST,
    SmithEssence::REGEN,
    SmithEssence::TELEPORT,
    SmithEssence::NO_MAGIC,
    SmithEssence::LITE,
    SmithEssence::TELEPATHY,

    SmithEssence::SLAY_EVIL,
    SmithEssence::SLAY_ANIMAL,
    SmithEssence::SLAY_UNDEAD,
    SmithEssence::SLAY_DEMON,
    SmithEssence::SLAY_ORC,
    SmithEssence::SLAY_TROLL,
    SmithEssence::SLAY_GIANT,
    SmithEssence::SLAY_DRAGON,
    SmithEssence::SLAY_HUMAN,

    SmithEssence::ATTACK,
    SmithEssence::AC,
};

/*!
 * @brief SmithEssence からエッセンスの表記名を引く連想配列
 */
const std::unordered_map<SmithEssence, concptr> Smith::essence_to_name = {
    { SmithEssence::STR, _("腕力", "strength") },
    { SmithEssence::INT, _("知能", "intelligen.") },
    { SmithEssence::WIS, _("賢さ", "wisdom") },
    { SmithEssence::DEX, _("器用さ", "dexterity") },
    { SmithEssence::CON, _("耐久力", "constitut.") },
    { SmithEssence::CHR, _("魅力", "charisma") },

    { SmithEssence::SUST_STATUS, _("能力維持", "sustain") },

    { SmithEssence::MAGIC_MASTERY, _("魔力支配", "magic mast.") },
    { SmithEssence::STEALTH, _("隠密", "stealth") },
    { SmithEssence::SEARCH, _("探索", "searching") },
    { SmithEssence::INFRA, _("赤外線視力", "infravision") },
    { SmithEssence::TUNNEL, _("採掘", "digging") },
    { SmithEssence::SPEED, _("スピード", "speed") },
    { SmithEssence::BLOWS, _("追加攻撃", "extra atk") },

    { SmithEssence::CHAOTIC, _("カオス攻撃", "chaos brand") },
    { SmithEssence::VAMPIRIC, _("吸血攻撃", "vampiric") },
    { SmithEssence::EATHQUAKE, _("地震", "quake") },
    { SmithEssence::BRAND_POIS, _("毒殺", "pois. brand") },
    { SmithEssence::BRAND_ACID, _("溶解", "acid brand") },
    { SmithEssence::BRAND_ELEC, _("電撃", "elec. brand") },
    { SmithEssence::BRAND_FIRE, _("焼棄", "fire brand") },
    { SmithEssence::BRAND_COLD, _("凍結", "cold brand") },

    { SmithEssence::IMMUNITY, _("免疫", "immunity") },
    { SmithEssence::REFLECT, _("反射", "reflection") },

    { SmithEssence::RES_ACID, _("耐酸", "res. acid") },
    { SmithEssence::RES_ELEC, _("耐電撃", "res. elec.") },
    { SmithEssence::RES_FIRE, _("耐火炎", "res. fire") },
    { SmithEssence::RES_COLD, _("耐冷気", "res. cold") },
    { SmithEssence::RES_POIS, _("耐毒", "res. poison") },
    { SmithEssence::RES_FEAR, _("耐恐怖", "res. fear") },
    { SmithEssence::RES_LITE, _("耐閃光", "res. light") },
    { SmithEssence::RES_DARK, _("耐暗黒", "res. dark") },
    { SmithEssence::RES_BLIND, _("耐盲目", "res. blind") },
    { SmithEssence::RES_CONF, _("耐混乱", "res.confuse") },
    { SmithEssence::RES_SOUND, _("耐轟音", "res. sound") },
    { SmithEssence::RES_SHARDS, _("耐破片", "res. shard") },
    { SmithEssence::RES_NETHER, _("耐地獄", "res. nether") },
    { SmithEssence::RES_NEXUS, _("耐因果混乱", "res. nexus") },
    { SmithEssence::RES_CHAOS, _("耐カオス", "res. chaos") },
    { SmithEssence::RES_DISEN, _("耐劣化", "res. disen.") },

    { SmithEssence::HOLD_EXP, _("経験値維持", "hold exp") },
    { SmithEssence::FREE_ACT, _("麻痺知らず", "free action") },
    { SmithEssence::WARNING, _("警告", "warning") },
    { SmithEssence::LEVITATION, _("浮遊", "levitation") },
    { SmithEssence::SEE_INVIS, _("可視透明", "see invis.") },
    { SmithEssence::TELEPATHY, _("テレパシー", "telepathy") },
    { SmithEssence::SLOW_DIGEST, _("遅消化", "slow dige.") },
    { SmithEssence::REGEN, _("急速回復", "regen.") },
    { SmithEssence::TELEPORT, _("テレポート", "teleport") },
    { SmithEssence::NO_MAGIC, _("反魔法", "anti magic") },
    { SmithEssence::LITE, _("永久光源", "perm. light") },

    { SmithEssence::SLAY_EVIL, _("邪悪倍打", "slay animal") },
    { SmithEssence::SLAY_ANIMAL, _("動物倍打", "slay evil") },
    { SmithEssence::SLAY_UNDEAD, _("不死倍打", "slay undead") },
    { SmithEssence::SLAY_DEMON, _("悪魔倍打", "slay demon") },
    { SmithEssence::SLAY_ORC, _("オーク倍打", "slay orc") },
    { SmithEssence::SLAY_TROLL, _("トロル倍打", "slay troll") },
    { SmithEssence::SLAY_GIANT, _("巨人倍打", "slay giant") },
    { SmithEssence::SLAY_DRAGON, _("竜倍打", "slay dragon") },
    { SmithEssence::SLAY_HUMAN, _("人間倍打", "slay human") },

    { SmithEssence::ATTACK, _("攻撃", "weapon enc.") },
    { SmithEssence::AC, _("防御", "armor enc.") },
};

/*!
 * @brief エッセンス抽出情報テーブル
 */
const std::vector<essence_drain_type> Smith::essence_drain_info_table = {
    { TR_STR, { SmithEssence::STR }, 10 },
    { TR_INT, { SmithEssence::INT }, 10 },
    { TR_WIS, { SmithEssence::WIS }, 10 },
    { TR_DEX, { SmithEssence::DEX }, 10 },
    { TR_CON, { SmithEssence::CON }, 10 },
    { TR_CHR, { SmithEssence::CHR }, 10 },
    { TR_MAGIC_MASTERY, { SmithEssence::MAGIC_MASTERY }, 10 },
    { TR_FORCE_WEAPON, { SmithEssence::INT, SmithEssence::WIS }, 5 },
    { TR_STEALTH, { SmithEssence::STEALTH }, 10 },
    { TR_SEARCH, { SmithEssence::SEARCH }, 10 },
    { TR_INFRA, { SmithEssence::INFRA }, 10 },
    { TR_TUNNEL, { SmithEssence::TUNNEL }, 10 },
    { TR_SPEED, { SmithEssence::SPEED }, 10 },
    { TR_BLOWS, { SmithEssence::BLOWS }, 10 },
    { TR_CHAOTIC, { SmithEssence::CHAOTIC }, 10 },
    { TR_VAMPIRIC, { SmithEssence::VAMPIRIC }, 10 },
    { TR_SLAY_ANIMAL, { SmithEssence::SLAY_ANIMAL }, 10 },
    { TR_SLAY_EVIL, { SmithEssence::SLAY_EVIL }, 10 },
    { TR_SLAY_UNDEAD, { SmithEssence::SLAY_UNDEAD }, 10 },
    { TR_SLAY_DEMON, { SmithEssence::SLAY_DEMON }, 10 },
    { TR_SLAY_ORC, { SmithEssence::SLAY_ORC }, 10 },
    { TR_SLAY_TROLL, { SmithEssence::SLAY_TROLL }, 10 },
    { TR_SLAY_GIANT, { SmithEssence::SLAY_GIANT }, 10 },
    { TR_SLAY_DRAGON, { SmithEssence::SLAY_DRAGON }, 10 },
    { TR_KILL_DRAGON, { SmithEssence::SLAY_DRAGON }, 10 },
    { TR_VORPAL, { SmithEssence::BRAND_POIS, SmithEssence::BRAND_ACID, SmithEssence::BRAND_ELEC, SmithEssence::BRAND_FIRE, SmithEssence::BRAND_COLD }, 5 },
    { TR_EARTHQUAKE, { SmithEssence::EATHQUAKE }, 10 },
    { TR_BRAND_POIS, { SmithEssence::BRAND_POIS }, 10 },
    { TR_BRAND_ACID, { SmithEssence::BRAND_ACID }, 10 },
    { TR_BRAND_ELEC, { SmithEssence::BRAND_ELEC }, 10 },
    { TR_BRAND_FIRE, { SmithEssence::BRAND_FIRE }, 10 },
    { TR_BRAND_COLD, { SmithEssence::BRAND_COLD }, 10 },

    { TR_SUST_STR, { SmithEssence::SUST_STATUS }, 10 },
    { TR_SUST_INT, { SmithEssence::SUST_STATUS }, 10 },
    { TR_SUST_WIS, { SmithEssence::SUST_STATUS }, 10 },
    { TR_SUST_DEX, { SmithEssence::SUST_STATUS }, 10 },
    { TR_SUST_CON, { SmithEssence::SUST_STATUS }, 10 },
    { TR_SUST_CHR, { SmithEssence::SUST_STATUS }, 10 },
    { TR_RIDING, {}, 0 },
    { TR_EASY_SPELL, {}, 0 },
    { TR_IM_ACID, { SmithEssence::IMMUNITY }, 10 },
    { TR_IM_ELEC, { SmithEssence::IMMUNITY }, 10 },
    { TR_IM_FIRE, { SmithEssence::IMMUNITY }, 10 },
    { TR_IM_COLD, { SmithEssence::IMMUNITY }, 10 },
    { TR_THROW, {}, 0 },
    { TR_REFLECT, { SmithEssence::REFLECT }, 10 },
    { TR_FREE_ACT, { SmithEssence::FREE_ACT }, 10 },
    { TR_HOLD_EXP, { SmithEssence::HOLD_EXP }, 10 },
    { TR_RES_ACID, { SmithEssence::RES_ACID }, 10 },
    { TR_RES_ELEC, { SmithEssence::RES_ELEC }, 10 },
    { TR_RES_FIRE, { SmithEssence::RES_FIRE }, 10 },
    { TR_RES_COLD, { SmithEssence::RES_COLD }, 10 },
    { TR_RES_POIS, { SmithEssence::RES_POIS }, 10 },
    { TR_RES_FEAR, { SmithEssence::RES_FEAR }, 10 },
    { TR_RES_LITE, { SmithEssence::RES_LITE }, 10 },
    { TR_RES_DARK, { SmithEssence::RES_DARK }, 10 },
    { TR_RES_BLIND, { SmithEssence::RES_BLIND }, 10 },
    { TR_RES_CONF, { SmithEssence::RES_CONF }, 10 },
    { TR_RES_SOUND, { SmithEssence::RES_SOUND }, 10 },
    { TR_RES_SHARDS, { SmithEssence::RES_SHARDS }, 10 },
    { TR_RES_NETHER, { SmithEssence::RES_NETHER }, 10 },
    { TR_RES_NEXUS, { SmithEssence::RES_NEXUS }, 10 },
    { TR_RES_CHAOS, { SmithEssence::RES_CHAOS }, 10 },
    { TR_RES_DISEN, { SmithEssence::RES_DISEN }, 10 },

    { TR_SH_FIRE, { SmithEssence::BRAND_FIRE, SmithEssence::RES_FIRE }, 10 },
    { TR_SH_ELEC, { SmithEssence::BRAND_ELEC, SmithEssence::RES_ELEC }, 10 },
    { TR_SLAY_HUMAN, { SmithEssence::SLAY_HUMAN }, 10 },
    { TR_SH_COLD, { SmithEssence::BRAND_COLD, SmithEssence::RES_COLD }, 10 },
    { TR_NO_TELE, {}, -1 },
    { TR_NO_MAGIC, { SmithEssence::NO_MAGIC }, 10 },
    { TR_DEC_MANA, { SmithEssence::INT }, 10 },
    { TR_TY_CURSE, {}, -1 },
    { TR_WARNING, { SmithEssence::WARNING }, 10 },
    { TR_HIDE_TYPE, {}, 0 },
    { TR_SHOW_MODS, {}, 0 },
    { TR_SLAY_GOOD, {}, 0 }, // todo
    { TR_LEVITATION, { SmithEssence::LEVITATION }, 10 },
    { TR_LITE_1, { SmithEssence::LITE }, 10 },
    { TR_SEE_INVIS, { SmithEssence::SEE_INVIS }, 10 },
    { TR_TELEPATHY, { SmithEssence::TELEPATHY }, 10 },
    { TR_SLOW_DIGEST, { SmithEssence::SLOW_DIGEST }, 10 },
    { TR_REGEN, { SmithEssence::REGEN }, 10 },
    { TR_XTRA_MIGHT, { SmithEssence::STR }, 10 },
    { TR_XTRA_SHOTS, { SmithEssence::DEX }, 10 },
    { TR_IGNORE_ACID, {}, 0 },
    { TR_IGNORE_ELEC, {}, 0 },
    { TR_IGNORE_FIRE, {}, 0 },
    { TR_IGNORE_COLD, {}, 0 },
    { TR_ACTIVATE, {}, 0 },
    { TR_DRAIN_EXP, {}, -1 },
    { TR_TELEPORT, { SmithEssence::TELEPORT }, 10 },
    { TR_AGGRAVATE, {}, -1 },
    { TR_BLESSED, {}, 0 },
    { TR_ES_ATTACK, {}, 0 },
    { TR_ES_AC, {}, 0 },
    { TR_KILL_GOOD, {}, 0 }, // todo

    { TR_KILL_ANIMAL, { SmithEssence::SLAY_ANIMAL }, 10 },
    { TR_KILL_EVIL, { SmithEssence::SLAY_EVIL }, 10 },
    { TR_KILL_UNDEAD, { SmithEssence::SLAY_UNDEAD }, 10 },
    { TR_KILL_DEMON, { SmithEssence::SLAY_DEMON }, 10 },
    { TR_KILL_ORC, { SmithEssence::SLAY_ORC }, 10 },
    { TR_KILL_TROLL, { SmithEssence::SLAY_TROLL }, 10 },
    { TR_KILL_GIANT, { SmithEssence::SLAY_GIANT }, 10 },
    { TR_KILL_HUMAN, { SmithEssence::SLAY_HUMAN }, 10 },
    { TR_ESP_ANIMAL, { SmithEssence::SLAY_ANIMAL }, 10 },
    { TR_ESP_UNDEAD, { SmithEssence::SLAY_UNDEAD }, 10 },
    { TR_ESP_DEMON, { SmithEssence::SLAY_DEMON }, 10 },
    { TR_ESP_ORC, { SmithEssence::SLAY_ORC }, 10 },
    { TR_ESP_TROLL, { SmithEssence::SLAY_TROLL }, 10 },
    { TR_ESP_GIANT, { SmithEssence::SLAY_GIANT }, 10 },
    { TR_ESP_DRAGON, { SmithEssence::SLAY_DRAGON }, 10 },
    { TR_ESP_HUMAN, { SmithEssence::SLAY_HUMAN }, 10 },
    { TR_ESP_EVIL, { SmithEssence::SLAY_EVIL }, 10 },
    { TR_ESP_GOOD, {}, 0 }, // TODO
    { TR_ESP_NONLIVING, {}, 0 }, // TODO
    { TR_ESP_UNIQUE, {}, 0 }, // TODO
    { TR_FULL_NAME, {}, 0 },
    { TR_FIXED_FLAVOR, {}, 0 },
    { TR_ADD_L_CURSE, {}, -1 },
    { TR_ADD_H_CURSE, {}, -1 },
    { TR_DRAIN_HP, {}, -1 },
    { TR_DRAIN_MANA, {}, -1 },
    { TR_LITE_2, { SmithEssence::LITE }, 20 },
    { TR_LITE_3, { SmithEssence::LITE }, 30 },
    { TR_LITE_M1, {}, 0 },
    { TR_LITE_M2, {}, 0 },
    { TR_LITE_M3, {}, 0 },
    { TR_LITE_FUEL, {}, 0 },

    { TR_CALL_ANIMAL, {}, -1 },
    { TR_CALL_DEMON, {}, -1 },
    { TR_CALL_DRAGON, {}, -1 },
    { TR_CALL_UNDEAD, {}, -1 },
    { TR_COWARDICE, {}, -1 },
    { TR_LOW_MELEE, {}, -1 },
    { TR_LOW_AC, {}, -1 },
    { TR_HARD_SPELL, {}, 0 },
    { TR_FAST_DIGEST, {}, -1 },
    { TR_SLOW_REGEN, {}, -1 },
    { TR_MIGHTY_THROW, {}, 0 },
    { TR_EASY2_WEAPON, { SmithEssence::DEX }, 20 },
    { TR_DOWN_SAVING, {}, 0 },
    { TR_NO_AC, {}, 0 },
    { TR_HEAVY_SPELL, {}, 0 },
    { TR_RES_TIME, {}, 0 }, // TODO
    { TR_RES_WATER, {}, 0 }, // TODO
    { TR_INVULN_ARROW, {}, 0 },
    { TR_DARK_SOURCE, {}, 0 },
    { TR_SUPPORTIVE, {}, 0 },
    { TR_RES_CURSE, {}, 0 }, // TODO
    { TR_BERS_RAGE, {}, 0 },
    { TR_BRAND_MAGIC, {}, 0 },
    { TR_IMPACT, {}, 0 },
    { TR_VUL_ACID, {}, 0 },
    { TR_VUL_COLD, {}, 0 },
    { TR_VUL_ELEC, {}, 0 },
    { TR_VUL_FIRE, {}, 0 },
    { TR_VUL_LITE, {}, 0 },
    { TR_IM_DARK, {}, 0 },
};

namespace {

template <typename T, typename... Args>
std::shared_ptr<smith_info_base> make_info(SmithEffect effect, concptr name, SmithCategory category, std::vector<SmithEssence> need_essences, int consumption, Args&&... args)
{
    return std::make_shared<T>(effect, name, category, std::move(need_essences), consumption, std::forward<Args>(args)...);
}

std::shared_ptr<smith_info_base> make_basic_smith_info(SmithEffect effect, concptr name, SmithCategory category, std::vector<SmithEssence> need_essences, int consumption, TrFlags add_flags)
{
    return make_info<basic_smith_info>(effect, name, category, std::move(need_essences), consumption, std::move(add_flags));
}

}

/*!
 * @brief 鍛冶情報テーブル
 */
const std::vector<std::shared_ptr<smith_info_base>> Smith::smith_info_table = {
    make_basic_smith_info(SmithEffect::NONE, _("なし", "None"), SmithCategory::NONE, std::vector<SmithEssence>{ SmithEssence::NONE }, 0, {}),

    make_basic_smith_info(SmithEffect::STR, _("腕力", "strength"), SmithCategory::PVAL, { SmithEssence::STR }, 20, { TR_STR }),
    make_basic_smith_info(SmithEffect::INT, _("知能", "intelligence"), SmithCategory::PVAL, { SmithEssence::INT }, 20, { TR_INT }),
    make_basic_smith_info(SmithEffect::WIS, _("賢さ", "wisdom"), SmithCategory::PVAL, { SmithEssence::WIS }, 20, { TR_WIS }),
    make_basic_smith_info(SmithEffect::DEX, _("器用さ", "dexterity"), SmithCategory::PVAL, { SmithEssence::DEX }, 20, { TR_DEX }),
    make_basic_smith_info(SmithEffect::CON, _("耐久力", "constitution"), SmithCategory::PVAL, { SmithEssence::CON }, 20, { TR_CON }),
    make_basic_smith_info(SmithEffect::CHR, _("魅力", "charisma"), SmithCategory::PVAL, { SmithEssence::CHR }, 20, { TR_CHR }),

    make_basic_smith_info(SmithEffect::SUST_STR, _("腕力維持", "sustain strength"), SmithCategory::ABILITY, { SmithEssence::SUST_STATUS }, 15, { TR_SUST_STR }),
    make_basic_smith_info(SmithEffect::SUST_INT, _("知能維持", "sustain intelligence"), SmithCategory::ABILITY, { SmithEssence::SUST_STATUS }, 15, { TR_SUST_INT }),
    make_basic_smith_info(SmithEffect::SUST_WIS, _("賢さ維持", "sustain wisdom"), SmithCategory::ABILITY, { SmithEssence::SUST_STATUS }, 15, { TR_SUST_WIS }),
    make_basic_smith_info(SmithEffect::SUST_DEX, _("器用さ維持", "sustain dexterity"), SmithCategory::ABILITY, { SmithEssence::SUST_STATUS }, 15, { TR_SUST_DEX }),
    make_basic_smith_info(SmithEffect::SUST_CON, _("耐久力維持", "sustain constitution"), SmithCategory::ABILITY, { SmithEssence::SUST_STATUS }, 15, { TR_SUST_CON }),
    make_basic_smith_info(SmithEffect::SUST_CHR, _("魅力維持", "sustain charisma"), SmithCategory::ABILITY, { SmithEssence::SUST_STATUS }, 15, { TR_SUST_CHR }),

    make_basic_smith_info(SmithEffect::MAGIC_MASTERY, _("魔力支配", "magic mastery"), SmithCategory::PVAL, { SmithEssence::MAGIC_MASTERY }, 20, { TR_MAGIC_MASTERY }),
    make_basic_smith_info(SmithEffect::STEALTH, _("隠密", "stealth"), SmithCategory::PVAL, { SmithEssence::STEALTH }, 40, { TR_STEALTH }),
    make_basic_smith_info(SmithEffect::SEARCH, _("探索", "searching"), SmithCategory::PVAL, { SmithEssence::SEARCH }, 15, { TR_SEARCH }),
    make_basic_smith_info(SmithEffect::INFRA, _("赤外線視力", "infravision"), SmithCategory::PVAL, { SmithEssence::INFRA }, 15, { TR_INFRA }),
    make_basic_smith_info(SmithEffect::TUNNEL, _("採掘", "digging"), SmithCategory::PVAL, { SmithEssence::TUNNEL }, 15, { TR_TUNNEL }),
    make_basic_smith_info(SmithEffect::SPEED, _("スピード", "speed"), SmithCategory::PVAL, { SmithEssence::SPEED }, 12, { TR_SPEED }),
    make_basic_smith_info(SmithEffect::BLOWS, _("追加攻撃", "extra attack"), SmithCategory::WEAPON_ATTR, { SmithEssence::BLOWS }, 20, { TR_BLOWS }),

    make_basic_smith_info(SmithEffect::CHAOTIC, _("カオス攻撃", "chaos brand"), SmithCategory::WEAPON_ATTR, { SmithEssence::CHAOTIC }, 15, { TR_CHAOTIC }),
    make_basic_smith_info(SmithEffect::VAMPIRIC, _("吸血攻撃", "vampiric brand"), SmithCategory::WEAPON_ATTR, { SmithEssence::VAMPIRIC }, 60, { TR_VAMPIRIC }),
    make_basic_smith_info(SmithEffect::BRAND_POIS, _("毒殺", "poison brand"), SmithCategory::WEAPON_ATTR, { SmithEssence::BRAND_POIS }, 20, { TR_BRAND_POIS }),
    make_basic_smith_info(SmithEffect::BRAND_ACID, _("溶解", "acid brand"), SmithCategory::WEAPON_ATTR, { SmithEssence::BRAND_ACID }, 20, { TR_BRAND_ACID }),
    make_basic_smith_info(SmithEffect::BRAND_ELEC, _("電撃", "electric brand"), SmithCategory::WEAPON_ATTR, { SmithEssence::BRAND_ELEC }, 20, { TR_BRAND_ELEC }),
    make_basic_smith_info(SmithEffect::BRAND_FIRE, _("焼棄", "fire brand"), SmithCategory::WEAPON_ATTR, { SmithEssence::BRAND_FIRE }, 20, { TR_BRAND_FIRE }),
    make_basic_smith_info(SmithEffect::BRAND_COLD, _("凍結", "cold brand"), SmithCategory::WEAPON_ATTR, { SmithEssence::BRAND_COLD }, 20, { TR_BRAND_COLD }),

    make_basic_smith_info(SmithEffect::IM_ACID, _("酸免疫", "acid immunity"), SmithCategory::RESISTANCE, { SmithEssence::IMMUNITY }, 20, { TR_IM_ACID }),
    make_basic_smith_info(SmithEffect::IM_ELEC, _("電撃免疫", "electric immunity"), SmithCategory::RESISTANCE, { SmithEssence::IMMUNITY }, 20, { TR_IM_ELEC }),
    make_basic_smith_info(SmithEffect::IM_FIRE, _("火炎免疫", "fire immunity"), SmithCategory::RESISTANCE, { SmithEssence::IMMUNITY }, 20, { TR_IM_FIRE }),
    make_basic_smith_info(SmithEffect::IM_COLD, _("冷気免疫", "cold immunity"), SmithCategory::RESISTANCE, { SmithEssence::IMMUNITY }, 20, { TR_IM_COLD }),
    make_basic_smith_info(SmithEffect::REFLECT, _("反射", "reflection"), SmithCategory::RESISTANCE, { SmithEssence::REFLECT }, 20, { TR_REFLECT }),

    make_basic_smith_info(SmithEffect::RES_ACID, _("耐酸", "resistance to acid"), SmithCategory::RESISTANCE, { SmithEssence::RES_ACID }, 15, { TR_RES_ACID }),
    make_basic_smith_info(SmithEffect::RES_ELEC, _("耐電撃", "resistance to electric"), SmithCategory::RESISTANCE, { SmithEssence::RES_ELEC }, 15, { TR_RES_ELEC }),
    make_basic_smith_info(SmithEffect::RES_FIRE, _("耐火炎", "resistance to fire"), SmithCategory::RESISTANCE, { SmithEssence::RES_FIRE }, 15, { TR_RES_FIRE }),
    make_basic_smith_info(SmithEffect::RES_COLD, _("耐冷気", "resistance to cold"), SmithCategory::RESISTANCE, { SmithEssence::RES_COLD }, 15, { TR_RES_COLD }),
    make_basic_smith_info(SmithEffect::RES_POIS, _("耐毒", "resistance to poison"), SmithCategory::RESISTANCE, { SmithEssence::RES_POIS }, 25, { TR_RES_POIS }),
    make_basic_smith_info(SmithEffect::RES_FEAR, _("耐恐怖", "resistance to fear"), SmithCategory::RESISTANCE, { SmithEssence::RES_FEAR }, 20, { TR_RES_FEAR }),
    make_basic_smith_info(SmithEffect::RES_LITE, _("耐閃光", "resistance to light"), SmithCategory::RESISTANCE, { SmithEssence::RES_LITE }, 20, { TR_RES_LITE }),
    make_basic_smith_info(SmithEffect::RES_DARK, _("耐暗黒", "resistance to dark"), SmithCategory::RESISTANCE, { SmithEssence::RES_DARK }, 20, { TR_RES_DARK }),
    make_basic_smith_info(SmithEffect::RES_BLIND, _("耐盲目", "resistance to blind"), SmithCategory::RESISTANCE, { SmithEssence::RES_BLIND }, 20, { TR_RES_BLIND }),
    make_basic_smith_info(SmithEffect::RES_CONF, _("耐混乱", "resistance to confusion"), SmithCategory::RESISTANCE, { SmithEssence::RES_CONF }, 20, { TR_RES_CONF }),
    make_basic_smith_info(SmithEffect::RES_SOUND, _("耐轟音", "resistance to sound"), SmithCategory::RESISTANCE, { SmithEssence::RES_SOUND }, 20, { TR_RES_SOUND }),
    make_basic_smith_info(SmithEffect::RES_SHARDS, _("耐破片", "resistance to shard"), SmithCategory::RESISTANCE, { SmithEssence::RES_SHARDS }, 20, { TR_RES_SHARDS }),
    make_basic_smith_info(SmithEffect::RES_NETHER, _("耐地獄", "resistance to nether"), SmithCategory::RESISTANCE, { SmithEssence::RES_NETHER }, 20, { TR_RES_NETHER }),
    make_basic_smith_info(SmithEffect::RES_NEXUS, _("耐因果混乱", "resistance to nexus"), SmithCategory::RESISTANCE, { SmithEssence::RES_NEXUS }, 20, { TR_RES_NEXUS }),
    make_basic_smith_info(SmithEffect::RES_CHAOS, _("耐カオス", "resistance to chaos"), SmithCategory::RESISTANCE, { SmithEssence::RES_CHAOS }, 20, { TR_RES_CHAOS }),
    make_basic_smith_info(SmithEffect::RES_DISEN, _("耐劣化", "resistance to disenchantment"), SmithCategory::RESISTANCE, { SmithEssence::RES_DISEN }, 20, { TR_RES_DISEN }),

    make_basic_smith_info(SmithEffect::HOLD_EXP, _("経験値維持", "hold experience"), SmithCategory::ABILITY, { SmithEssence::HOLD_EXP }, 20, { TR_HOLD_EXP }),
    make_basic_smith_info(SmithEffect::FREE_ACT, _("麻痺知らず", "free action"), SmithCategory::ABILITY, { SmithEssence::FREE_ACT }, 20, { TR_FREE_ACT }),
    make_basic_smith_info(SmithEffect::WARNING, _("警告", "warning"), SmithCategory::ABILITY, { SmithEssence::WARNING }, 20, { TR_WARNING }),
    make_basic_smith_info(SmithEffect::LEVITATION, _("浮遊", "levitation"), SmithCategory::ABILITY, { SmithEssence::LEVITATION }, 20, { TR_LEVITATION }),
    make_basic_smith_info(SmithEffect::SEE_INVIS, _("可視透明", "see invisible"), SmithCategory::ABILITY, { SmithEssence::SEE_INVIS }, 20, { TR_SEE_INVIS }),
    make_basic_smith_info(SmithEffect::SLOW_DIGEST, _("遅消化", "slow digestion"), SmithCategory::ABILITY, { SmithEssence::SLOW_DIGEST }, 15, { TR_SLOW_DIGEST }),
    make_basic_smith_info(SmithEffect::REGEN, _("急速回復", "regeneration"), SmithCategory::ABILITY, { SmithEssence::REGEN }, 20, { TR_REGEN }),
    make_basic_smith_info(SmithEffect::TELEPORT, _("テレポート", "teleport"), SmithCategory::ABILITY, { SmithEssence::TELEPORT }, 25, { TR_TELEPORT }),
    make_basic_smith_info(SmithEffect::NO_MAGIC, _("反魔法", "anti magic"), SmithCategory::ABILITY, { SmithEssence::NO_MAGIC }, 15, { TR_NO_MAGIC }),
    make_basic_smith_info(SmithEffect::LITE, _("永久光源", "permanent light"), SmithCategory::ABILITY, { SmithEssence::LITE }, 15, { TR_LITE_1 }),

    make_basic_smith_info(SmithEffect::SLAY_EVIL, _("邪悪倍打", "slay evil"), SmithCategory::SLAYING, { SmithEssence::SLAY_EVIL }, 100, { TR_SLAY_EVIL }),
    make_basic_smith_info(SmithEffect::SLAY_ANIMAL, _("動物倍打", "slay animal"), SmithCategory::SLAYING, { SmithEssence::SLAY_ANIMAL }, 20, { TR_SLAY_ANIMAL }),
    make_basic_smith_info(SmithEffect::SLAY_UNDEAD, _("不死倍打", "slay undead"), SmithCategory::SLAYING, { SmithEssence::SLAY_UNDEAD }, 20, { TR_SLAY_UNDEAD }),
    make_basic_smith_info(SmithEffect::SLAY_DEMON, _("悪魔倍打", "slay demon"), SmithCategory::SLAYING, { SmithEssence::SLAY_DEMON }, 20, { TR_SLAY_DEMON }),
    make_basic_smith_info(SmithEffect::SLAY_ORC, _("オーク倍打", "slay orc"), SmithCategory::SLAYING, { SmithEssence::SLAY_ORC }, 20, { TR_SLAY_ORC }),
    make_basic_smith_info(SmithEffect::SLAY_TROLL, _("トロル倍打", "slay troll"), SmithCategory::SLAYING, { SmithEssence::SLAY_TROLL }, 20, { TR_SLAY_TROLL }),
    make_basic_smith_info(SmithEffect::SLAY_GIANT, _("巨人倍打", "slay giant"), SmithCategory::SLAYING, { SmithEssence::SLAY_GIANT }, 20, { TR_SLAY_GIANT }),
    make_basic_smith_info(SmithEffect::SLAY_DRAGON, _("竜倍打", "slay dragon"), SmithCategory::SLAYING, { SmithEssence::SLAY_DRAGON }, 20, { TR_SLAY_DRAGON }),
    make_basic_smith_info(SmithEffect::SLAY_HUMAN, _("人間倍打", "slay human"), SmithCategory::SLAYING, { SmithEssence::SLAY_HUMAN }, 20, { TR_SLAY_HUMAN }),

    make_basic_smith_info(SmithEffect::KILL_EVIL, _("邪悪倍倍打", "kill evil"), SmithCategory::NONE, { SmithEssence::SLAY_EVIL }, 0, { TR_KILL_EVIL }), // 強力すぎるため無効(SmithCategory:NONE)
    make_basic_smith_info(SmithEffect::KILL_ANIMAL, _("動物倍倍打", "kill animal"), SmithCategory::SLAYING, { SmithEssence::SLAY_ANIMAL }, 60, { TR_KILL_ANIMAL }),
    make_basic_smith_info(SmithEffect::KILL_UNDEAD, _("不死倍倍打", "kill undead"), SmithCategory::SLAYING, { SmithEssence::SLAY_UNDEAD }, 60, { TR_KILL_UNDEAD }),
    make_basic_smith_info(SmithEffect::KILL_DEMON, _("悪魔倍倍打", "kill demon"), SmithCategory::SLAYING, { SmithEssence::SLAY_DEMON }, 60, { TR_KILL_DEMON }),
    make_basic_smith_info(SmithEffect::KILL_ORC, _("オーク倍倍打", "kill orc"), SmithCategory::SLAYING, { SmithEssence::SLAY_ORC }, 60, { TR_KILL_ORC }),
    make_basic_smith_info(SmithEffect::KILL_TROLL, _("トロル倍倍打", "kill troll"), SmithCategory::SLAYING, { SmithEssence::SLAY_TROLL }, 60, { TR_KILL_TROLL }),
    make_basic_smith_info(SmithEffect::KILL_GIANT, _("巨人倍倍打", "kill giant"), SmithCategory::SLAYING, { SmithEssence::SLAY_GIANT }, 60, { TR_KILL_GIANT }),
    make_basic_smith_info(SmithEffect::KILL_DRAGON, _("竜倍倍打", "kill dragon"), SmithCategory::SLAYING, { SmithEssence::SLAY_DRAGON }, 60, { TR_KILL_DRAGON }),
    make_basic_smith_info(SmithEffect::KILL_HUMAN, _("人間倍倍打", "kill human"), SmithCategory::SLAYING, { SmithEssence::SLAY_HUMAN }, 60, { TR_KILL_HUMAN }),

    make_basic_smith_info(SmithEffect::TELEPATHY, _("テレパシー", "telepathy"), SmithCategory::ESP, { SmithEssence::TELEPATHY }, 15, { TR_TELEPATHY }),
    make_basic_smith_info(SmithEffect::ESP_ANIMAL, _("動物ESP", "sense animal"), SmithCategory::ESP, { SmithEssence::SLAY_ANIMAL }, 40, { TR_ESP_ANIMAL }),
    make_basic_smith_info(SmithEffect::ESP_UNDEAD, _("不死ESP", "sense undead"), SmithCategory::ESP, { SmithEssence::SLAY_UNDEAD }, 40, { TR_ESP_UNDEAD }),
    make_basic_smith_info(SmithEffect::ESP_DEMON, _("悪魔ESP", "sense demon"), SmithCategory::ESP, { SmithEssence::SLAY_DEMON }, 40, { TR_ESP_DEMON }),
    make_basic_smith_info(SmithEffect::ESP_ORC, _("オークESP", "sense orc"), SmithCategory::ESP, { SmithEssence::SLAY_ORC }, 40, { TR_ESP_ORC }),
    make_basic_smith_info(SmithEffect::ESP_TROLL, _("トロルESP", "sense troll"), SmithCategory::ESP, { SmithEssence::SLAY_TROLL }, 40, { TR_ESP_TROLL }),
    make_basic_smith_info(SmithEffect::ESP_GIANT, _("巨人ESP", "sense giant"), SmithCategory::ESP, { SmithEssence::SLAY_GIANT }, 40, { TR_ESP_GIANT }),
    make_basic_smith_info(SmithEffect::ESP_DRAGON, _("竜ESP", "sense dragon"), SmithCategory::ESP, { SmithEssence::SLAY_DRAGON }, 40, { TR_ESP_DRAGON }),
    make_basic_smith_info(SmithEffect::ESP_HUMAN, _("人間ESP", "sense human"), SmithCategory::ESP, { SmithEssence::SLAY_HUMAN }, 40, { TR_ESP_HUMAN }),

    make_info<activation_smith_info>(SmithEffect::EARTHQUAKE, _("地震発動", "quake activation"), SmithCategory::ETC, { SmithEssence::EATHQUAKE }, 15, TrFlags{ TR_EARTHQUAKE }, ACT_QUAKE),
    make_info<activation_smith_info>(SmithEffect::TMP_RES_ACID, _("酸耐性発動", "resist acid activation"), SmithCategory::ETC, { SmithEssence::RES_ACID }, 50, TrFlags{ TR_RES_ACID }, ACT_RESIST_ACID),
    make_info<activation_smith_info>(SmithEffect::TMP_RES_ELEC, _("電撃耐性発動", "resist electricity activation"), SmithCategory::ETC, { SmithEssence::RES_ELEC }, 50, TrFlags{ TR_RES_ELEC }, ACT_RESIST_ELEC),
    make_info<activation_smith_info>(SmithEffect::TMP_RES_FIRE, _("火炎耐性発動", "resist fire activation"), SmithCategory::ETC, { SmithEssence::RES_FIRE }, 50, TrFlags{ TR_RES_FIRE }, ACT_RESIST_FIRE),
    make_info<activation_smith_info>(SmithEffect::TMP_RES_COLD, _("冷気耐性発動", "resist cold activation"), SmithCategory::ETC, { SmithEssence::RES_COLD }, 50, TrFlags{ TR_RES_COLD }, ACT_RESIST_COLD),
    make_basic_smith_info(SmithEffect::SH_FIRE, _("火炎オーラ", "fiery sheath"), SmithCategory::ETC, { SmithEssence::RES_FIRE, SmithEssence::BRAND_FIRE }, 50, { TR_RES_FIRE, TR_SH_FIRE }),
    make_basic_smith_info(SmithEffect::SH_ELEC, _("電撃オーラ", "electric sheath"), SmithCategory::ETC, { SmithEssence::RES_ELEC, SmithEssence::BRAND_ELEC }, 50, { TR_RES_ELEC, TR_SH_ELEC }),
    make_basic_smith_info(SmithEffect::SH_COLD, _("冷気オーラ", "sheath of coldness"), SmithCategory::ETC, { SmithEssence::RES_COLD, SmithEssence::BRAND_COLD }, 50, { TR_RES_COLD, TR_SH_COLD }),

    make_basic_smith_info(SmithEffect::RESISTANCE, _("全耐性", "resistance"), SmithCategory::RESISTANCE, { SmithEssence::RES_ACID, SmithEssence::RES_ELEC, SmithEssence::RES_FIRE, SmithEssence::RES_COLD }, 150, { TR_RES_ACID, TR_RES_ELEC, TR_RES_FIRE, TR_RES_COLD }),
    make_info<slaying_gloves_smith_info>(SmithEffect::SLAY_GLOVE, _("殺戮の小手", "gauntlets of slaying"), SmithCategory::WEAPON_ATTR, { SmithEssence::ATTACK }, 200),

    make_info<enchant_weapon_smith_info>(SmithEffect::ATTACK, _("攻撃", "weapon enchant"), SmithCategory::ENCHANT, { SmithEssence::ATTACK }, 30),
    make_info<enchant_armour_smith_info>(SmithEffect::AC, _("防御", "armor enchant"), SmithCategory::ENCHANT, { SmithEssence::AC }, 15),
    make_info<sustain_smith_info>(SmithEffect::SUSTAIN, _("装備保持", "elements proof"), SmithCategory::ENCHANT, { SmithEssence::RES_ACID, SmithEssence::RES_ELEC, SmithEssence::RES_FIRE, SmithEssence::RES_COLD }, 10),
};
