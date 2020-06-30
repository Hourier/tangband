﻿/*!
 * @brief プレイヤーのステータス管理 / effects of various "objects"
 * @date 2014/01/01
 * @author
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * 2013 Deskull rearranged comment for Doxygen.
 */

#include "player/player-effects.h"
#include "autopick/autopick-reader-writer.h"
#include "birth/birth-body-spec.h"
#include "birth/birth-stat.h"
#include "birth/character-builder.h"
#include "cmd-building/cmd-building.h"
#include "cmd-io/cmd-dump.h"
#include "core/speed-table.h"
#include "core/stuff-handler.h"
#include "dungeon/quest.h"
#include "floor/floor.h"
#include "floor/wild.h"
#include "game-option/birth-options.h"
#include "game-option/disturbance-options.h"
#include "grid/grid.h"
#include "inventory/inventory-object.h"
#include "io/input-key-acceptor.h"
#include "io/report.h"
#include "io/save.h"
#include "locale/vowel-checker.h"
#include "mind/mind-force-trainer.h"
#include "mind/mind-magic-resistance.h"
#include "mind/mind-mirror-master.h"
#include "mind/mind-sniper.h"
#include "mind/stances-table.h"
#include "monster/monster-status.h"
#include "mutation/mutation.h"
#include "object-enchant/artifact.h"
#include "object-enchant/item-feeling.h"
#include "object-enchant/object-ego.h"
#include "object-enchant/special-object-flags.h"
#include "object-enchant/trg-types.h"
#include "object-hook/hook-checker.h"
#include "object-hook/hook-enchant.h"
#include "object-hook/hook-weapon.h"
#include "object/object-generator.h"
#include "object/object-kind.h"
#include "object/object-value-calc.h"
#include "object/object-value.h"
#include "perception/object-perception.h"
#include "player/avatar.h"
#include "player/player-class.h"
#include "player/player-damage.h"
#include "player/player-move.h"
#include "player/player-personalities-types.h"
#include "player/player-personality.h"
#include "player/player-race-types.h"
#include "player/player-sex.h"
#include "player/race-info-table.h"
#include "realm/realm-song-numbers.h"
#include "spell-kind/spells-floor.h"
#include "spell-realm/spells-craft.h"
#include "spell-realm/spells-crusade.h"
#include "spell-realm/spells-demon.h"
#include "spell-realm/spells-hex.h"
#include "spell-realm/spells-song.h"
#include "spell/spells-status.h"
#include "status/bad-status-setter.h"
#include "status/base-status.h"
#include "status/buff-setter.h"
#include "status/element-resistance.h"
#include "status/form-changer.h"
#include "status/sight-setter.h"
#include "status/temporary-resistance.h"
#include "sv-definition/sv-armor-types.h"
#include "sv-definition/sv-protector-types.h"
#include "sv-definition/sv-weapon-types.h"
#include "term/screen-processor.h"
#include "term/term-color-types.h"
#include "view/display-messages.h"
#include "world/world.h"

/*!
 * @brief プレイヤーの継続行動を設定する。
 * @param typ 継続行動のID\n
 * #ACTION_NONE / #ACTION_SEARCH / #ACTION_REST / #ACTION_LEARN / #ACTION_FISH / #ACTION_KAMAE / #ACTION_KATA / #ACTION_SING / #ACTION_HAYAGAKE / #ACTION_SPELL
 * から選択。
 * @return なし
 */
