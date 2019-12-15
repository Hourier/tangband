﻿/*!
 * @file monster-process.c
 * @brief モンスターの特殊技能と移動処理/ Monster spells and movement
 * @date 2014/01/17
 * @author
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke\n
 * This software may be copied and distributed for educational, research,\n
 * and not for profit purposes provided that this copyright and statement\n
 * are included in all such copies.  Other copyrights may also apply.\n
 * 2014 Deskull rearranged comment for Doxygen.\n
 * @details
 * This file has several additions to it by Keldon Jones (keldon@umr.edu)
 * to improve the general quality of the AI (version 0.1.1).
 */

#include "angband.h"
#include "util.h"

#include "cmd-dump.h"
#include "cmd-pet.h"
#include "creature.h"
#include "melee.h"
#include "spells.h"
#include "spells-floor.h"
#include "spells-summon.h"
#include "quest.h"
#include "avatar.h"
#include "realm-hex.h"
#include "object-flavor.h"
#include "object-hook.h"
#include "feature.h"
#include "grid.h"
#include "player-move.h"
#include "monster-status.h"
#include "monster-spell.h"
#include "monster-process.h"
#include "monsterrace-hook.h"
#include "dungeon.h"
#include "floor.h"
#include "files.h"
#include "view-mainwindow.h"


 /*!
  * @brief モンスターが敵に接近するための方向を決める /
  * Calculate the direction to the next enemy
  * @param target_ptr プレーヤーへの参照ポインタ
  * @param m_idx モンスターの参照ID
  * @param mm 移動するべき方角IDを返す参照ポインタ
  * @return 方向が確定した場合TRUE、接近する敵がそもそもいない場合FALSEを返す
  */
static bool get_enemy_dir(player_type *target_ptr, MONSTER_IDX m_idx, int *mm)
{
	floor_type *floor_ptr = target_ptr->current_floor_ptr;
	monster_type *m_ptr = &floor_ptr->m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_type *t_ptr;

	POSITION x = 0, y = 0;
	if (target_ptr->riding_t_m_idx && player_bold(target_ptr, m_ptr->fy, m_ptr->fx))
	{
		y = floor_ptr->m_list[target_ptr->riding_t_m_idx].fy;
		x = floor_ptr->m_list[target_ptr->riding_t_m_idx].fx;
	}
	else if (is_pet(m_ptr) && target_ptr->pet_t_m_idx)
	{
		y = floor_ptr->m_list[target_ptr->pet_t_m_idx].fy;
		x = floor_ptr->m_list[target_ptr->pet_t_m_idx].fx;
	}
	else
	{
		int start;
		int plus = 1;
		if (target_ptr->phase_out)
		{
			start = randint1(floor_ptr->m_max - 1) + floor_ptr->m_max;
			if (randint0(2)) plus = -1;
		}
		else start = floor_ptr->m_max + 1;

		/* Scan thru all monsters */
		for (int i = start; ((i < start + floor_ptr->m_max) && (i > start - floor_ptr->m_max)); i += plus)
		{
			MONSTER_IDX dummy = (i % floor_ptr->m_max);

			if (!dummy) continue;

			MONSTER_IDX t_idx = dummy;
			t_ptr = &floor_ptr->m_list[t_idx];

			/* The monster itself isn't a target */
			if (t_ptr == m_ptr) continue;

			if (!monster_is_valid(t_ptr)) continue;

			if (is_pet(m_ptr))
			{
				/* Hack -- only fight away from player */
				if (target_ptr->pet_follow_distance < 0)
				{
					/* No fighting near player */
					if (t_ptr->cdis <= (0 - target_ptr->pet_follow_distance))
					{
						continue;
					}
				}
				/* Hack -- no fighting away from player */
				else if ((m_ptr->cdis < t_ptr->cdis) && (t_ptr->cdis > target_ptr->pet_follow_distance))
				{
					continue;
				}

				if (r_ptr->aaf < t_ptr->cdis) continue;
			}

			/* Monster must be 'an enemy' */
			if (!are_enemies(target_ptr, m_ptr, t_ptr)) continue;

			/* Monster must be projectable if we can't pass through walls */
			if (((r_ptr->flags2 & RF2_PASS_WALL) && ((m_idx != target_ptr->riding) || target_ptr->pass_wall)) ||
				((r_ptr->flags2 & RF2_KILL_WALL) && (m_idx != target_ptr->riding)))
			{
				if (!in_disintegration_range(floor_ptr, m_ptr->fy, m_ptr->fx, t_ptr->fy, t_ptr->fx)) continue;
			}
			else
			{
				if (!projectable(target_ptr, m_ptr->fy, m_ptr->fx, t_ptr->fy, t_ptr->fx)) continue;
			}

			/* OK -- we've got a target */
			y = t_ptr->fy;
			x = t_ptr->fx;

			break;
		}

		if (!x && !y) return FALSE;
	}

	/* Extract the direction */
	x -= m_ptr->fx;
	y -= m_ptr->fy;

	/* North */
	if ((y < 0) && (x == 0))
	{
		mm[0] = 8;
		mm[1] = 7;
		mm[2] = 9;
	}
	/* South */
	else if ((y > 0) && (x == 0))
	{
		mm[0] = 2;
		mm[1] = 1;
		mm[2] = 3;
	}
	/* East */
	else if ((x > 0) && (y == 0))
	{
		mm[0] = 6;
		mm[1] = 9;
		mm[2] = 3;
	}
	/* West */
	else if ((x < 0) && (y == 0))
	{
		mm[0] = 4;
		mm[1] = 7;
		mm[2] = 1;
	}
	/* North-West */
	else if ((y < 0) && (x < 0))
	{
		mm[0] = 7;
		mm[1] = 4;
		mm[2] = 8;
	}
	/* North-East */
	else if ((y < 0) && (x > 0))
	{
		mm[0] = 9;
		mm[1] = 6;
		mm[2] = 8;
	}
	/* South-West */
	else if ((y > 0) && (x < 0))
	{
		mm[0] = 1;
		mm[1] = 4;
		mm[2] = 2;
	}
	/* South-East */
	else if ((y > 0) && (x > 0))
	{
		mm[0] = 3;
		mm[1] = 6;
		mm[2] = 2;
	}

	return TRUE;
}

/*!
 * @brief モンスターがプレイヤーから逃走するかどうかを返す /
 * Returns whether a given monster will try to run from the player.
 * @param m_idx 逃走するモンスターの参照ID
 * @return モンスターがプレイヤーから逃走するならばTRUEを返す。
 * @details
 * Monsters will attempt to avoid very powerful players.  See below.\n
 *\n
 * Because this function is called so often, little details are important\n
 * for efficiency.  Like not using "mod" or "div" when possible.  And\n
 * attempting to check the conditions in an optimal order.  Note that\n
 * "(x << 2) == (x * 4)" if "x" has enough bits to hold the result.\n
 *\n
 * Note that this function is responsible for about one to five percent\n
 * of the processor use in normal conditions...\n
 */
static bool mon_will_run(player_type *target_ptr, MONSTER_IDX m_idx)
{
	monster_type *m_ptr = &target_ptr->current_floor_ptr->m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	PLAYER_LEVEL p_lev;
	DEPTH m_lev;
	HIT_POINT p_chp, p_mhp;
	HIT_POINT m_chp, m_mhp;
	u32b p_val, m_val;

	/* Friends can be commanded to avoid the player */
	if (is_pet(m_ptr))
	{
		/* Are we trying to avoid the player? */
		return ((target_ptr->pet_follow_distance < 0) &&
			(m_ptr->cdis <= (0 - target_ptr->pet_follow_distance)));
	}

	/* Keep monsters from running too far away */
	if (m_ptr->cdis > MAX_SIGHT + 5) return FALSE;

	/* All "afraid" monsters will run away */
	if (MON_MONFEAR(m_ptr)) return TRUE;

	/* Nearby monsters will not become terrified */
	if (m_ptr->cdis <= 5) return FALSE;

	/* Examine player power (level) */
	p_lev = target_ptr->lev;

	/* Examine monster power (level plus morale) */
	m_lev = r_ptr->level + (m_idx & 0x08) + 25;

	/* Optimize extreme cases below */
	if (m_lev > p_lev + 4) return FALSE;
	if (m_lev + 4 <= p_lev) return TRUE;

	/* Examine player health */
	p_chp = target_ptr->chp;
	p_mhp = target_ptr->mhp;

	/* Examine monster health */
	m_chp = m_ptr->hp;
	m_mhp = m_ptr->maxhp;

	/* Prepare to optimize the calculation */
	p_val = (p_lev * p_mhp) + (p_chp << 2); /* div p_mhp */
	m_val = (m_lev * m_mhp) + (m_chp << 2); /* div m_mhp */

	/* Strong players scare strong monsters */
	if (p_val * m_mhp > m_val * p_mhp) return TRUE;

	return FALSE;
}


/*!
 * @brief モンスターがプレイヤーに向けて遠距離攻撃を行うことが可能なマスを走査する /
 * Search spell castable grid
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param m_idx モンスターの参照ID
 * @param yp 適したマスのY座標を返す参照ポインタ
 * @param xp 適したマスのX座標を返す参照ポインタ
 * @return 有効なマスがあった場合TRUEを返す
 */
static bool get_moves_aux2(player_type *target_ptr, MONSTER_IDX m_idx, POSITION *yp, POSITION *xp)
{
	floor_type *floor_ptr = target_ptr->current_floor_ptr;
	monster_type *m_ptr = &floor_ptr->m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Monster location */
	POSITION y1 = m_ptr->fy;
	POSITION x1 = m_ptr->fx;

	/* Monster can already cast spell to player */
	if (projectable(target_ptr, y1, x1, target_ptr->y, target_ptr->x)) return FALSE;

	/* Set current grid cost */
	int now_cost = floor_ptr->grid_array[y1][x1].cost;
	if (now_cost == 0) now_cost = 999;

	/* Can monster bash or open doors? */
	bool can_open_door = FALSE;
	if (r_ptr->flags2 & (RF2_BASH_DOOR | RF2_OPEN_DOOR))
	{
		can_open_door = TRUE;
	}

	/* Check nearby grids, diagonals first */
	int best = 999;
	for (int i = 7; i >= 0; i--)
	{
		int cost;

		POSITION y = y1 + ddy_ddd[i];
		POSITION x = x1 + ddx_ddd[i];

		/* Ignore locations off of edge */
		if (!in_bounds2(floor_ptr, y, x)) continue;

		/* Simply move to player */
		if (player_bold(target_ptr, y, x)) return FALSE;

		grid_type *g_ptr;
		g_ptr = &floor_ptr->grid_array[y][x];

		cost = g_ptr->cost;

		/* Monster cannot kill or pass walls */
		if (!(((r_ptr->flags2 & RF2_PASS_WALL) && ((m_idx != target_ptr->riding) || target_ptr->pass_wall)) || ((r_ptr->flags2 & RF2_KILL_WALL) && (m_idx != target_ptr->riding))))
		{
			if (cost == 0) continue;
			if (!can_open_door && is_closed_door(target_ptr, g_ptr->feat)) continue;
		}

		/* Hack -- for kill or pass wall monster.. */
		if (cost == 0) cost = 998;

		if (now_cost < cost) continue;

		if (!projectable(target_ptr, y, x, target_ptr->y, target_ptr->x)) continue;

		/* Accept louder sounds */
		if (best < cost) continue;
		best = cost;

		(*yp) = y1 + ddy_ddd[i];
		(*xp) = x1 + ddx_ddd[i];
	}

	if (best == 999) return FALSE;

	return TRUE;
}


