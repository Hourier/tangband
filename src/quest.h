﻿/*
 * Quest constants
 */
#define MIN_RANDOM_QUEST 40 /*<! ランダムクエストを割り当てるクエストIDの開始値 */
#define MAX_RANDOM_QUEST 49 /*<! ランダムクエストを割り当てるクエストIDの終了値 */

/*
 * Quest status
 */
#define QUEST_STATUS_UNTAKEN              0 /*!< クエストステータス状態：未発生*/
#define QUEST_STATUS_TAKEN                1 /*!< クエストステータス状態：発生中*/
#define QUEST_STATUS_COMPLETED            2 /*!< クエストステータス状態：達成*/
#define QUEST_STATUS_REWARDED             3 /*!< クエストステータス状態：報酬受け取り前*/
#define QUEST_STATUS_FINISHED             4 /*!< クエストステータス状態：完了*/
#define QUEST_STATUS_FAILED               5 /*!< クエストステータス状態：失敗*/
#define QUEST_STATUS_FAILED_DONE          6 /*!< クエストステータス状態：失敗完了*/
#define QUEST_STATUS_STAGE_COMPLETED      7 /*!< クエストステータス状態：ステージ毎達成*/

/*
 * Quest type
 */
#define QUEST_TYPE_KILL_LEVEL                1 /*!< クエスト目的: 特定のユニークモンスターを倒す */
#define QUEST_TYPE_KILL_ANY_LEVEL            2 /*!< クエスト目的: イベント受託時点でランダムで選ばれた特定のユニークモンスターを倒す */
#define QUEST_TYPE_FIND_ARTIFACT             3 /*!< クエスト目的: 特定のアーティファクトを発見する */
#define QUEST_TYPE_FIND_EXIT                 4 /*!< クエスト目的: 脱出する */
#define QUEST_TYPE_KILL_NUMBER               5 /*!< クエスト目的: モンスターを無差別に特定数倒す */
#define QUEST_TYPE_KILL_ALL                  6 /*!< クエスト目的: エリア中のすべてのモンスターを全て倒す */
#define QUEST_TYPE_RANDOM                    7 /*!< クエスト目的: ランダムクエストとして選ばれたユニーク1体を倒す */
#define QUEST_TYPE_TOWER                     8 /*!< クエスト目的: 複数のエリアの全てのモンスターを倒す */

/*
 * Quest flags
 */
#define QUEST_FLAG_SILENT  0x01 /*!< クエストフラグ: クエスト進行に関する情報表示を抑止する / no messages from completion */
#define QUEST_FLAG_PRESET  0x02 /*!< クエストフラグ: クエストがダンジョン外で発生する / quest is outside the main dungeon */
#define QUEST_FLAG_ONCE    0x04 /*!< クエストフラグ: クエストがフロアを出た時点で完了する / quest is marked finished after leaving */
#define QUEST_FLAG_TOWER   0x08 /*!< クエストフラグ: クエスト:塔の形式で進行する / Tower quest is special */

/*!
 * @brief 該当IDが固定クエストかどうかを判定する / Check is the quest index is "fixed"
 * @param Q_IDX クエストID
 * @return 固定クエストならばTRUEを返す
 */
#define is_fixed_quest_idx(Q_IDX) (((Q_IDX) < MIN_RANDOM_QUEST) || ((Q_IDX) > MAX_RANDOM_QUEST))

#define QUEST_TOWER1 5 /*<! 塔クエスト(第1階層)に割り振るクエストID */
#define QUEST_TOWER2 6 /*<! 塔クエスト(第2階層)に割り振るクエストID */
#define QUEST_TOWER3 7 /*<! 塔クエスト(第3階層)に割り振るクエストID */
#define QUEST_OBERON 8 /*<! オベロン打倒クエストに割り振るクエストID */
#define QUEST_SERPENT 9 /*<! サーペント打倒クエストに割り振るクエストID */


extern void determine_random_questor(quest_type *q_ptr);
extern void complete_quest(QUEST_IDX quest_num);
extern void check_quest_completion(monster_type *m_ptr);
extern void check_find_art_quest_completion(object_type *o_ptr);
extern void quest_discovery(QUEST_IDX q_idx);
extern QUEST_IDX quest_number(DEPTH level);
extern QUEST_IDX random_quest_number(DEPTH level);
extern void leave_quest_check(void);
extern void leave_tower_check(void);
extern void do_cmd_quest(void);