void set_action(player_type *creature_ptr, ACTION_IDX typ)
{
    int prev_typ = creature_ptr->action;

    if (typ == prev_typ) {
        return;
    } else {
        switch (prev_typ) {
        case ACTION_SEARCH: {
            msg_print(_("探索をやめた。", "You no longer walk carefully."));
            creature_ptr->redraw |= (PR_SPEED);
            break;
        }
        case ACTION_REST: {
            creature_ptr->resting = 0;
            break;
        }
        case ACTION_LEARN: {
            msg_print(_("学習をやめた。", "You stop learning."));
            creature_ptr->new_mane = FALSE;
            break;
        }
        case ACTION_KAMAE: {
            msg_print(_("構えをといた。", "You stop assuming the special stance."));
            creature_ptr->special_defense &= ~(KAMAE_MASK);
            break;
        }
        case ACTION_KATA: {
            msg_print(_("型を崩した。", "You stop assuming the special stance."));
            creature_ptr->special_defense &= ~(KATA_MASK);
            creature_ptr->update |= (PU_MONSTERS);
            creature_ptr->redraw |= (PR_STATUS);
            break;
        }
        case ACTION_SING: {
            msg_print(_("歌うのをやめた。", "You stop singing."));
            break;
        }
        case ACTION_HAYAGAKE: {
            msg_print(_("足が重くなった。", "You are no longer walking extremely fast."));
            take_turn(creature_ptr, 100);
            break;
        }
        case ACTION_SPELL: {
            msg_print(_("呪文の詠唱を中断した。", "You stopped casting."));
            break;
        }
        }
    }

    creature_ptr->action = typ;

    /* If we are requested other action, stop singing */
    if (prev_typ == ACTION_SING)
        stop_singing(creature_ptr);
    if (prev_typ == ACTION_SPELL)
        stop_hex_spell(creature_ptr);

    switch (creature_ptr->action) {
    case ACTION_SEARCH: {
        msg_print(_("注意深く歩き始めた。", "You begin to walk carefully."));
        creature_ptr->redraw |= (PR_SPEED);
        break;
    }
    case ACTION_LEARN: {
        msg_print(_("学習を始めた。", "You begin learning"));
        break;
    }
    case ACTION_FISH: {
        msg_print(_("水面に糸を垂らした．．．", "You begin fishing..."));
        break;
    }
    case ACTION_HAYAGAKE: {
        msg_print(_("足が羽のように軽くなった。", "You begin to walk extremely fast."));
        break;
    }
    default: {
        break;
    }
    }
    creature_ptr->update |= (PU_BONUS);
    creature_ptr->redraw |= (PR_STATE);
}

/*!
 * @brief プレイヤーに魔力消去効果を与える。
 * @return なし
 */
void dispel_player(player_type *creature_ptr)
{
    (void)set_fast(creature_ptr, 0, TRUE);
    set_lightspeed(creature_ptr, 0, TRUE);
    (void)set_slow(creature_ptr, 0, TRUE);
    (void)set_shield(creature_ptr, 0, TRUE);
    (void)set_blessed(creature_ptr, 0, TRUE);
    (void)set_tsuyoshi(creature_ptr, 0, TRUE);
    (void)set_hero(creature_ptr, 0, TRUE);
    (void)set_shero(creature_ptr, 0, TRUE);
    (void)set_protevil(creature_ptr, 0, TRUE);
    (void)set_invuln(creature_ptr, 0, TRUE);
    (void)set_wraith_form(creature_ptr, 0, TRUE);
    (void)set_kabenuke(creature_ptr, 0, TRUE);
    (void)set_tim_res_nether(creature_ptr, 0, TRUE);
    (void)set_tim_res_time(creature_ptr, 0, TRUE);
    (void)set_tim_reflect(creature_ptr, 0, TRUE);
    (void)set_multishadow(creature_ptr, 0, TRUE);
    (void)set_dustrobe(creature_ptr, 0, TRUE);

    (void)set_tim_invis(creature_ptr, 0, TRUE);
    (void)set_tim_infra(creature_ptr, 0, TRUE);
    (void)set_tim_esp(creature_ptr, 0, TRUE);
    (void)set_tim_regen(creature_ptr, 0, TRUE);
    (void)set_tim_stealth(creature_ptr, 0, TRUE);
    (void)set_tim_levitation(creature_ptr, 0, TRUE);
    (void)set_tim_sh_force(creature_ptr, 0, TRUE);
    (void)set_tim_sh_fire(creature_ptr, 0, TRUE);
    (void)set_tim_sh_holy(creature_ptr, 0, TRUE);
    (void)set_tim_eyeeye(creature_ptr, 0, TRUE);
    (void)set_magicdef(creature_ptr, 0, TRUE);
    (void)set_resist_magic(creature_ptr, 0, TRUE);
    (void)set_oppose_acid(creature_ptr, 0, TRUE);
    (void)set_oppose_elec(creature_ptr, 0, TRUE);
    (void)set_oppose_fire(creature_ptr, 0, TRUE);
    (void)set_oppose_cold(creature_ptr, 0, TRUE);
    (void)set_oppose_pois(creature_ptr, 0, TRUE);
    (void)set_ultimate_res(creature_ptr, 0, TRUE);
    (void)set_mimic(creature_ptr, 0, 0, TRUE);
    (void)set_ele_attack(creature_ptr, 0, 0);
    (void)set_ele_immune(creature_ptr, 0, 0);

    if (creature_ptr->special_attack & ATTACK_CONFUSE) {
        creature_ptr->special_attack &= ~(ATTACK_CONFUSE);
        msg_print(_("手の輝きがなくなった。", "Your hands stop glowing."));
    }

    if (music_singing_any(creature_ptr) || hex_spelling_any(creature_ptr)) {
        concptr str = (music_singing_any(creature_ptr)) ? _("歌", "singing") : _("呪文", "casting");
        INTERUPTING_SONG_EFFECT(creature_ptr) = SINGING_SONG_EFFECT(creature_ptr);
        SINGING_SONG_EFFECT(creature_ptr) = MUSIC_NONE;
        msg_format(_("%sが途切れた。", "Your %s is interrupted."), str);

        creature_ptr->action = ACTION_NONE;
        creature_ptr->update |= (PU_BONUS | PU_HP | PU_MONSTERS);
        creature_ptr->redraw |= (PR_MAP | PR_STATUS | PR_STATE);
        creature_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
        creature_ptr->energy_need += ENERGY_NEED();
    }
}