/*!
 * @brief モンスターがプレイヤーに向けて接近することが可能なマスを走査する /
 * Choose the "best" direction for "flowing"
 * @param m_idx モンスターの参照ID
 * @param yp 移動先のマスのY座標を返す参照ポインタ
 * @param xp 移動先のマスのX座標を返す参照ポインタ
 * @param no_flow モンスターにFLOWフラグが経っていない状態でTRUE
 * @return 有効なマスがあった場合TRUEを返す
 * @details
 * Note that ghosts and rock-eaters are never allowed to "flow",\n
 * since they should move directly towards the player.\n
 *\n
 * Prefer "non-diagonal" directions, but twiddle them a little\n
 * to angle slightly towards the player's actual location.\n
 *\n
 * Allow very perceptive monsters to track old "spoor" left by\n
 * previous locations occupied by the player.  This will tend\n
 * to have monsters end up either near the player or on a grid\n
 * recently occupied by the player (and left via "teleport").\n
 *\n
 * Note that if "smell" is turned on, all monsters get vicious.\n
 *\n
 * Also note that teleporting away from a location will cause\n
 * the monsters who were chasing you to converge on that location\n
 * as long as you are still near enough to "annoy" them without\n
 * being close enough to chase directly.  I have no idea what will\n
 * happen if you combine "smell" with low "aaf" values.\n
 */
static bool get_moves_aux(player_type *target_ptr, MONSTER_IDX m_idx, POSITION *yp, POSITION *xp, bool no_flow)
{
	grid_type *g_ptr;
	floor_type *floor_ptr = target_ptr->current_floor_ptr;
	monster_type *m_ptr = &floor_ptr->m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Can monster cast attack spell? */
	if (r_ptr->flags4 & (RF4_ATTACK_MASK) ||
		r_ptr->a_ability_flags1 & (RF5_ATTACK_MASK) ||
		r_ptr->a_ability_flags2 & (RF6_ATTACK_MASK))
	{
		/* Can move spell castable grid? */
		if (get_moves_aux2(target_ptr, m_idx, yp, xp)) return TRUE;
	}

	/* Monster can't flow */
	if (no_flow) return FALSE;

	/* Monster can go through rocks */
	if ((r_ptr->flags2 & RF2_PASS_WALL) && ((m_idx != target_ptr->riding) || target_ptr->pass_wall)) return FALSE;
	if ((r_ptr->flags2 & RF2_KILL_WALL) && (m_idx != target_ptr->riding)) return FALSE;

	/* Monster location */
	POSITION y1 = m_ptr->fy;
	POSITION x1 = m_ptr->fx;

	/* Hack -- Player can see us, run towards him */
	if (player_has_los_bold(target_ptr, y1, x1) && projectable(target_ptr, target_ptr->y, target_ptr->x, y1, x1)) return FALSE;

	/* Monster grid */
	g_ptr = &floor_ptr->grid_array[y1][x1];

	/* If we can hear noises, advance towards them */
	int best;
	bool use_scent = FALSE;
	if (g_ptr->cost)
	{
		best = 999;
	}

	/* Otherwise, try to follow a scent trail */
	else if (g_ptr->when)
	{
		/* Too old smell */
		if (floor_ptr->grid_array[target_ptr->y][target_ptr->x].when - g_ptr->when > 127) return FALSE;

		use_scent = TRUE;
		best = 0;
	}

	/* Otherwise, advance blindly */
	else
	{
		return FALSE;
	}

	/* Check nearby grids, diagonals first */
	for (int i = 7; i >= 0; i--)
	{
		POSITION y = y1 + ddy_ddd[i];
		POSITION x = x1 + ddx_ddd[i];

		/* Ignore locations off of edge */
		if (!in_bounds2(floor_ptr, y, x)) continue;

		g_ptr = &floor_ptr->grid_array[y][x];

		/* We're following a scent trail */
		if (use_scent)
		{
			int when = g_ptr->when;

			/* Accept younger scent */
			if (best > when) continue;
			best = when;
		}

		/* We're using sound */
		else
		{
			int cost;

			if (r_ptr->flags2 & (RF2_BASH_DOOR | RF2_OPEN_DOOR))
				cost = g_ptr->dist;
			else cost = g_ptr->cost;

			/* Accept louder sounds */
			if ((cost == 0) || (best < cost)) continue;
			best = cost;
		}

		/* Hack -- Save the "twiddled" location */
		(*yp) = target_ptr->y + 16 * ddy_ddd[i];
		(*xp) = target_ptr->x + 16 * ddx_ddd[i];
	}

	if (best == 999 || best == 0) return FALSE;

	return TRUE;
}


/*!
 * @brief モンスターがプレイヤーから逃走することが可能なマスを走査する /
 * Provide a location to flee to, but give the player a wide berth.
 * @param m_idx モンスターの参照ID
 * @param yp 移動先のマスのY座標を返す参照ポインタ
 * @param xp 移動先のマスのX座標を返す参照ポインタ
 * @return 有効なマスがあった場合TRUEを返す
 * @details
 * A monster may wish to flee to a location that is behind the player,\n
 * but instead of heading directly for it, the monster should "swerve"\n
 * around the player so that he has a smaller chance of getting hit.\n
 */
static bool get_fear_moves_aux(floor_type *floor_ptr, MONSTER_IDX m_idx, POSITION *yp, POSITION *xp)
{
	POSITION gy = 0, gx = 0;

	/* Monster location */
	monster_type *m_ptr = &floor_ptr->m_list[m_idx];
	POSITION fy = m_ptr->fy;
	POSITION fx = m_ptr->fx;

	/* Desired destination */
	POSITION y1 = fy - (*yp);
	POSITION x1 = fx - (*xp);

	/* Check nearby grids, diagonals first */
	int score = -1;
	for (int i = 7; i >= 0; i--)
	{
		POSITION dis, s;
		POSITION y = fy + ddy_ddd[i];
		POSITION x = fx + ddx_ddd[i];

		/* Ignore locations off of edge */
		if (!in_bounds2(floor_ptr, y, x)) continue;

		/* Calculate distance of this grid from our destination */
		dis = distance(y, x, y1, x1);

		/* Score this grid */
		s = 5000 / (dis + 3) - 500 / (floor_ptr->grid_array[y][x].dist + 1);

		/* No negative scores */
		if (s < 0) s = 0;

		/* Ignore lower scores */
		if (s < score) continue;

		/* Save the score and time */
		score = s;

		/* Save the location */
		gy = y;
		gx = x;
	}

	/* No legal move (?) */
	if (score == -1) return FALSE;

	/* Find deltas */
	(*yp) = fy - gy;
	(*xp) = fx - gx;

	/* Success */
	return TRUE;
}

/*
 * Hack -- Precompute a bunch of calls to distance() in find_safety() and
 * find_hiding().
 *
 * The pair of arrays dist_offsets_y[n] and dist_offsets_x[n] contain the
 * offsets of all the locations with a distance of n from a central point,
 * with an offset of (0,0) indicating no more offsets at this distance.
 *
 * This is, of course, fairly unreadable, but it eliminates multiple loops
 * from the previous version.
 *
 * It is probably better to replace these arrays with code to compute
 * the relevant arrays, even if the storage is pre-allocated in hard
 * coded sizes.  At the very least, code should be included which is
 * able to generate and dump these arrays (ala "los()").
 *
 * Also, the storage needs could be halved by using bytes.
 *
 * These arrays could be combined into two big arrays, using sub-arrays
 * to hold the offsets and lengths of each portion of the sub-arrays, and
 * this could perhaps also be used somehow in the "look" code.
 */


static POSITION d_off_y_0[] = { 0 };
static POSITION d_off_x_0[] = { 0 };

static POSITION d_off_y_1[] = { -1, -1, -1, 0, 0, 1, 1, 1, 0 };
static POSITION d_off_x_1[] = { -1, 0, 1, -1, 1, -1, 0, 1, 0 };

static POSITION d_off_y_2[] = { -1, -1, -2, -2, -2, 0, 0, 1, 1, 2, 2, 2, 0 };
static POSITION d_off_x_2[] = { -2, 2, -1, 0, 1, -2, 2, -2, 2, -1, 0, 1, 0 };

static POSITION d_off_y_3[] = { -1, -1, -2, -2, -3, -3, -3, 0, 0, 1, 1, 2, 2, 3, 3, 3, 0 };
static POSITION d_off_x_3[] = { -3, 3, -2, 2, -1, 0, 1, -3, 3, -3, 3, -2, 2, -1, 0, 1, 0 };

static POSITION d_off_y_4[] = { -1, -1, -2, -2, -3, -3, -3, -3, -4, -4, -4, 0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 0 };
static POSITION d_off_x_4[] = { -4, 4, -3, 3, -2, -3, 2, 3, -1, 0, 1, -4, 4, -4, 4, -3, 3, -2, -3, 2, 3, -1, 0, 1, 0 };


static POSITION d_off_y_5[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -4, -4, -5, -5,
  -5, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 5, 5,
  5, 0 };

static POSITION d_off_x_5[] =
{ -5, 5, -4, 4, -4, 4, -2, -3, 2, 3, -1, 0, 1,
  -5, 5, -5, 5, -4, 4, -4, 4, -2, -3, 2, 3, -1,
  0, 1, 0 };


static POSITION d_off_y_6[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -5, -5,
  -6, -6, -6, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5,
  5, 5, 6, 6, 6, 0 };

static POSITION d_off_x_6[] =
{ -6, 6, -5, 5, -5, 5, -4, 4, -2, -3, 2, 3, -1,
  0, 1, -6, 6, -6, 6, -5, 5, -5, 5, -4, 4, -2,
  -3, 2, 3, -1, 0, 1, 0 };


static POSITION d_off_y_7[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -5, -5,
  -6, -6, -6, -6, -7, -7, -7, 0, 0, 1, 1, 2, 2, 3,
  3, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 0 };

static POSITION d_off_x_7[] =
{ -7, 7, -6, 6, -6, 6, -5, 5, -4, -5, 4, 5, -2,
  -3, 2, 3, -1, 0, 1, -7, 7, -7, 7, -6, 6, -6,
  6, -5, 5, -4, -5, 4, 5, -2, -3, 2, 3, -1, 0,
  1, 0 };


static POSITION d_off_y_8[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6,
  -6, -6, -7, -7, -7, -7, -8, -8, -8, 0, 0, 1, 1,
  2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
  8, 8, 8, 0 };

static POSITION d_off_x_8[] =
{ -8, 8, -7, 7, -7, 7, -6, 6, -6, 6, -4, -5, 4,
  5, -2, -3, 2, 3, -1, 0, 1, -8, 8, -8, 8, -7,
  7, -7, 7, -6, 6, -6, 6, -4, -5, 4, 5, -2, -3,
  2, 3, -1, 0, 1, 0 };


