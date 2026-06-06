/*! ***************************************************************************
 * \brief  puzzle_registry.c — wraps Khanh's 4 games as puzzle plugins
 * \file   puzzle/puzzle_registry.c
 *
 * Each of Khanh's games already exposes init / input_detect / *_game_tick.
 * Here we wrap each behind the uniform puzzle_plugin_t contract so the FSM
 * drives them identically. Khanh's main.c game-chaining is NOT used — the
 * KidBytes FSM (Layer 5) chains rooms instead, per the functional requirements.
 *
 * Simon Says adapter: its game_tick() returns void and loops forever, and
 * input_detect() is named differently. We add a thin solved-flag wrapper.
 *****************************************************************************/
#include "puzzle_registry.h"

#include "rps.h"
#include "quiz.h"
#include "mole.h"
#include "start.h"   /* Simon Says */

/* -------------------------------------------------------------------------
 * RPS plugin — direct contract match
 * ---------------------------------------------------------------------- */
static void rps_plugin_start(void) { rps_init(); }
static void rps_plugin_input(void) { rps_input_detect(); }
static bool rps_plugin_tick(void)  { return rps_game_tick(); }

/* -------------------------------------------------------------------------
 * QUIZ plugin — direct contract match
 * ---------------------------------------------------------------------- */
static void quiz_plugin_start(void) { quiz_init(); }
static void quiz_plugin_input(void) { quiz_input_detect(); }
static bool quiz_plugin_tick(void)  { return quiz_game_tick(); }

/* -------------------------------------------------------------------------
 * MOLE plugin — direct contract match
 * ---------------------------------------------------------------------- */
static void mole_plugin_start(void) { mole_init(); }
static void mole_plugin_input(void) { mole_input_detect(); }
static bool mole_plugin_tick(void)  { return mole_game_tick(); }

/* -------------------------------------------------------------------------
 * SIMON plugin — adapter (Simon's game_tick is void + endless)
 *
 * Simon Says shows a 4-color pattern then asks the kid to repeat it.
 * Khanh's game_tick() loops the round forever. We wrap it: the puzzle is
 * "solved" the first time the kid reproduces the pattern correctly. We
 * detect that by watching for the GAME_WIN screen via a solved flag set
 * inside the adapter's own copy of the tick logic.
 *
 * Solved detection: start.c sets the shared flag g_simon_solved = true in its
 * GAME_WIN case (the 1-line hook). simon_plugin_start() clears it; tick()
 * reports it. Until the kid reproduces the pattern, tick() returns false so
 * the FSM stays in PUZZLE (F2.3 — no false progress).
 * ---------------------------------------------------------------------- */
/* Shared flag — set by start.c GAME_WIN case, cleared on puzzle start. */
bool g_simon_solved = false;

static void simon_plugin_start(void)
{
    g_simon_solved = false;
    start_screen();
    generate_pattern();
}

static void simon_plugin_input(void)
{
    input_detect();
}

static bool simon_plugin_tick(void)
{
    /* Drive Khanh's Simon round. game_tick() auto-restarts on win/lose;
     * start.c flips g_simon_solved on GAME_WIN, which we report here. */
    game_tick();
    return g_simon_solved;
}

/* -------------------------------------------------------------------------
 * Registry table — the ONLY place rooms map to puzzles
 * ---------------------------------------------------------------------- */
static const puzzle_plugin_t g_puzzles[] = {
    { "RPS",   rps_plugin_start,   rps_plugin_input,   rps_plugin_tick   },
    { "QUIZ",  quiz_plugin_start,  quiz_plugin_input,  quiz_plugin_tick  },
    { "MOLE",  mole_plugin_start,  mole_plugin_input,  mole_plugin_tick  },
    { "SIMON", simon_plugin_start, simon_plugin_input, simon_plugin_tick },
    { "QUIZ2", quiz_plugin_start,  quiz_plugin_input,  quiz_plugin_tick  },
};

#define PUZZLE_COUNT  (sizeof(g_puzzles) / sizeof(g_puzzles[0]))

const puzzle_plugin_t *puzzle_registry_get(uint8_t room_index)
{
    if (room_index >= PUZZLE_COUNT) return 0;
    return &g_puzzles[room_index];
}

uint8_t puzzle_registry_count(void)
{
    return (uint8_t)PUZZLE_COUNT;
}