/*!
 * @brief 対邪悪結界の継続時間をセットする / Set "protevil", notice observable changes
 * @param v 継続時間
 * @param do_dec 現在の継続時間より長い値のみ上書きする
 * @return ステータスに影響を及ぼす変化があった場合TRUEを返す。
 */
bool set_protevil(player_type *creature_ptr, TIME_EFFECT v, bool do_dec)
{
    bool notice = FALSE;
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (creature_ptr->is_dead)
        return FALSE;

    if (v) {
        if (creature_ptr->protevil && !do_dec) {
            if (creature_ptr->protevil > v)
                return FALSE;
        } else if (!creature_ptr->protevil) {
            msg_print(_("邪悪なる存在から守られているような感じがする！", "You feel safe from evil!"));
            notice = TRUE;
        }
    } else {
        if (creature_ptr->protevil) {
            msg_print(_("邪悪なる存在から守られている感じがなくなった。", "You no longer feel safe from evil."));
            notice = TRUE;
        }
    }

    creature_ptr->protevil = v;
    creature_ptr->redraw |= (PR_STATUS);

    if (!notice)
        return FALSE;

    if (disturb_state)
        disturb(creature_ptr, FALSE, FALSE);
    handle_stuff(creature_ptr);
    return TRUE;
}

/*!
 * @brief 無傷球の継続時間をセットする / Set "invuln", notice observable changes
 * @param v 継続時間
 * @param do_dec 現在の継続時間より長い値のみ上書きする
 * @return ステータスに影響を及ぼす変化があった場合TRUEを返す。
 */
bool set_invuln(player_type *creature_ptr, TIME_EFFECT v, bool do_dec)
{
    bool notice = FALSE;
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (creature_ptr->is_dead)
        return FALSE;

    if (v) {
        if (creature_ptr->invuln && !do_dec) {
            if (creature_ptr->invuln > v)
                return FALSE;
        } else if (!is_invuln(creature_ptr)) {
            msg_print(_("無敵だ！", "Invulnerability!"));
            notice = TRUE;

            chg_virtue(creature_ptr, V_UNLIFE, -2);
            chg_virtue(creature_ptr, V_HONOUR, -2);
            chg_virtue(creature_ptr, V_SACRIFICE, -3);
            chg_virtue(creature_ptr, V_VALOUR, -5);

            creature_ptr->redraw |= (PR_MAP);
            creature_ptr->update |= (PU_MONSTERS);

            creature_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
        }
    } else {
        if (creature_ptr->invuln && !music_singing(creature_ptr, MUSIC_INVULN)) {
            msg_print(_("無敵ではなくなった。", "The invulnerability wears off."));
            notice = TRUE;

            creature_ptr->redraw |= (PR_MAP);
            creature_ptr->update |= (PU_MONSTERS);

            creature_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

            creature_ptr->energy_need += ENERGY_NEED();
        }
    }

    creature_ptr->invuln = v;
    creature_ptr->redraw |= (PR_STATUS);

    if (!notice)
        return FALSE;

    if (disturb_state)
        disturb(creature_ptr, FALSE, FALSE);
    creature_ptr->update |= (PU_BONUS);
    handle_stuff(creature_ptr);
    return TRUE;
}