static POSITION d_off_y_9[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6,
  -7, -7, -7, -7, -8, -8, -8, -8, -9, -9, -9, 0,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7,
  7, 8, 8, 8, 8, 9, 9, 9, 0 };

static POSITION d_off_x_9[] =
{ -9, 9, -8, 8, -8, 8, -7, 7, -7, 7, -6, 6, -4,
  -5, 4, 5, -2, -3, 2, 3, -1, 0, 1, -9, 9, -9,
  9, -8, 8, -8, 8, -7, 7, -7, 7, -6, 6, -4, -5,
  4, 5, -2, -3, 2, 3, -1, 0, 1, 0 };


static POSITION *dist_offsets_y[10] =
{
	d_off_y_0, d_off_y_1, d_off_y_2, d_off_y_3, d_off_y_4,
	d_off_y_5, d_off_y_6, d_off_y_7, d_off_y_8, d_off_y_9
};

static POSITION *dist_offsets_x[10] =
{
	d_off_x_0, d_off_x_1, d_off_x_2, d_off_x_3, d_off_x_4,
	d_off_x_5, d_off_x_6, d_off_x_7, d_off_x_8, d_off_x_9
};

/*!
 * @brief モンスターが逃げ込める安全な地点を返す /
 * Choose a "safe" location near a monster for it to run toward.
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param m_idx モンスターの参照ID
 * @param yp 移動先のマスのY座標を返す参照ポインタ
 * @param xp 移動先のマスのX座標を返す参照ポインタ
 * @return 有効なマスがあった場合TRUEを返す
 * @details
 * A location is "safe" if it can be reached quickly and the player\n
 * is not able to fire into it (it isn't a "clean shot").  So, this will\n
 * cause monsters to "duck" behind walls.  Hopefully, monsters will also\n
 * try to run towards corridor openings if they are in a room.\n
 *\n
 * This function may take lots of CPU time if lots of monsters are\n
 * fleeing.\n
 *\n
 * Return TRUE if a safe location is available.\n
 */
static bool find_safety(player_type *target_ptr, MONSTER_IDX m_idx, POSITION *yp, POSITION *xp)
{
	floor_type *floor_ptr = target_ptr->current_floor_ptr;
	monster_type *m_ptr = &floor_ptr->m_list[m_idx];

	POSITION fy = m_ptr->fy;
	POSITION fx = m_ptr->fx;

	POSITION gy = 0, gx = 0, gdis = 0;

	/* Start with adjacent locations, spread further */
	for (POSITION d = 1; d < 10; d++)
	{
		/* Get the lists of points with a distance d from (fx, fy) */
		POSITION *y_offsets;
		y_offsets = dist_offsets_y[d];

		POSITION *x_offsets;
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (POSITION i = 0, dx = x_offsets[0], dy = y_offsets[0];
			dx != 0 || dy != 0;
			i++, dx = x_offsets[i], dy = y_offsets[i])
		{
			POSITION y = fy + dy;
			POSITION x = fx + dx;

			/* Skip illegal locations */
			if (!in_bounds(floor_ptr, y, x)) continue;

			grid_type *g_ptr;
			g_ptr = &floor_ptr->grid_array[y][x];

			/* Skip locations in a wall */
			if (!monster_can_cross_terrain(target_ptr, g_ptr->feat, &r_info[m_ptr->r_idx], (m_idx == target_ptr->riding) ? CEM_RIDING : 0)) continue;

			/* Check for "availability" (if monsters can flow) */
			if (!(m_ptr->mflag2 & MFLAG2_NOFLOW))
			{
				/* Ignore grids very far from the player */
				if (g_ptr->dist == 0) continue;

				/* Ignore too-distant grids */
				if (g_ptr->dist > floor_ptr->grid_array[fy][fx].dist + 2 * d) continue;
			}

			/* Check for absence of shot (more or less) */
			if (projectable(target_ptr, target_ptr->y, target_ptr->x, y, x)) continue;

			/* Calculate distance from player */
			POSITION dis = distance(y, x, target_ptr->y, target_ptr->x);

			/* Remember if further than previous */
			if (dis <= gdis) continue;

			gy = y;
			gx = x;
			gdis = dis;
		}

		/* Check for success */
		if (gdis <= 0) continue;

		/* Good location */
		(*yp) = fy - gy;
		(*xp) = fx - gx;

		return TRUE;
	}

	return FALSE;
}


/*!
 * @brief モンスターが隠れ潜める地点を返す /
 * Choose a good hiding place near a monster for it to run toward.
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param m_idx モンスターの参照ID
 * @param yp 移動先のマスのY座標を返す参照ポインタ
 * @param xp 移動先のマスのX座標を返す参照ポインタ
 * @return 有効なマスがあった場合TRUEを返す
 * @details
 * Pack monsters will use this to "ambush" the player and lure him out\n
 * of corridors into open space so they can swarm him.\n
 *\n
 * Return TRUE if a good location is available.\n
 */
static bool find_hiding(player_type *target_ptr, MONSTER_IDX m_idx, POSITION *yp, POSITION *xp)
{
	floor_type *floor_ptr = target_ptr->current_floor_ptr;
	monster_type *m_ptr = &floor_ptr->m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	POSITION fy = m_ptr->fy;
	POSITION fx = m_ptr->fx;

	POSITION gy = 0, gx = 0, gdis = 999;

	/* Start with adjacent locations, spread further */
	for (POSITION d = 1; d < 10; d++)
	{
		/* Get the lists of points with a distance d from (fx, fy) */
		POSITION *y_offsets;
		y_offsets = dist_offsets_y[d];

		POSITION *x_offsets;
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (POSITION i = 0, dx = x_offsets[0], dy = y_offsets[0];
			dx != 0 || dy != 0;
			i++, dx = x_offsets[i], dy = y_offsets[i])
		{
			POSITION y = fy + dy;
			POSITION x = fx + dx;

			/* Skip illegal locations */
			if (!in_bounds(floor_ptr, y, x)) continue;

			/* Skip occupied locations */
			if (!monster_can_enter(target_ptr, y, x, r_ptr, 0)) continue;

			/* Check for hidden, available grid */
			if (projectable(target_ptr, target_ptr->y, target_ptr->x, y, x) && clean_shot(target_ptr, fy, fx, y, x, FALSE))
				continue;

			/* Calculate distance from player */
			POSITION dis = distance(y, x, target_ptr->y, target_ptr->x);

			/* Remember if closer than previous */
			if (dis < gdis && dis >= 2)
			{
				gy = y;
				gx = x;
				gdis = dis;
			}
		}

		if (gdis >= 999) continue;

		*yp = fy - gy;
		*xp = fx - gx;

		return TRUE;
	}

	return FALSE;
}


/*!
 * @brief モンスターの移動方向を返す /
 * Choose "logical" directions for monster movement
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param m_idx モンスターの参照ID
 * @param mm 移動方向を返す方向IDの参照ポインタ
 * @return 有効方向があった場合TRUEを返す
 */
