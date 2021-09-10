﻿#pragma once

#include "system/angband.h"

#include "object-enchant/tr-flags.h"

#include <optional>
#include <vector>

enum class SmithEffect;
enum class SmithCategory;
enum class SmithEssence;
enum random_art_activation_type : uint8_t;

struct player_type;
struct object_type;

/*!
 * @brief 鍛冶効果の情報の基底クラス
 */
class smith_info_base {
public:
    virtual ~smith_info_base() = default;

    /*!
     * @brief 鍛冶効果を付与する
     *
     * @param o_ptr 鍛冶効果の付与を行うアイテム構造体へのポインタ
     * @param number 付与を行うエッセンスの個数
     * @return 鍛冶効果の付与に成功した場合は true、失敗した場合は false
     */
    virtual bool add_essence(player_type *player_ptr, object_type *o_ptr, int number) const = 0;

    /*!
     * @brief 鍛冶効果を消去する
     *
     * @param o_ptr 鍛冶効果の消去を行うアイテム構造体へのポインタ
     */
    virtual void erase_essence(object_type *o_ptr) const = 0;

    /*!
     * @brief 鍛冶効果により与えられる特性フラグ(tr_type)のFlagGroupオブジェクトを取得する
     * 基底クラスは空のFlagGroupオブジェクトを返す。派生クラスで必要に応じて鍛冶効果で与えられる特性フラグを返すようオーバーライドする。
     *
     * @return TrFlags 鍛冶効果により与えられる特性フラグ(tr_type)のFlagGroupオブジェクト
     */
    virtual TrFlags tr_flags() const;

    /*!
     * @brief 鍛冶効果により与えられる発動IDを取得する
     * 基底クラスは std::nullopt を返す。派生クラスで必要に応じて鍛冶効果により与えられる発動IDを返すようオーバーライドする。
     *
     * @return std::optional<random_art_activation_type> 発動IDを保持する std::optional オブジェクト
     * 発動効果が付与されない場合は std::nullopt
     */
    virtual std::optional<random_art_activation_type> activation_index() const;

    SmithEffect effect; //!< 鍛冶で与える効果の種類
    concptr name; //!< 鍛冶で与える能力の名称
    SmithCategory category; //!< 鍛冶で与える能力が所属するグループ
    std::vector<SmithEssence> need_essences; //!< 能力を与えるのに必要なエッセンスのリスト
    int consumption; //!< 能力を与えるのに必要な消費量(need_essencesに含まれるエッセンスそれぞれについてこの量を消費)

protected:
    smith_info_base(SmithEffect effect, concptr name, SmithCategory category, std::vector<SmithEssence> need_essences, int consumption);
};

/*!
 * @brief 基本的な鍛冶効果の情報クラス
 * 多くの鍛冶効果が該当する、指定した特性フラグを与える鍛冶の情報を扱う
 */
class basic_smith_info : public smith_info_base {
public:
    basic_smith_info(SmithEffect effect, concptr name, SmithCategory category, std::vector<SmithEssence> need_essences, int consumption, TrFlags add_flags);
    virtual bool add_essence(player_type *player_ptr, object_type *o_ptr, int number) const override;
    virtual void erase_essence(object_type *o_ptr) const override;
    virtual TrFlags tr_flags() const override;

private:
    TrFlags add_flags; //!< 鍛冶で能力を与えることにより付与されるアイテム特性フラグ
};

/*!
 * @brief 発動効果を付与する鍛冶効果の情報クラス
 * 発動効果を付与する（かつ、特性フラグを与える場合もある）鍛冶の情報を扱う
 */
class activation_smith_info : public basic_smith_info {
public:
    activation_smith_info(SmithEffect effect, concptr name, SmithCategory category, std::vector<SmithEssence> need_essences, int consumption, TrFlags add_flags, random_art_activation_type act_idx);
    virtual TrFlags tr_flags() const override;
    virtual std::optional<random_art_activation_type> activation_index() const override;

private:
    random_art_activation_type act_idx; //!< 発動能力ID
};

/*!
 * @brief 武器強化を行う鍛冶情報のクラス
 * 武器の命中/ダメージ修正を強化する。鍛冶効果とは別枠で行うので鍛冶師の銘付きにはならない。
 */
class enchant_weapon_smith_info : public smith_info_base {
public:
    enchant_weapon_smith_info(SmithEffect effect, concptr name, SmithCategory category, std::vector<SmithEssence> need_essences, int consumption);
    virtual bool add_essence(player_type *player_ptr, object_type *o_ptr, int number) const override;
    virtual void erase_essence(object_type *) const override {}
};

/*!
 * @brief 防具強化を行う鍛冶情報のクラス
 * 防具のAC修正を強化する。鍛冶効果とは別枠で行うので鍛冶師の銘付きにはならない。
 */
class enchant_armour_smith_info : public smith_info_base {
public:
    enchant_armour_smith_info(SmithEffect effect, concptr name, SmithCategory category, std::vector<SmithEssence> need_essences, int consumption);
    virtual bool add_essence(player_type *player_ptr, object_type *o_ptr, int number) const override;
    virtual void erase_essence(object_type *) const override {}
};

/*!
 * @brief 装備保持を行う鍛冶情報のクラス
 * 四元素属性で破壊されないフラグを付与する。鍛冶効果とは別枠で行うので鍛冶師の銘付きにはならない。
 */
class sustain_smith_info : public smith_info_base {
public:
    sustain_smith_info(SmithEffect effect, concptr name, SmithCategory category, std::vector<SmithEssence> need_essences, int consumption);
    virtual bool add_essence(player_type *player_ptr, object_type *o_ptr, int number) const override;
    virtual void erase_essence(object_type *) const override {}
};

/*!
 * @brief 殺戮の小手を作成する鍛冶情報のクラス
 * 小手に殺戮修正を付ける。(もともと殺戮修正のある小手は修正をさらに強化する)
 */
class slaying_gloves_smith_info : public basic_smith_info {
public:
    slaying_gloves_smith_info(SmithEffect effect, concptr name, SmithCategory category, std::vector<SmithEssence> need_essences, int consumption);
    virtual bool add_essence(player_type *player_ptr, object_type *o_ptr, int number) const override;
    virtual void erase_essence(object_type *) const override;
};