/*!
 * @brief 時限急回復の継続時間をセットする / Set "tim_regen", notice observable changes
 * @param v 継続時間
 * @param do_dec 現在の継続時間より長い値のみ上書きする
 * @return ステータスに影響を及ぼす変化があった場合TRUEを返す。
 */
bool set_tim_regen(player_type *creature_ptr, TIME_EFFECT v, bool do_dec)
{
    bool notice = FALSE;
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (creature_ptr->is_dead)
        return FALSE;

    if (v) {
        if (creature_ptr->tim_regen && !do_dec) {
            if (creature_ptr->tim_regen > v)
                return FALSE;
        } else if (!creature_ptr->tim_regen) {
            msg_print(_("回復力が上がった！", "You feel yourself regenerating quickly!"));
            notice = TRUE;
        }
    } else {
        if (creature_ptr->tim_regen) {
            msg_print(_("素早く回復する感じがなくなった。", "You feel yourself regenerating slowly."));
            notice = TRUE;
        }
    }

    creature_ptr->tim_regen = v;
    creature_ptr->redraw |= (PR_STATUS);

    if (!notice)
        return FALSE;

    if (disturb_state)
        disturb(creature_ptr, FALSE, FALSE);
    creature_ptr->update |= (PU_BONUS);
    handle_stuff(creature_ptr);
    return TRUE;
}

/*!
 * @brief 一時的反射の継続時間をセットする / Set "tim_reflect", notice observable changes
 * @param v 継続時間
 * @param do_dec 現在の継続時間より長い値のみ上書きする
 * @return ステータスに影響を及ぼす変化があった場合TRUEを返す。
 */
bool set_tim_reflect(player_type *creature_ptr, TIME_EFFECT v, bool do_dec)
{
    bool notice = FALSE;
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (creature_ptr->is_dead)
        return FALSE;

    if (v) {
        if (creature_ptr->tim_reflect && !do_dec) {
            if (creature_ptr->tim_reflect > v)
                return FALSE;
        } else if (!creature_ptr->tim_reflect) {
            msg_print(_("体の表面が滑かになった気がする。", "Your body becames smooth."));
            notice = TRUE;
        }
    } else {
        if (creature_ptr->tim_reflect) {
            msg_print(_("体の表面が滑かでなくなった。", "Your body is no longer smooth."));
            notice = TRUE;
        }
    }

    creature_ptr->tim_reflect = v;
    creature_ptr->redraw |= (PR_STATUS);

    if (!notice)
        return FALSE;

    if (disturb_state)
        disturb(creature_ptr, FALSE, FALSE);
    creature_ptr->update |= (PU_BONUS);
    handle_stuff(creature_ptr);
    return TRUE;
}

/*!
 * @brief 一時的壁抜けの継続時間をセットする / Set "kabenuke", notice observable changes
 * @param v 継続時間
 * @param do_dec 現在の継続時間より長い値のみ上書きする
 * @return ステータスに影響を及ぼす変化があった場合TRUEを返す。
 */
bool set_kabenuke(player_type *creature_ptr, TIME_EFFECT v, bool do_dec)
{
    bool notice = FALSE;
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (creature_ptr->is_dead)
        return FALSE;

    if (v) {
        if (creature_ptr->kabenuke && !do_dec) {
            if (creature_ptr->kabenuke > v)
                return FALSE;
        } else if (!creature_ptr->kabenuke) {
            msg_print(_("体が半物質の状態になった。", "You became ethereal."));
            notice = TRUE;
        }
    } else {
        if (creature_ptr->kabenuke) {
            msg_print(_("体が物質化した。", "You are no longer ethereal."));
            notice = TRUE;
        }
    }

    creature_ptr->kabenuke = v;
    creature_ptr->redraw |= (PR_STATUS);

    if (!notice)
        return FALSE;

    if (disturb_state)
        disturb(creature_ptr, FALSE, FALSE);
    creature_ptr->update |= (PU_BONUS);
    handle_stuff(creature_ptr);
    return TRUE;
}