static bool get_moves(player_type *target_ptr, MONSTER_IDX m_idx, DIRECTION *mm)
{
	floor_type *floor_ptr = target_ptr->current_floor_ptr;
	monster_type *m_ptr = &floor_ptr->m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	POSITION y = 0, ay, x = 0, ax;
	int move_val = 0;
	POSITION y2 = target_ptr->y;
	POSITION x2 = target_ptr->x;
	bool done = FALSE;
	bool will_run = mon_will_run(target_ptr, m_idx);
	grid_type *g_ptr;
	bool no_flow = ((m_ptr->mflag2 & MFLAG2_NOFLOW) && (floor_ptr->grid_array[m_ptr->fy][m_ptr->fx].cost > 2));
	bool can_pass_wall = ((r_ptr->flags2 & RF2_PASS_WALL) && ((m_idx != target_ptr->riding) || target_ptr->pass_wall));

	/* Counter attack to an enemy monster */
	if (!will_run && m_ptr->target_y)
	{
		int t_m_idx = floor_ptr->grid_array[m_ptr->target_y][m_ptr->target_x].m_idx;

		/* The monster must be an enemy, and in LOS */
		if (t_m_idx &&
			are_enemies(target_ptr, m_ptr, &floor_ptr->m_list[t_m_idx]) &&
			los(target_ptr, m_ptr->fy, m_ptr->fx, m_ptr->target_y, m_ptr->target_x) &&
			projectable(target_ptr, m_ptr->fy, m_ptr->fx, m_ptr->target_y, m_ptr->target_x))
		{
			/* Extract the "pseudo-direction" */
			y = m_ptr->fy - m_ptr->target_y;
			x = m_ptr->fx - m_ptr->target_x;
			done = TRUE;
		}
	}

	if (!done && !will_run && is_hostile(m_ptr) &&
		(r_ptr->flags1 & RF1_FRIENDS) &&
		((los(target_ptr, m_ptr->fy, m_ptr->fx, target_ptr->y, target_ptr->x) && projectable(target_ptr, m_ptr->fy, m_ptr->fx, target_ptr->y, target_ptr->x)) ||
		(floor_ptr->grid_array[m_ptr->fy][m_ptr->fx].dist < MAX_SIGHT / 2)))
	{
		/*
		 * Animal packs try to get the player out of corridors
		 * (...unless they can move through walls -- TY)
		 */
		if ((r_ptr->flags3 & RF3_ANIMAL) && !can_pass_wall &&
			!(r_ptr->flags2 & RF2_KILL_WALL))
		{
			int i, room = 0;

			/* Count room grids next to player */
			for (i = 0; i < 8; i++)
			{
				int xx = target_ptr->x + ddx_ddd[i];
				int yy = target_ptr->y + ddy_ddd[i];

				if (!in_bounds2(floor_ptr, yy, xx)) continue;

				g_ptr = &floor_ptr->grid_array[yy][xx];

				/* Check grid */
				if (monster_can_cross_terrain(target_ptr, g_ptr->feat, r_ptr, 0))
				{
					/* One more room grid */
					room++;
				}
			}
			if (floor_ptr->grid_array[target_ptr->y][target_ptr->x].info & CAVE_ROOM) room -= 2;
			if (!r_ptr->flags4 && !r_ptr->a_ability_flags1 && !r_ptr->a_ability_flags2) room -= 2;

			/* Not in a room and strong player */
			if (room < (8 * (target_ptr->chp + target_ptr->csp)) /
				(target_ptr->mhp + target_ptr->msp))
			{
				/* Find hiding place */
				if (find_hiding(target_ptr, m_idx, &y, &x)) done = TRUE;
			}
		}

		/* Monster groups try to surround the player */
		if (!done && (floor_ptr->grid_array[m_ptr->fy][m_ptr->fx].dist < 3))
		{
			int i;

			/* Find an empty square near the player to fill */
			for (i = 0; i < 8; i++)
			{
				/* Pick squares near player (semi-randomly) */
				y2 = target_ptr->y + ddy_ddd[(m_idx + i) & 7];
				x2 = target_ptr->x + ddx_ddd[(m_idx + i) & 7];

				/* Already there? */
				if ((m_ptr->fy == y2) && (m_ptr->fx == x2))
				{
					/* Attack the player */
					y2 = target_ptr->y;
					x2 = target_ptr->x;

					break;
				}

				if (!in_bounds2(floor_ptr, y2, x2)) continue;

				/* Ignore filled grids */
				if (!monster_can_enter(target_ptr, y2, x2, r_ptr, 0)) continue;

				/* Try to fill this hole */
				break;
			}

			/* Extract the new "pseudo-direction" */
			y = m_ptr->fy - y2;
			x = m_ptr->fx - x2;

			done = TRUE;
		}
	}

	if (!done)
	{
		/* Flow towards the player */
		(void)get_moves_aux(target_ptr, m_idx, &y2, &x2, no_flow);

		/* Extract the "pseudo-direction" */
		y = m_ptr->fy - y2;
		x = m_ptr->fx - x2;

		/* Not done */
	}

	/* Apply fear if possible and necessary */
	if (is_pet(m_ptr) && will_run)
	{
		/* XXX XXX Not very "smart" */
		y = (-y), x = (-x);
	}
	else
	{
		if (!done && will_run)
		{
			int tmp_x = (-x);
			int tmp_y = (-y);

			/* Try to find safe place */
			if (find_safety(target_ptr, m_idx, &y, &x))
			{
				/* Attempt to avoid the player */
				if (!no_flow)
				{
					/* Adjust movement */
					if (get_fear_moves_aux(target_ptr->current_floor_ptr, m_idx, &y, &x)) done = TRUE;
				}
			}

			if (!done)
			{
				/* This is not a very "smart" method XXX XXX */
				y = tmp_y;
				x = tmp_x;
			}
		}
	}


	/* Check for no move */
	if (!x && !y) return FALSE;


	/* Extract the "absolute distances" */
	ax = ABS(x);
	ay = ABS(y);

	/* Do something weird */
	if (y < 0) move_val += 8;
	if (x > 0) move_val += 4;

	/* Prevent the diamond maneuvre */
	if (ay > (ax << 1)) move_val += 2;
	else if (ax > (ay << 1)) move_val++;

	/* Extract some directions */
	switch (move_val)
	{
	case 0:
		mm[0] = 9;
		if (ay > ax)
		{
			mm[1] = 8;
			mm[2] = 6;
			mm[3] = 7;
			mm[4] = 3;
		}
		else
		{
			mm[1] = 6;
			mm[2] = 8;
			mm[3] = 3;
			mm[4] = 7;
		}
		break;
	case 1:
	case 9:
		mm[0] = 6;
		if (y < 0)
		{
			mm[1] = 3;
			mm[2] = 9;
			mm[3] = 2;
			mm[4] = 8;
		}
		else
		{
			mm[1] = 9;
			mm[2] = 3;
			mm[3] = 8;
			mm[4] = 2;
		}
		break;
	case 2:
	case 6:
		mm[0] = 8;
		if (x < 0)
		{
			mm[1] = 9;
			mm[2] = 7;
			mm[3] = 6;
			mm[4] = 4;
		}
		else
		{
			mm[1] = 7;
			mm[2] = 9;
			mm[3] = 4;
			mm[4] = 6;
		}
		break;
	case 4:
		mm[0] = 7;
		if (ay > ax)
		{
			mm[1] = 8;
			mm[2] = 4;
			mm[3] = 9;
			mm[4] = 1;
		}
		else
		{
			mm[1] = 4;
			mm[2] = 8;
			mm[3] = 1;
			mm[4] = 9;
		}
		break;
	case 5:
	case 13:
		mm[0] = 4;
		if (y < 0)
		{
			mm[1] = 1;
			mm[2] = 7;
			mm[3] = 2;
			mm[4] = 8;
		}
		else
		{
			mm[1] = 7;
			mm[2] = 1;
			mm[3] = 8;
			mm[4] = 2;
		}
		break;
	case 8:
		mm[0] = 3;
		if (ay > ax)
		{
			mm[1] = 2;
			mm[2] = 6;
			mm[3] = 1;
			mm[4] = 9;
		}
		else
		{
			mm[1] = 6;
			mm[2] = 2;
			mm[3] = 9;
			mm[4] = 1;
		}
		break;
	case 10:
	case 14:
		mm[0] = 2;
		if (x < 0)
		{
			mm[1] = 3;
			mm[2] = 1;
			mm[3] = 6;
			mm[4] = 4;
		}
		else
		{
			mm[1] = 1;
			mm[2] = 3;
			mm[3] = 4;
			mm[4] = 6;
		}
		break;
	case 12:
		mm[0] = 1;
		if (ay > ax)
		{
			mm[1] = 2;
			mm[2] = 4;
			mm[3] = 3;
			mm[4] = 7;
		}
		else
		{
			mm[1] = 4;
			mm[2] = 2;
			mm[3] = 7;
			mm[4] = 3;
		}
		break;
	}

	/* Wants to move... */
	return TRUE;
}


static bool check_hp_for_feat_destruction(feature_type *f_ptr, monster_type *m_ptr)
{
	return !have_flag(f_ptr->flags, FF_GLASS) ||
		(r_info[m_ptr->r_idx].flags2 & RF2_STUPID) ||
		(m_ptr->hp >= MAX(m_ptr->maxhp / 3, 200));
}


/*!
 * @brief モンスター単体の1ターン行動処理メインルーチン /
 * Process a monster
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param m_idx 行動モンスターの参照ID
 * @return なし
 * @details
 * The monster is known to be within 100 grids of the player\n
 *\n
 * In several cases, we directly update the monster lore\n
 *\n
 * Note that a monster is only allowed to "reproduce" if there\n
 * are a limited number of "reproducing" monsters on the current\n
 * level.  This should prevent the level from being "swamped" by\n
 * reproducing monsters.  It also allows a large mass of mice to\n
 * prevent a louse from multiplying, but this is a small price to\n
 * pay for a simple multiplication method.\n
 *\n
 * XXX Monster fear is slightly odd, in particular, monsters will\n
 * fixate on opening a door even if they cannot open it.  Actually,\n
 * the same thing happens to normal monsters when they hit a door\n
 *\n
 * In addition, monsters which *cannot* open or bash\n
 * down a door will still stand there trying to open it...\n
 *\n
 * XXX Technically, need to check for monster in the way\n
 * combined with that monster being in a wall (or door?)\n
 *\n
 * A "direction" of "5" means "pick a random direction".\n
 */
