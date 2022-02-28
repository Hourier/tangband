﻿#include "combat/hallucination-attacks-table.h"

/*!
 * @brief 幻覚時の打撃記述テーブル / Weird melee attack types when hallucinating
 */
#ifdef JP
const concptr silly_attacks[MAX_SILLY_ATTACK] = {
    "に小便をかけられた。",
    "があなたの回りを3回回ってワンと言った。",
    "にしばきまわされた。",
    "に靴をなめさせられた。",
    "にハナクソを飛ばされた。",
    "にジャン拳で攻撃された。",
    "があなたの頬を札束でしばいた。",
    "があなたの前でポージングをした。",
    "にアカンベーされた。",
    "に「神の国」発言の撤回を求められた。",
    "にメッ○ールを飲まされた。",
    "につっこみを入れられた。",
    "はあなたと踊った。",
    "に顔にらく書きをされた。",
    "に借金の返済をせまられた。",
    "にスカートをめくられた。",
    "はあなたの手相を占った。",
    "から役満を上がられた。",
    "から愛の告白をされた。",
    "はあなたを時給500円で雇った。",
    "はあなたの100の秘密について熱く語った。",
    "がニャーと鳴いた。",
    "はあなたに気をつけた。",
    "はあなたをポリゴン化させた。",
    "に少しかじられた。",
    "はアルテマの呪文を唱えた！",
    "はあなたのスパイクをブロックした。",
    "はスライド移動した。",
    "は昇龍拳コマンドの入力に失敗した。",
    "は拡散波動砲を発射した。",
    "はデスラー戦法をしかけた。",
    "にライダーキックで攻撃された。",
    "に二週間以内でビデオを人に見せないと死ぬ呪いをかけられた。",
    "はパルプンテを唱えた。",
    "はスーパーウルトラギャラクティカマグナムを放った。",
    "にしゃがみ小キックでハメられた。",
    "にジェットストリームアタックをかけられた。",
    "はあなたに卍固めをかけて「1、2、3、ダーッ！」と叫んだ。",
    "は「いくじなし！ばかばかばか！」といって駆け出した。",
    "が「ごらん、ルーベンスの絵だよ」と言って静かに目を閉じた。",
    "は言った。「変愚蛮怒、絶賛公開中！」",
};

/*!
 * @brief 幻覚時の打撃記述テーブル(フォーマットつき) / Weird melee attack types when hallucinating (%s for strfmt())
 */
const concptr silly_attacks2[MAX_SILLY_ATTACK] = {
    "%sに小便をかけた。",
    "%sの回りを3回回ってワンと言った。",
    "%sをしばきまわした。",
    "%sに靴をなめさせた。",
    "%sにハナクソを飛ばした。",
    "%sをジャン拳で攻撃した。",
    "%sの頬を札束でしばいた。",
    "%sの前でポージングをした。",
    "%sにアカンベーした。",
    "%sに「神の国」発言の撤回を求めた。",
    "%sにメッ○ールを飲ませた。",
    "%sにつっこみを入れた。",
    "%sと踊った。",
    "%sの顔にらく書きをした。",
    "%sに借金の返済をせまった。",
    "%sのスカートをめくった。",
    "%sの手相を占った。",
    "%sから役満を上がった。",
    "%sに愛の告白をした。",
    "%sを時給500円で雇った。",
    "%sの100の秘密について熱く語った。",
    "ニャーと鳴いた。",
    "%sに気をつけた。",
    "%sをポリゴン化させた。",
    "%sを少しかじった。",
    "アルテマの呪文を唱えた！",
    "%sのスパイクをブロックした。",
    "スライド移動した。",
    "昇龍拳コマンドの入力に失敗した。",
    "%sに拡散波動砲を発射した。",
    "%sにデスラー戦法をしかけた。",
    "%sをライダーキックで攻撃した。",
    "%sに二週間以内でビデオを人に見せないと死ぬ呪いをかけた。",
    "パルプンテを唱えた。",
    "%sにスーパーウルトラギャラクティカマグナムを放った。",
    "%sをしゃがみ小キックでハメた。",
    "%sにジェットストリームアタックをかけた。",
    "%sに卍固めをかけて「1、2、3、ダーッ！」と叫んだ。",
    "「いくじなし！ばかばかばか！」といって駆け出した。",
    "「ごらん、ルーベンスの絵だよ」と言って静かに目を閉じた。",
    "言った。「変愚蛮怒、絶賛公開中！」",
};
#else
const concptr silly_attacks[MAX_SILLY_ATTACK] = {
    "smothers",
    "hugs",
    "humiliates",
    "whips",
    "kisses",

    "disgusts",
    "pees all over",
    "passes the gas on",
    "makes obscene gestures at",
    "licks",

    "stomps on",
    "swallows",
    "drools on",
    "misses",
    "shrinks",

    "emasculates",
    "evaporates",
    "solidifies",
    "digitizes",
    "insta-kills",

    "massacres",
    "slaughters",
    "drugs",
    "psychoanalyzes",
    "deconstructs",

    "falsifies",
    "disbelieves",
    "molests",
    "pusupusu",
};
#endif