/*
 * Increase players hit points, notice effects
 */
bool hp_player(player_type *creature_ptr, int num)
{
    int vir;
    vir = virtue_number(creature_ptr, V_VITALITY);

    if (num <= 0)
        return FALSE;

    if (vir) {
        num = num * (creature_ptr->virtues[vir - 1] + 1250) / 1250;
    }

    if (creature_ptr->chp < creature_ptr->mhp) {
        if ((num > 0) && (creature_ptr->chp < (creature_ptr->mhp / 3)))
            chg_virtue(creature_ptr, V_TEMPERANCE, 1);

        creature_ptr->chp += num;
        if (creature_ptr->chp >= creature_ptr->mhp) {
            creature_ptr->chp = creature_ptr->mhp;
            creature_ptr->chp_frac = 0;
        }

        creature_ptr->redraw |= (PR_HP);
        creature_ptr->window |= (PW_PLAYER);
        if (num < 5) {
            msg_print(_("少し気分が良くなった。", "You feel a little better."));
        } else if (num < 15) {
            msg_print(_("気分が良くなった。", "You feel better."));
        } else if (num < 35) {
            msg_print(_("とても気分が良くなった。", "You feel much better."));
        } else {
            msg_print(_("ひじょうに気分が良くなった。", "You feel very good."));
        }

        return TRUE;
    }

    return FALSE;
}

void do_poly_wounds(player_type *creature_ptr)
{
    s16b wounds = creature_ptr->cut;
    s16b hit_p = (creature_ptr->mhp - creature_ptr->chp);
    s16b change = damroll(creature_ptr->lev, 5);
    bool Nasty_effect = one_in_(5);
    if (!(wounds || hit_p || Nasty_effect))
        return;

    msg_print(_("傷がより軽いものに変化した。", "Your wounds are polymorphed into less serious ones."));
    hp_player(creature_ptr, change);
    if (Nasty_effect) {
        msg_print(_("新たな傷ができた！", "A new wound was created!"));
        take_hit(creature_ptr, DAMAGE_LOSELIFE, change / 2, _("変化した傷", "a polymorphed wound"), -1);
        set_cut(creature_ptr, change);
    } else {
        set_cut(creature_ptr, creature_ptr->cut - (change / 2));
    }
}

/*
 * Change player race
 */
void change_race(player_type *creature_ptr, player_race_type new_race, concptr effect_msg)
{
    concptr title = race_info[new_race].title;
    int old_race = creature_ptr->prace;
#ifdef JP
    msg_format("あなたは%s%sに変化した！", effect_msg, title);
#else
    msg_format("You turn into %s %s%s!", (!effect_msg[0] && is_a_vowel(title[0]) ? "an" : "a"), effect_msg, title);
#endif

    chg_virtue(creature_ptr, V_CHANCE, 2);
    if (creature_ptr->prace < 32) {
        creature_ptr->old_race1 |= 1L << creature_ptr->prace;
    } else {
        creature_ptr->old_race2 |= 1L << (creature_ptr->prace - 32);
    }

    creature_ptr->prace = new_race;
    rp_ptr = &race_info[creature_ptr->prace];
    creature_ptr->expfact = rp_ptr->r_exp + cp_ptr->c_exp;

    bool is_special_class = creature_ptr->pclass == CLASS_MONK;
    is_special_class |= creature_ptr->pclass == CLASS_FORCETRAINER;
    is_special_class |= creature_ptr->pclass == CLASS_NINJA;
    bool is_special_race = creature_ptr->prace == RACE_KLACKON;
    is_special_race |= creature_ptr->prace == RACE_SPRITE;
    if (is_special_class && is_special_race)
        creature_ptr->expfact -= 15;

    get_height_weight(creature_ptr);

    if (creature_ptr->pclass == CLASS_SORCERER)
        creature_ptr->hitdie = rp_ptr->r_mhp / 2 + cp_ptr->c_mhp + ap_ptr->a_mhp;
    else
        creature_ptr->hitdie = rp_ptr->r_mhp + cp_ptr->c_mhp + ap_ptr->a_mhp;

    roll_hitdice(creature_ptr, 0L);
    check_experience(creature_ptr);
    creature_ptr->redraw |= (PR_BASIC);
    creature_ptr->update |= (PU_BONUS);
    handle_stuff(creature_ptr);

    if (old_race != creature_ptr->prace)
        autopick_load_pref(creature_ptr, FALSE);

    lite_spot(creature_ptr, creature_ptr->y, creature_ptr->x);
}