void process_monster(player_type *target_ptr, MONSTER_IDX m_idx)
{
	monster_type    *m_ptr = &target_ptr->current_floor_ptr->m_list[m_idx];
	monster_race    *r_ptr = &r_info[m_ptr->r_idx];
	monster_race    *ap_r_ptr = &r_info[m_ptr->ap_r_idx];

	int i, d;
	POSITION oy, ox, ny, nx;

	DIRECTION mm[8];

	grid_type       *g_ptr;
	feature_type    *f_ptr;

	monster_type    *y_ptr;

	bool do_turn;
	bool do_move;
	bool do_view;
	bool must_alter_to_move;

	bool did_open_door;
	bool did_bash_door;
	bool did_take_item;
	bool did_kill_item;
	bool did_move_body;
	bool did_pass_wall;
	bool did_kill_wall;
	bool gets_angry = FALSE;
	bool can_cross;
	bool aware = TRUE;

	bool fear, dead;
	bool is_riding_mon = (m_idx == target_ptr->riding);
	bool see_m = is_seen(m_ptr);

	if (is_riding_mon && !(r_ptr->flags7 & RF7_RIDING))
	{
		if (rakuba(target_ptr, 0, TRUE))
		{
#ifdef JP
			msg_print("地面に落とされた。");
#else
			GAME_TEXT m_name[MAX_NLEN];
			monster_desc(target_ptr, m_name, &target_ptr->current_floor_ptr->m_list[target_ptr->riding], 0);
			msg_format("You have fallen from %s.", m_name);
#endif
		}
	}

	if ((m_ptr->mflag2 & MFLAG2_CHAMELEON) && one_in_(13) && !MON_CSLEEP(m_ptr))
	{
		choose_new_monster(target_ptr, m_idx, FALSE, 0);
		r_ptr = &r_info[m_ptr->r_idx];
	}

	/* Players hidden in shadow are almost imperceptable. -LM- */
	if (target_ptr->special_defense & NINJA_S_STEALTH)
	{
		int tmp = target_ptr->lev * 6 + (target_ptr->skill_stl + 10) * 4;
		if (target_ptr->monlite) tmp /= 3;
		if (target_ptr->cursed & TRC_AGGRAVATE) tmp /= 2;
		if (r_ptr->level > (target_ptr->lev * target_ptr->lev / 20 + 10)) tmp /= 3;
		/* Low-level monsters will find it difficult to locate the player. */
		if (randint0(tmp) > (r_ptr->level + 20)) aware = FALSE;
	}

	/* Are there its parent? */
	if (m_ptr->parent_m_idx && !target_ptr->current_floor_ptr->m_list[m_ptr->parent_m_idx].r_idx)
	{
		/* Its parent have gone, it also goes away. */

		if (see_m)
		{
			GAME_TEXT m_name[MAX_NLEN];
			monster_desc(target_ptr, m_name, m_ptr, 0);
			msg_format(_("%sは消え去った！", "%^s disappears!"), m_name);
		}

		if (record_named_pet && is_pet(m_ptr) && m_ptr->nickname)
		{
			GAME_TEXT m_name[MAX_NLEN];
			monster_desc(target_ptr, m_name, m_ptr, MD_INDEF_VISIBLE);
			exe_write_diary(target_ptr, DIARY_NAMED_PET, RECORD_NAMED_PET_LOSE_PARENT, m_name);
		}

		delete_monster_idx(target_ptr, m_idx);

		return;
	}

	/* Quantum monsters are odd */
	if (r_ptr->flags2 & (RF2_QUANTUM))
	{
		/* Sometimes skip move */
		if (!randint0(2)) return;

		/* Sometimes die */
		if (!randint0((m_idx % 100) + 10) && !(r_ptr->flags1 & RF1_QUESTOR))
		{
			bool sad = FALSE;

			if (is_pet(m_ptr) && !(m_ptr->ml)) sad = TRUE;

			if (see_m)
			{
				GAME_TEXT m_name[MAX_NLEN];
				monster_desc(target_ptr, m_name, m_ptr, 0);

				msg_format(_("%sは消え去った！", "%^s disappears!"), m_name);
			}

			/* Generate treasure, etc */
			monster_death(target_ptr, m_idx, FALSE);

			delete_monster_idx(target_ptr, m_idx);
			if (sad)
			{
				msg_print(_("少しの間悲しい気分になった。", "You feel sad for a moment."));
			}

			return;
		}
	}

	if (m_ptr->r_idx == MON_SHURYUUDAN)
	{
		mon_take_hit_mon(target_ptr, m_idx, 1, &dead, &fear, _("は爆発して粉々になった。", " explodes into tiny shreds."), m_idx);
		if (dead) return;
	}

	if ((is_pet(m_ptr) || is_friendly(m_ptr)) && ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL)) && !target_ptr->phase_out)
	{
		static int riding_pinch = 0;

		if (m_ptr->hp < m_ptr->maxhp / 3)
		{
			GAME_TEXT m_name[MAX_NLEN];
			monster_desc(target_ptr, m_name, m_ptr, 0);

			if (is_riding_mon && riding_pinch < 2)
			{
				msg_format(_("%sは傷の痛さの余りあなたの束縛から逃れようとしている。",
					"%^s seems to be in so much pain and tries to escape from your restriction."), m_name);
				riding_pinch++;
				disturb(target_ptr, TRUE, TRUE);
			}
			else
			{
				if (is_riding_mon)
				{
					msg_format(_("%sはあなたの束縛から脱出した。", "%^s succeeded to escape from your restriction!"), m_name);
					if (rakuba(target_ptr, -1, FALSE))
					{
						msg_print(_("地面に落とされた。", "You have fallen from the pet you were riding."));
					}
				}

				if (see_m)
				{
					if ((r_ptr->flags2 & RF2_CAN_SPEAK) && (m_ptr->r_idx != MON_GRIP) && (m_ptr->r_idx != MON_WOLF) && (m_ptr->r_idx != MON_FANG) &&
						player_has_los_bold(target_ptr, m_ptr->fy, m_ptr->fx) && projectable(target_ptr, m_ptr->fy, m_ptr->fx, target_ptr->y, target_ptr->x))
					{
						msg_format(_("%^s「ピンチだ！退却させてもらう！」", "%^s says 'It is the pinch! I will retreat'."), m_name);
					}
					msg_format(_("%^sがテレポート・レベルの巻物を読んだ。", "%^s reads a scroll of teleport level."), m_name);
					msg_format(_("%^sが消え去った。", "%^s disappears."), m_name);
				}

				if (is_riding_mon && rakuba(target_ptr, -1, FALSE))
				{
					msg_print(_("地面に落とされた。", "You have fallen from the pet you were riding."));
				}

				check_quest_completion(target_ptr, m_ptr);
				delete_monster_idx(target_ptr, m_idx);
				return;
			}
		}
		else
		{
			/* Reset the counter */
			if (is_riding_mon) riding_pinch = 0;
		}
	}

	/* Handle "sleep" */
	if (MON_CSLEEP(m_ptr))
	{
		/* Handle non-aggravation - Still sleeping */
		if (!(target_ptr->cursed & TRC_AGGRAVATE)) return;

		(void)set_monster_csleep(target_ptr, m_idx, 0);

		/* Notice the "waking up" */
		if (m_ptr->ml)
		{
			GAME_TEXT m_name[MAX_NLEN];
			monster_desc(target_ptr, m_name, m_ptr, 0);
			msg_format(_("%^sが目を覚ました。", "%^s wakes up."), m_name);
		}

		/* Hack -- Count the wakings */
		if (is_original_ap_and_seen(target_ptr, m_ptr) && (r_ptr->r_wake < MAX_UCHAR))
		{
			r_ptr->r_wake++;
		}
	}

	/* Handle "stun" */
	if (MON_STUNNED(m_ptr))
	{
		/* Sometimes skip move */
		if (one_in_(2)) return;
	}

	if (is_riding_mon)
	{
		target_ptr->update |= (PU_BONUS);
	}

	/* No one wants to be your friend if you're aggravating */
	if (is_friendly(m_ptr) && (target_ptr->cursed & TRC_AGGRAVATE))
		gets_angry = TRUE;

	/* Paranoia... no pet uniques outside wizard mode -- TY */
	if (is_pet(m_ptr) && ((((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL)) &&
		monster_has_hostile_align(target_ptr, NULL, 10, -10, r_ptr)) || (r_ptr->flagsr & RFR_RES_ALL)))
	{
		gets_angry = TRUE;
	}

	if (target_ptr->phase_out) gets_angry = FALSE;

	if (gets_angry)
	{
		if (is_pet(m_ptr) || see_m)
		{
			GAME_TEXT m_name[MAX_NLEN];
			monster_desc(target_ptr, m_name, m_ptr, is_pet(m_ptr) ? MD_ASSUME_VISIBLE : 0);
			msg_format(_("%^sは突然敵にまわった！", "%^s suddenly becomes hostile!"), m_name);
		}

		set_hostile(target_ptr, m_ptr);
	}

	/* Get the origin */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Attempt to "multiply" if able and allowed */
	if ((r_ptr->flags2 & RF2_MULTIPLY) && (target_ptr->current_floor_ptr->num_repro < MAX_REPRO))
	{
		int k;
		POSITION y, x;

		/* Count the adjacent monsters */
		for (k = 0, y = oy - 1; y <= oy + 1; y++)
		{
			for (x = ox - 1; x <= ox + 1; x++)
			{
				/* Ignore locations off of edge */
				if (!in_bounds2(target_ptr->current_floor_ptr, y, x)) continue;
				if (target_ptr->current_floor_ptr->grid_array[y][x].m_idx) k++;
			}
		}

		if (multiply_barrier(target_ptr, m_idx)) k = 8;

		/* Hack -- multiply slower in crowded areas */
		if ((k < 4) && (!k || !randint0(k * MON_MULT_ADJ)))
		{
			/* Try to multiply */
			if (multiply_monster(target_ptr, m_idx, FALSE, (is_pet(m_ptr) ? PM_FORCE_PET : 0)))
			{
				/* Take note if visible */
				if (target_ptr->current_floor_ptr->m_list[hack_m_idx_ii].ml && is_original_ap_and_seen(target_ptr, m_ptr))
				{
					r_ptr->r_flags2 |= (RF2_MULTIPLY);
				}

				/* Multiplying takes energy */
				return;
			}
		}
	}

	if (r_ptr->a_ability_flags2 & RF6_SPECIAL)
	{
		/* Hack -- Ohmu scatters molds! */
		if (m_ptr->r_idx == MON_OHMU)
		{
			if (!target_ptr->current_floor_ptr->inside_arena && !target_ptr->phase_out)
			{
				if (r_ptr->freq_spell && (randint1(100) <= r_ptr->freq_spell))
				{
					int k, count = 0;
					DEPTH rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);
					BIT_FLAGS p_mode = is_pet(m_ptr) ? PM_FORCE_PET : 0L;

					for (k = 0; k < A_MAX; k++)
					{
						if (summon_specific(target_ptr, m_idx, m_ptr->fy, m_ptr->fx, rlev, SUMMON_MOLD, (PM_ALLOW_GROUP | p_mode)))
						{
							if (target_ptr->current_floor_ptr->m_list[hack_m_idx_ii].ml) count++;
						}
					}

					if (count && is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags6 |= (RF6_SPECIAL);
				}
			}
		}
	}

	if (!target_ptr->phase_out)
	{
		/* Hack! "Cyber" monster makes noise... */
		if (m_ptr->ap_r_idx == MON_CYBER &&
			one_in_(CYBERNOISE) &&
			!m_ptr->ml && (m_ptr->cdis <= MAX_SIGHT))
		{
			if (disturb_minor) disturb(target_ptr, FALSE, FALSE);
			msg_print(_("重厚な足音が聞こえた。", "You hear heavy steps."));
		}

		/* Some monsters can speak */
		if ((ap_r_ptr->flags2 & RF2_CAN_SPEAK) && aware &&
			one_in_(SPEAK_CHANCE) &&
			player_has_los_bold(target_ptr, oy, ox) &&
			projectable(target_ptr, oy, ox, target_ptr->y, target_ptr->x))
		{
			GAME_TEXT m_name[MAX_NLEN];
			char monmessage[1024];
			concptr filename;

			/* Acquire the monster name/poss */
			if (m_ptr->ml)
				monster_desc(target_ptr, m_name, m_ptr, 0);
			else
				strcpy(m_name, _("それ", "It"));

			/* Select the file for monster quotes */
			if (MON_MONFEAR(m_ptr))
				filename = _("monfear_j.txt", "monfear.txt");
			else if (is_pet(m_ptr))
				filename = _("monpet_j.txt", "monpet.txt");
			else if (is_friendly(m_ptr))
				filename = _("monfrien_j.txt", "monfrien.txt");
			else
				filename = _("monspeak_j.txt", "monspeak.txt");
			/* Get the monster line */
			if (get_rnd_line(filename, m_ptr->ap_r_idx, monmessage) == 0)
			{
				/* Say something */
				msg_format(_("%^s%s", "%^s %s"), m_name, monmessage);
			}
		}
	}

	/* Try to cast spell occasionally */
	if (r_ptr->freq_spell && randint1(100) <= r_ptr->freq_spell)
	{
		bool counterattack = FALSE;

		/* Give priority to counter attack? */
		if (m_ptr->target_y)
		{
			MONSTER_IDX t_m_idx = target_ptr->current_floor_ptr->grid_array[m_ptr->target_y][m_ptr->target_x].m_idx;

			/* The monster must be an enemy, and projectable */
			if (t_m_idx && are_enemies(target_ptr, m_ptr, &target_ptr->current_floor_ptr->m_list[t_m_idx]) &&
				projectable(target_ptr, m_ptr->fy, m_ptr->fx, m_ptr->target_y, m_ptr->target_x))
			{
				counterattack = TRUE;
			}
		}

		if (!counterattack)
		{
			/* Attempt to cast a spell */
			if (aware && make_attack_spell(m_idx, target_ptr)) return;

			/*
			 * Attempt to cast a spell at an enemy other than the player
			 * (may slow the game a smidgeon, but I haven't noticed.)
			 */
			if (monst_spell_monst(target_ptr, m_idx)) return;
		}
		else
		{
			/* Attempt to do counter attack at first */
			if (monst_spell_monst(target_ptr, m_idx)) return;

			if (aware && make_attack_spell(m_idx, target_ptr)) return;
		}
	}

	/* Hack -- Assume no movement */
	mm[0] = mm[1] = mm[2] = mm[3] = 0;
	mm[4] = mm[5] = mm[6] = mm[7] = 0;


	/* Confused -- 100% random */
	if (MON_CONFUSED(m_ptr) || !aware)
	{
		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* 75% random movement */
	else if (((r_ptr->flags1 & (RF1_RAND_50 | RF1_RAND_25)) == (RF1_RAND_50 | RF1_RAND_25)) &&
		(randint0(100) < 75))
	{
		/* Memorize flags */
		if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags1 |= (RF1_RAND_50 | RF1_RAND_25);

		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* 50% random movement */
	else if ((r_ptr->flags1 & RF1_RAND_50) &&
		(randint0(100) < 50))
	{
		/* Memorize flags */
		if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags1 |= (RF1_RAND_50);

		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* 25% random movement */
	else if ((r_ptr->flags1 & RF1_RAND_25) &&
		(randint0(100) < 25))
	{
		/* Memorize flags */
		if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags1 |= RF1_RAND_25;

		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* Can't reach player - find something else to hit */
	else if ((r_ptr->flags1 & RF1_NEVER_MOVE) && (m_ptr->cdis > 1))
	{
		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* Pets will follow the player */
	else if (is_pet(m_ptr))
	{
		/* Are we trying to avoid the player? */
		bool avoid = ((target_ptr->pet_follow_distance < 0) && (m_ptr->cdis <= (0 - target_ptr->pet_follow_distance)));

		/* Do we want to find the player? */
		bool lonely = (!avoid && (m_ptr->cdis > target_ptr->pet_follow_distance));

		/* Should we find the player if we can't find a monster? */
		bool distant = (m_ptr->cdis > PET_SEEK_DIST);

		/* by default, move randomly */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;

		/* Look for an enemy */
		if (!get_enemy_dir(target_ptr, m_idx, mm))
		{
			/* Find the player if necessary */
			if (avoid || lonely || distant)
			{
				/* Remember the leash length */
				POSITION dis = target_ptr->pet_follow_distance;

				/* Hack -- adjust follow distance temporarily */
				if (target_ptr->pet_follow_distance > PET_SEEK_DIST)
				{
					target_ptr->pet_follow_distance = PET_SEEK_DIST;
				}

				/* Find the player */
				(void)get_moves(target_ptr, m_idx, mm);

				/* Restore the leash */
				target_ptr->pet_follow_distance = (s16b)dis;
			}
		}
	}

	/* Friendly monster movement */
	else if (!is_hostile(m_ptr))
	{
		/* by default, move randomly */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;

		/* Look for an enemy */
		get_enemy_dir(target_ptr, m_idx, mm);
	}
	/* Normal movement */
	else
	{
		/* Logical moves, may do nothing */
		if (!get_moves(target_ptr, m_idx, mm)) return;
	}

	/* Assume nothing */
	do_turn = FALSE;
	do_move = FALSE;
	do_view = FALSE;
	must_alter_to_move = FALSE;

	/* Assume nothing */
	did_open_door = FALSE;
	did_bash_door = FALSE;
	did_take_item = FALSE;
	did_kill_item = FALSE;
	did_move_body = FALSE;
	did_pass_wall = FALSE;
	did_kill_wall = FALSE;

	/* Take a zero-terminated array of "directions" */
	for (i = 0; mm[i]; i++)
	{
		/* Get the direction */
		d = mm[i];

		/* Hack -- allow "randomized" motion */
		if (d == 5) d = ddd[randint0(8)];

		/* Get the destination */
		ny = oy + ddy[d];
		nx = ox + ddx[d];

		/* Ignore locations off of edge */
		if (!in_bounds2(target_ptr->current_floor_ptr, ny, nx)) continue;

		/* Access that grid */
		g_ptr = &target_ptr->current_floor_ptr->grid_array[ny][nx];
		f_ptr = &f_info[g_ptr->feat];
		can_cross = monster_can_cross_terrain(target_ptr, g_ptr->feat, r_ptr, is_riding_mon ? CEM_RIDING : 0);

		/* Access that grid's contents */
		y_ptr = &target_ptr->current_floor_ptr->m_list[g_ptr->m_idx];

		/* Hack -- player 'in' wall */
		if (player_bold(target_ptr, ny, nx))
		{
			do_move = TRUE;
		}

		/* Possibly a monster to attack */
		else if (g_ptr->m_idx)
		{
			do_move = TRUE;
		}

		/* Monster destroys walls (and doors) */
		else if ((r_ptr->flags2 & RF2_KILL_WALL) &&
			(can_cross ? !have_flag(f_ptr->flags, FF_LOS) : !is_riding_mon) &&
			have_flag(f_ptr->flags, FF_HURT_DISI) && !have_flag(f_ptr->flags, FF_PERMANENT) &&
			check_hp_for_feat_destruction(f_ptr, m_ptr))
		{
			/* Eat through walls/doors/rubble */
			do_move = TRUE;
			if (!can_cross) must_alter_to_move = TRUE;

			/* Monster destroyed a wall (later) */
			did_kill_wall = TRUE;
		}

		/* Floor is open? */
		else if (can_cross)
		{
			/* Go ahead and move */
			do_move = TRUE;

			/* Monster moves through walls (and doors) */
			if ((r_ptr->flags2 & RF2_PASS_WALL) && (!is_riding_mon || target_ptr->pass_wall) &&
				have_flag(f_ptr->flags, FF_CAN_PASS))
			{
				/* Monster went through a wall */
				did_pass_wall = TRUE;
			}
		}

		/* Handle doors and secret doors */
		else if (is_closed_door(target_ptr, g_ptr->feat))
		{
			bool may_bash = TRUE;

			/* Assume no move allowed */
			do_move = FALSE;

			/* Creature can open doors. */
			if ((r_ptr->flags2 & RF2_OPEN_DOOR) && have_flag(f_ptr->flags, FF_OPEN) &&
				(!is_pet(m_ptr) || (target_ptr->pet_extra_flags & PF_OPEN_DOORS)))
			{
				/* Closed doors */
				if (!f_ptr->power)
				{
					/* The door is open */
					did_open_door = TRUE;

					/* Do not bash the door */
					may_bash = FALSE;

					do_turn = TRUE;
				}

				/* Locked doors (not jammed) */
				else
				{
					/* Try to unlock it */
					if (randint0(m_ptr->hp / 10) > f_ptr->power)
					{
						/* Unlock the door */
						cave_alter_feat(target_ptr, ny, nx, FF_DISARM);

						/* Do not bash the door */
						may_bash = FALSE;

						do_turn = TRUE;
					}
				}
			}

			/* Stuck doors -- attempt to bash them down if allowed */
			if (may_bash && (r_ptr->flags2 & RF2_BASH_DOOR) && have_flag(f_ptr->flags, FF_BASH) &&
				(!is_pet(m_ptr) || (target_ptr->pet_extra_flags & PF_OPEN_DOORS)))
			{
				/* Attempt to Bash */
				if (check_hp_for_feat_destruction(f_ptr, m_ptr) && (randint0(m_ptr->hp / 10) > f_ptr->power))
				{
					if (have_flag(f_ptr->flags, FF_GLASS))
						msg_print(_("ガラスが砕ける音がした！", "You hear glass breaking!"));
					else
						msg_print(_("ドアを叩き開ける音がした！", "You hear a door burst open!"));

					/* Disturb (sometimes) */
					if (disturb_minor) disturb(target_ptr, FALSE, FALSE);

					/* The door was bashed open */
					did_bash_door = TRUE;

					/* Hack -- fall into doorway */
					do_move = TRUE;
					must_alter_to_move = TRUE;
				}
			}


			/* Deal with doors in the way */
			if (did_open_door || did_bash_door)
			{
				/* Break down the door */
				if (did_bash_door && ((randint0(100) < 50) || (feat_state(target_ptr, g_ptr->feat, FF_OPEN) == g_ptr->feat) || have_flag(f_ptr->flags, FF_GLASS)))
				{
					cave_alter_feat(target_ptr, ny, nx, FF_BASH);

					if (!monster_is_valid(m_ptr)) /* Killed by shards of glass, etc. */
					{
						target_ptr->update |= (PU_FLOW);
						target_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
						if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_BASH_DOOR);

						return;
					}
				}

				/* Open the door */
				else
				{
					cave_alter_feat(target_ptr, ny, nx, FF_OPEN);
				}

				f_ptr = &f_info[g_ptr->feat];

				/* Handle viewable doors */
				do_view = TRUE;
			}
		}

		/* Hack -- check for Glyph of Warding */
		if (do_move && is_glyph_grid(g_ptr) &&
			!((r_ptr->flags1 & RF1_NEVER_BLOW) && player_bold(target_ptr, ny, nx)))
		{
			/* Assume no move allowed */
			do_move = FALSE;

			/* Break the ward */
			if (!is_pet(m_ptr) && (randint1(BREAK_GLYPH) < r_ptr->level))
			{
				/* Describe observable breakage */
				if (g_ptr->info & CAVE_MARK)
				{
					msg_print(_("守りのルーンが壊れた！", "The rune of protection is broken!"));
				}

				/* Forget the rune */
				g_ptr->info &= ~(CAVE_MARK);

				/* Break the rune */
				g_ptr->info &= ~(CAVE_OBJECT);
				g_ptr->mimic = 0;

				/* Allow movement */
				do_move = TRUE;

				note_spot(target_ptr, ny, nx);
			}
		}
		else if (do_move && is_explosive_rune_grid(g_ptr) &&
			!((r_ptr->flags1 & RF1_NEVER_BLOW) && player_bold(target_ptr, ny, nx)))
		{
			/* Assume no move allowed */
			do_move = FALSE;

			/* Break the ward */
			if (!is_pet(m_ptr))
			{
				/* Break the ward */
				if (randint1(BREAK_MINOR_GLYPH) > r_ptr->level)
				{
					/* Describe observable breakage */
					if (g_ptr->info & CAVE_MARK)
					{
						msg_print(_("ルーンが爆発した！", "The rune explodes!"));
						project(target_ptr, 0, 2, ny, nx, 2 * (target_ptr->lev + damroll(7, 7)), GF_MANA, (PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP | PROJECT_NO_HANGEKI), -1);
					}
				}
				else
				{
					msg_print(_("爆発のルーンは解除された。", "An explosive rune was disarmed."));
				}

				/* Forget the rune */
				g_ptr->info &= ~(CAVE_MARK);

				/* Break the rune */
				g_ptr->info &= ~(CAVE_OBJECT);
				g_ptr->mimic = 0;

				note_spot(target_ptr, ny, nx);
				lite_spot(target_ptr, ny, nx);

				if (!monster_is_valid(m_ptr)) return;
				/* Allow movement */
				do_move = TRUE;
			}
		}

		/* The player is in the way */
		if (do_move && player_bold(target_ptr, ny, nx))
		{
			/* Some monsters never attack */
			if (r_ptr->flags1 & RF1_NEVER_BLOW)
			{
				/* Hack -- memorize lack of attacks */
				if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags1 |= (RF1_NEVER_BLOW);

				/* Do not move */
				do_move = FALSE;
			}

			/* In anti-melee dungeon, stupid or confused monster takes useless turn */
			if (do_move && (d_info[target_ptr->dungeon_idx].flags1 & DF1_NO_MELEE))
			{
				if (!MON_CONFUSED(m_ptr))
				{
					if (!(r_ptr->flags2 & RF2_STUPID)) do_move = FALSE;
					else
					{
						if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_STUPID);
					}
				}
			}

			/* The player is in the way.  Attack him. */
			if (do_move)
			{
				if (!target_ptr->riding || one_in_(2))
				{
					/* Do the attack */
					(void)make_attack_normal(target_ptr, m_idx);

					/* Do not move */
					do_move = FALSE;

					/* Took a turn */
					do_turn = TRUE;
				}
			}
		}

		/* A monster is in the way */
		if (do_move && g_ptr->m_idx)
		{
			monster_race *z_ptr = &r_info[y_ptr->r_idx];

			/* Assume no movement */
			do_move = FALSE;

			/* Attack 'enemies' */
			if (((r_ptr->flags2 & RF2_KILL_BODY) && !(r_ptr->flags1 & RF1_NEVER_BLOW) &&
				(r_ptr->mexp * r_ptr->level > z_ptr->mexp * z_ptr->level) &&
				can_cross && (g_ptr->m_idx != target_ptr->riding)) ||
				are_enemies(target_ptr, m_ptr, y_ptr) || MON_CONFUSED(m_ptr))
			{
				if (!(r_ptr->flags1 & RF1_NEVER_BLOW))
				{
					if (r_ptr->flags2 & RF2_KILL_BODY)
					{
						if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_KILL_BODY);
					}

					/* attack */
					if (y_ptr->r_idx && (y_ptr->hp >= 0))
					{
						if (monst_attack_monst(target_ptr, m_idx, g_ptr->m_idx)) return;

						/* In anti-melee dungeon, stupid or confused monster takes useless turn */
						else if (d_info[target_ptr->dungeon_idx].flags1 & DF1_NO_MELEE)
						{
							if (MON_CONFUSED(m_ptr)) return;
							else if (r_ptr->flags2 & RF2_STUPID)
							{
								if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_STUPID);
								return;
							}
						}
					}
				}
			}

			/* Push past weaker monsters (unless leaving a wall) */
			else if ((r_ptr->flags2 & RF2_MOVE_BODY) && !(r_ptr->flags1 & RF1_NEVER_MOVE) &&
				(r_ptr->mexp > z_ptr->mexp) &&
				can_cross && (g_ptr->m_idx != target_ptr->riding) &&
				monster_can_cross_terrain(target_ptr, target_ptr->current_floor_ptr->grid_array[m_ptr->fy][m_ptr->fx].feat, z_ptr, 0))
			{
				/* Allow movement */
				do_move = TRUE;

				/* Monster pushed past another monster */
				did_move_body = TRUE;

				/* Wake up the moved monster */
				(void)set_monster_csleep(target_ptr, g_ptr->m_idx, 0);

				/* Message */
			}
		}

		if (is_riding_mon)
		{
			if (!target_ptr->riding_ryoute && !MON_MONFEAR(&target_ptr->current_floor_ptr->m_list[target_ptr->riding])) do_move = FALSE;
		}

		if (did_kill_wall && do_move)
		{
			if (one_in_(GRINDNOISE))
			{
				if (have_flag(f_ptr->flags, FF_GLASS))
					msg_print(_("何かの砕ける音が聞こえる。", "There is a crashing sound."));
				else
					msg_print(_("ギシギシいう音が聞こえる。", "There is a grinding sound."));
			}

			cave_alter_feat(target_ptr, ny, nx, FF_HURT_DISI);

			if (!monster_is_valid(m_ptr)) /* Killed by shards of glass, etc. */
			{
				target_ptr->update |= (PU_FLOW);
				target_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
				if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_KILL_WALL);

				return;
			}

			f_ptr = &f_info[g_ptr->feat];

			/* Note changes to viewable region */
			do_view = TRUE;
			do_turn = TRUE;
		}

		if (must_alter_to_move && (r_ptr->flags7 & RF7_AQUATIC))
		{
			if (!monster_can_cross_terrain(target_ptr, g_ptr->feat, r_ptr, is_riding_mon ? CEM_RIDING : 0))
			{
				/* Assume no move allowed */
				do_move = FALSE;
			}
		}

		/*
		 * Check if monster can cross terrain
		 * This is checked after the normal attacks
		 * to allow monsters to attack an enemy,
		 * even if it can't enter the terrain.
		 */
		if (do_move && !can_cross && !did_kill_wall && !did_bash_door)
		{
			/* Assume no move allowed */
			do_move = FALSE;
		}

		/* Some monsters never move */
		if (do_move && (r_ptr->flags1 & RF1_NEVER_MOVE))
		{
			/* Hack -- memorize lack of moves */
			if (is_original_ap_and_seen(target_ptr, m_ptr)) r_ptr->r_flags1 |= (RF1_NEVER_MOVE);

			/* Do not move */
			do_move = FALSE;
		}

		/* Creature has been allowed move */
		if (!do_move)
		{
			if (do_turn) break;
			continue;
		}

		do_turn = TRUE;

		if (have_flag(f_ptr->flags, FF_TREE))
		{
			if (!(r_ptr->flags7 & RF7_CAN_FLY) && !(r_ptr->flags8 & RF8_WILD_WOOD))
			{
				m_ptr->energy_need += ENERGY_NEED();
			}
		}

		if (!is_riding_mon)
		{
			/* Hack -- Update the old location */
			target_ptr->current_floor_ptr->grid_array[oy][ox].m_idx = g_ptr->m_idx;

			/* Mega-Hack -- move the old monster, if any */
			if (g_ptr->m_idx)
			{
				/* Move the old monster */
				y_ptr->fy = oy;
				y_ptr->fx = ox;

				/* Update the old monster */
				update_monster(target_ptr, g_ptr->m_idx, TRUE);
			}

			/* Hack -- Update the new location */
			g_ptr->m_idx = m_idx;

			/* Move the monster */
			m_ptr->fy = ny;
			m_ptr->fx = nx;
			update_monster(target_ptr, m_idx, TRUE);

			lite_spot(target_ptr, oy, ox);
			lite_spot(target_ptr, ny, nx);
		}
		else
		{
			/* sound(SOUND_WALK); */
			if (!move_player_effect(target_ptr, ny, nx, MPE_DONT_PICKUP)) break;
		}

		/* Possible disturb */
		if (m_ptr->ml &&
			(disturb_move ||
			(disturb_near && (m_ptr->mflag & MFLAG_VIEW) && projectable(target_ptr, target_ptr->y, target_ptr->x, m_ptr->fy, m_ptr->fx)) ||
				(disturb_high && ap_r_ptr->r_tkills && ap_r_ptr->level >= target_ptr->lev)))
		{
			if (is_hostile(m_ptr))
				disturb(target_ptr, FALSE, TRUE);
		}

		/* Take or Kill objects on the floor */
		bool is_takable_or_killable = g_ptr->o_idx > 0;
		is_takable_or_killable &= (r_ptr->flags2 & (RF2_TAKE_ITEM | RF2_KILL_ITEM));
		is_takable_or_killable &= !is_pet(m_ptr) || ((target_ptr->pet_extra_flags & PF_PICKUP_ITEMS) && (r_ptr->flags2 & RF2_TAKE_ITEM));
		if (!is_takable_or_killable)
		{
			if (do_turn) break;
			continue;
		}

		OBJECT_IDX this_o_idx, next_o_idx;
		bool do_take = (r_ptr->flags2 & RF2_TAKE_ITEM) ? TRUE : FALSE;

		/* Scan all objects in the grid */
		for (this_o_idx = g_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
		{
			BIT_FLAGS flgs[TR_FLAG_SIZE], flg2 = 0L, flg3 = 0L, flgr = 0L;
			GAME_TEXT m_name[MAX_NLEN], o_name[MAX_NLEN];
			object_type *o_ptr = &target_ptr->current_floor_ptr->o_list[this_o_idx];
			next_o_idx = o_ptr->next_o_idx;

			if (do_take)
			{
				/* Skip gold */
				if (o_ptr->tval == TV_GOLD) continue;

				/*
				 * Skip "real" corpses and statues, to avoid extreme
				 * silliness like a novice rogue pockets full of statues
				 * and corpses.
				 */
				if ((o_ptr->tval == TV_CORPSE) ||
					(o_ptr->tval == TV_STATUE)) continue;
			}

			/* Extract some flags */
			object_flags(o_ptr, flgs);

			/* Acquire the object name */
			object_desc(target_ptr, o_name, o_ptr, 0);
			monster_desc(target_ptr, m_name, m_ptr, MD_INDEF_HIDDEN);

			/* React to objects that hurt the monster */
			if (have_flag(flgs, TR_SLAY_DRAGON)) flg3 |= (RF3_DRAGON);
			if (have_flag(flgs, TR_KILL_DRAGON)) flg3 |= (RF3_DRAGON);
			if (have_flag(flgs, TR_SLAY_TROLL))  flg3 |= (RF3_TROLL);
			if (have_flag(flgs, TR_KILL_TROLL))  flg3 |= (RF3_TROLL);
			if (have_flag(flgs, TR_SLAY_GIANT))  flg3 |= (RF3_GIANT);
			if (have_flag(flgs, TR_KILL_GIANT))  flg3 |= (RF3_GIANT);
			if (have_flag(flgs, TR_SLAY_ORC))    flg3 |= (RF3_ORC);
			if (have_flag(flgs, TR_KILL_ORC))    flg3 |= (RF3_ORC);
			if (have_flag(flgs, TR_SLAY_DEMON))  flg3 |= (RF3_DEMON);
			if (have_flag(flgs, TR_KILL_DEMON))  flg3 |= (RF3_DEMON);
			if (have_flag(flgs, TR_SLAY_UNDEAD)) flg3 |= (RF3_UNDEAD);
			if (have_flag(flgs, TR_KILL_UNDEAD)) flg3 |= (RF3_UNDEAD);
			if (have_flag(flgs, TR_SLAY_ANIMAL)) flg3 |= (RF3_ANIMAL);
			if (have_flag(flgs, TR_KILL_ANIMAL)) flg3 |= (RF3_ANIMAL);
			if (have_flag(flgs, TR_SLAY_EVIL))   flg3 |= (RF3_EVIL);
			if (have_flag(flgs, TR_KILL_EVIL))   flg3 |= (RF3_EVIL);
			if (have_flag(flgs, TR_SLAY_HUMAN))  flg2 |= (RF2_HUMAN);
			if (have_flag(flgs, TR_KILL_HUMAN))  flg2 |= (RF2_HUMAN);
			if (have_flag(flgs, TR_BRAND_ACID))  flgr |= (RFR_IM_ACID);
			if (have_flag(flgs, TR_BRAND_ELEC))  flgr |= (RFR_IM_ELEC);
			if (have_flag(flgs, TR_BRAND_FIRE))  flgr |= (RFR_IM_FIRE);
			if (have_flag(flgs, TR_BRAND_COLD))  flgr |= (RFR_IM_COLD);
			if (have_flag(flgs, TR_BRAND_POIS))  flgr |= (RFR_IM_POIS);

			/* The object cannot be picked up by the monster */
			if (object_is_artifact(o_ptr) || (r_ptr->flags3 & flg3) || (r_ptr->flags2 & flg2) ||
				((~(r_ptr->flagsr) & flgr) && !(r_ptr->flagsr & RFR_RES_ALL)))
			{
				/* Only give a message for "take_item" */
				if (do_take && (r_ptr->flags2 & RF2_STUPID))
				{
					did_take_item = TRUE;

					/* Describe observable situations */
					if (m_ptr->ml && player_can_see_bold(target_ptr, ny, nx))
					{
						msg_format(_("%^sは%sを拾おうとしたが、だめだった。", "%^s tries to pick up %s, but fails."), m_name, o_name);
					}
				}
			}

			/* Pick up the item */
			else if (do_take)
			{
				did_take_item = TRUE;

				/* Describe observable situations */
				if (player_can_see_bold(target_ptr, ny, nx))
				{
					msg_format(_("%^sが%sを拾った。", "%^s picks up %s."), m_name, o_name);
				}

				/* Excise the object */
				excise_object_idx(target_ptr->current_floor_ptr, this_o_idx);

				/* Forget mark */
				o_ptr->marked &= OM_TOUCHED;

				/* Forget location */
				o_ptr->iy = o_ptr->ix = 0;

				/* Memorize monster */
				o_ptr->held_m_idx = m_idx;

				/* Build a stack */
				o_ptr->next_o_idx = m_ptr->hold_o_idx;

				/* Carry object */
				m_ptr->hold_o_idx = this_o_idx;
			}

			/* Destroy the item if not a pet */
			else if (!is_pet(m_ptr))
			{
				did_kill_item = TRUE;

				/* Describe observable situations */
				if (player_has_los_bold(target_ptr, ny, nx))
				{
					msg_format(_("%^sが%sを破壊した。", "%^s destroys %s."), m_name, o_name);
				}

				delete_object_idx(target_ptr, this_o_idx);
			}
		}

		if (do_turn) break;
	}

	/*
	 *  Forward movements failed, but now received LOS attack!
	 *  Try to flow by smell.
	 */
	if (target_ptr->no_flowed && i > 2 && m_ptr->target_y)
		m_ptr->mflag2 &= ~MFLAG2_NOFLOW;

	/* If we haven't done anything, try casting a spell again */
	if (!do_turn && !do_move && !MON_MONFEAR(m_ptr) && !is_riding_mon && aware)
	{
		/* Try to cast spell again */
		if (r_ptr->freq_spell && randint1(100) <= r_ptr->freq_spell)
		{
			if (make_attack_spell(m_idx, target_ptr)) return;
		}
	}

	/* Notice changes in view */
	if (do_view)
	{
		target_ptr->update |= (PU_FLOW);
		target_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
	}

	/* Notice changes in view */
	if (do_move && ((r_ptr->flags7 & (RF7_SELF_LD_MASK | RF7_HAS_DARK_1 | RF7_HAS_DARK_2))
		|| ((r_ptr->flags7 & (RF7_HAS_LITE_1 | RF7_HAS_LITE_2)) && !target_ptr->phase_out)))
	{
		target_ptr->update |= (PU_MON_LITE);
	}

	/* Learn things from observable monster */
	if (is_original_ap_and_seen(target_ptr, m_ptr))
	{
		/* Monster opened a door */
		if (did_open_door) r_ptr->r_flags2 |= (RF2_OPEN_DOOR);

		/* Monster bashed a door */
		if (did_bash_door) r_ptr->r_flags2 |= (RF2_BASH_DOOR);

		/* Monster tried to pick something up */
		if (did_take_item) r_ptr->r_flags2 |= (RF2_TAKE_ITEM);

		/* Monster tried to crush something */
		if (did_kill_item) r_ptr->r_flags2 |= (RF2_KILL_ITEM);

		/* Monster pushed past another monster */
		if (did_move_body) r_ptr->r_flags2 |= (RF2_MOVE_BODY);

		/* Monster passed through a wall */
		if (did_pass_wall) r_ptr->r_flags2 |= (RF2_PASS_WALL);

		/* Monster destroyed a wall */
		if (did_kill_wall) r_ptr->r_flags2 |= (RF2_KILL_WALL);
	}

	bool is_battle_determined = !do_turn && !do_move && MON_MONFEAR(m_ptr) && aware;
	if (!is_battle_determined) return;

	/* No longer afraid */
	(void)set_monster_monfear(target_ptr, m_idx, 0);

	/* Message if seen */
	if (see_m)
	{
		GAME_TEXT m_name[MAX_NLEN];
		monster_desc(target_ptr, m_name, m_ptr, 0);
		msg_format(_("%^sは戦いを決意した！", "%^s turns to fight!"), m_name);
	}

	if (m_ptr->ml) chg_virtue(target_ptr, V_COMPASSION, -1);
}