void do_poly_self(player_type *creature_ptr)
{
    int power = creature_ptr->lev;

    msg_print(_("あなたは変化の訪れを感じた...", "You feel a change coming over you..."));
    chg_virtue(creature_ptr, V_CHANCE, 1);

    if ((power > randint0(20)) && one_in_(3) && (creature_ptr->prace != RACE_ANDROID)) {
        char effect_msg[80] = "";
        player_race_type new_race;

        power -= 10;
        if ((power > randint0(5)) && one_in_(4)) {
            power -= 2;
            if (creature_ptr->psex == SEX_MALE) {
                creature_ptr->psex = SEX_FEMALE;
                sp_ptr = &sex_info[creature_ptr->psex];
                sprintf(effect_msg, _("女性の", "female "));
            } else {
                creature_ptr->psex = SEX_MALE;
                sp_ptr = &sex_info[creature_ptr->psex];
                sprintf(effect_msg, _("男性の", "male "));
            }
        }

        if ((power > randint0(30)) && one_in_(5)) {
            int tmp = 0;
            power -= 15;
            while (tmp < A_MAX) {
                if (one_in_(2)) {
                    (void)dec_stat(creature_ptr, tmp, randint1(6) + 6, one_in_(3));
                    power -= 1;
                }
                tmp++;
            }

            (void)dec_stat(creature_ptr, A_CHR, randint1(6), TRUE);

            if (effect_msg[0]) {
                char tmp_msg[10];
                sprintf(tmp_msg, _("%s", "%s "), effect_msg);
                sprintf(effect_msg, _("奇形の%s", "deformed %s "), tmp_msg);
            } else {
                sprintf(effect_msg, _("奇形の", "deformed "));
            }
        }

        while ((power > randint0(20)) && one_in_(10)) {
            power -= 10;

            if (!lose_mutation(creature_ptr, 0))
                msg_print(_("奇妙なくらい普通になった気がする。", "You feel oddly normal."));
        }

        do {
            new_race = (player_race_type)randint0(MAX_RACES);
        } while ((new_race == creature_ptr->prace) || (new_race == RACE_ANDROID));

        change_race(creature_ptr, new_race, effect_msg);
    }

    if ((power > randint0(30)) && one_in_(6)) {
        int tmp = 0;
        power -= 20;
        msg_format(_("%sの構成が変化した！", "Your internal organs are rearranged!"), creature_ptr->prace == RACE_ANDROID ? "機械" : "内臓");

        while (tmp < A_MAX) {
            (void)dec_stat(creature_ptr, tmp, randint1(6) + 6, one_in_(3));
            tmp++;
        }
        if (one_in_(6)) {
            msg_print(_("現在の姿で生きていくのは困難なようだ！", "You find living difficult in your present form!"));
            take_hit(creature_ptr, DAMAGE_LOSELIFE, damroll(randint1(10), creature_ptr->lev), _("致命的な突然変異", "a lethal mutation"), -1);

            power -= 10;
        }
    }

    if ((power > randint0(20)) && one_in_(4)) {
        power -= 10;

        get_max_stats(creature_ptr);
        roll_hitdice(creature_ptr, 0L);
    }

    while ((power > randint0(15)) && one_in_(3)) {
        power -= 7;
        (void)gain_mutation(creature_ptr, 0);
    }

    if (power > randint0(5)) {
        power -= 5;
        do_poly_wounds(creature_ptr);
    }

    while (power > 0) {
        status_shuffle(creature_ptr);
        power--;
    }
}