/*!
 * @brief 全モンスターのターン管理メインルーチン /
 * Process all the "live" monsters, once per game turn.
 * @return なし
 * @details
 * During each game current game turn, we scan through the list of all the "live" monsters,\n
 * (backwards, so we can excise any "freshly dead" monsters), energizing each\n
 * monster, and allowing fully energized monsters to move, attack, pass, etc.\n
 *\n
 * Note that monsters can never move in the monster array (except when the\n
 * "compact_monsters()" function is called by "dungeon()" or "save_player()").\n
 *\n
 * This function is responsible for at least half of the processor time\n
 * on a normal system with a "normal" amount of monsters and a player doing\n
 * normal things.\n
 *\n
 * When the player is resting, virtually 90% of the processor time is spent\n
 * in this function, and its children, "process_monster()" and "make_move()".\n
 *\n
 * Most of the rest of the time is spent in "update_view()" and "lite_spot()",\n
 * especially when the player is running.\n
 *\n
 * Note the special "MFLAG_BORN" flag, which allows us to ignore "fresh"\n
 * monsters while they are still being "born".  A monster is "fresh" only\n
 * during the game turn in which it is created, and we use the "hack_m_idx" to\n
 * determine if the monster is yet to be processed during the game turn.\n
 *\n
 * Note the special "MFLAG_NICE" flag, which allows the player to get one\n
 * move before any "nasty" monsters get to use their spell attacks.\n
 *\n
 * Note that when the "knowledge" about the currently tracked monster\n
 * changes (flags, attacks, spells), we induce a redraw of the monster\n
 * recall window.\n
 */
void process_monsters(player_type *target_ptr)
{
	BIT_FLAGS old_r_flags1 = 0L;
	BIT_FLAGS old_r_flags2 = 0L;
	BIT_FLAGS old_r_flags3 = 0L;
	BIT_FLAGS old_r_flags4 = 0L;
	BIT_FLAGS old_r_flags5 = 0L;
	BIT_FLAGS old_r_flags6 = 0L;
	BIT_FLAGS old_r_flagsr = 0L;

	byte old_r_blows0 = 0;
	byte old_r_blows1 = 0;
	byte old_r_blows2 = 0;
	byte old_r_blows3 = 0;

	/* Clear monster fighting indicator */
	floor_type *floor_ptr = target_ptr->current_floor_ptr;
	floor_ptr->monster_noise = FALSE;

	/* Memorize old race */
	MONRACE_IDX old_monster_race_idx = target_ptr->monster_race_idx;

	/* Acquire knowledge */
	byte old_r_cast_spell = 0;
	if (target_ptr->monster_race_idx)
	{
		/* Acquire current monster */
		monster_race *r_ptr;
		r_ptr = &r_info[target_ptr->monster_race_idx];

		/* Memorize flags */
		old_r_flags1 = r_ptr->r_flags1;
		old_r_flags2 = r_ptr->r_flags2;
		old_r_flags3 = r_ptr->r_flags3;
		old_r_flags4 = r_ptr->r_flags4;
		old_r_flags5 = r_ptr->r_flags5;
		old_r_flags6 = r_ptr->r_flags6;
		old_r_flagsr = r_ptr->r_flagsr;

		/* Memorize blows */
		old_r_blows0 = r_ptr->r_blows[0];
		old_r_blows1 = r_ptr->r_blows[1];
		old_r_blows2 = r_ptr->r_blows[2];
		old_r_blows3 = r_ptr->r_blows[3];

		/* Memorize castings */
		old_r_cast_spell = r_ptr->r_cast_spell;
	}

	/* Process the monsters (backwards) */
	bool test;
	for (MONSTER_IDX i = floor_ptr->m_max - 1; i >= 1; i--)
	{
		monster_type *m_ptr;
		monster_race *r_ptr;
		m_ptr = &floor_ptr->m_list[i];
		r_ptr = &r_info[m_ptr->r_idx];

		/* Handle "leaving" */
		if (target_ptr->leaving) break;

		/* Ignore "dead" monsters */
		if (!monster_is_valid(m_ptr)) continue;

		if (target_ptr->wild_mode) continue;


		/* Handle "fresh" monsters */
		if (m_ptr->mflag & MFLAG_BORN)
		{
			/* No longer "fresh" */
			m_ptr->mflag &= ~(MFLAG_BORN);

			/* Skip */
			continue;
		}

		/* Hack -- Require proximity */
		if (m_ptr->cdis >= AAF_LIMIT) continue;

		POSITION fx = m_ptr->fx;
		POSITION fy = m_ptr->fy;

		/* Flow by smell is allowed */
		if (!target_ptr->no_flowed)
		{
			m_ptr->mflag2 &= ~MFLAG2_NOFLOW;
		}

		/* Assume no move */
		test = FALSE;

		/* Handle "sensing radius" */
		if (m_ptr->cdis <= (is_pet(m_ptr) ? (r_ptr->aaf > MAX_SIGHT ? MAX_SIGHT : r_ptr->aaf) : r_ptr->aaf))
		{
			/* We can "sense" the player */
			test = TRUE;
		}

		/* Handle "sight" and "aggravation" */
		else if ((m_ptr->cdis <= MAX_SIGHT || target_ptr->phase_out) &&
			(player_has_los_bold(target_ptr, fy, fx) || (target_ptr->cursed & TRC_AGGRAVATE)))
		{
			/* We can "see" or "feel" the player */
			test = TRUE;
		}

		else if (m_ptr->target_y) test = TRUE;

		/* Do nothing */
		if (!test) continue;

		SPEED speed;
		if (target_ptr->riding == i)
			speed = target_ptr->pspeed;
		else
		{
			speed = m_ptr->mspeed;

			/* Monsters move quickly in Nightmare mode */
			if (ironman_nightmare) speed += 5;

			if (MON_FAST(m_ptr)) speed += 10;
			if (MON_SLOW(m_ptr)) speed -= 10;
		}

		/* Give this monster some energy */
		m_ptr->energy_need -= SPEED_TO_ENERGY(speed);

		/* Not enough energy to move */
		if (m_ptr->energy_need > 0) continue;

		/* Use up "some" energy */
		m_ptr->energy_need += ENERGY_NEED();

		/* Save global index */
		hack_m_idx = i;

		/* Process the monster */
		process_monster(target_ptr, i);

		reset_target(m_ptr);

		/* Give up flow_by_smell when it might useless */
		if (target_ptr->no_flowed && one_in_(3))
			m_ptr->mflag2 |= MFLAG2_NOFLOW;

		/* Hack -- notice death or departure */
		if (!target_ptr->playing || target_ptr->is_dead) break;

		/* Notice leaving */
		if (target_ptr->leaving) break;
	}

	/* Reset global index */
	hack_m_idx = 0;


	/* Tracking a monster race (the same one we were before) */
	if (!target_ptr->monster_race_idx || (target_ptr->monster_race_idx != old_monster_race_idx))
		return;

	/* Acquire monster race */
	monster_race *r_ptr;
	r_ptr = &r_info[target_ptr->monster_race_idx];

	/* Check for knowledge change */
	if ((old_r_flags1 != r_ptr->r_flags1) ||
		(old_r_flags2 != r_ptr->r_flags2) ||
		(old_r_flags3 != r_ptr->r_flags3) ||
		(old_r_flags4 != r_ptr->r_flags4) ||
		(old_r_flags5 != r_ptr->r_flags5) ||
		(old_r_flags6 != r_ptr->r_flags6) ||
		(old_r_flagsr != r_ptr->r_flagsr) ||
		(old_r_blows0 != r_ptr->r_blows[0]) ||
		(old_r_blows1 != r_ptr->r_blows[1]) ||
		(old_r_blows2 != r_ptr->r_blows[2]) ||
		(old_r_blows3 != r_ptr->r_blows[3]) ||
		(old_r_cast_spell != r_ptr->r_cast_spell))
	{
		target_ptr->window |= (PW_MONSTER);
	}
}
