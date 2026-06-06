/*! ***************************************************************************
 * \brief  Unit test for puzzle_engine — runs on PC with gcc
 * \file   test_puzzle.c
 *
 * Build:  gcc -o test_puzzle test_puzzle.c ../puzzle_engine.c ../config_store.c -I.. && ./test_puzzle
 * (from the tests/ directory)
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "config_store.h"
#include "puzzle_engine.h"

static int tests_passed = 0;

#define TEST(name) printf("  [TEST] %s ... ", name)
#define PASS()     do { printf("PASS\n"); tests_passed++; } while(0)

/* -------------------------------------------------------------------------
 * Test cases
 * ---------------------------------------------------------------------- */

static void test_init_no_puzzle_active(void)
{
    TEST("init: no puzzle active, nothing collected");

    config_store_t cfg;
    config_init(&cfg);

    puzzle_state_t puz;
    puzzle_init(&puz, &cfg);

    assert(puzzle_is_solved(&puz) == false);
    assert(strlen(puzzle_get_collected_code(&puz)) == 0);

    PASS();
}

static void test_start_sets_active_room(void)
{
    TEST("start(2) activates room 2's puzzle");

    config_store_t cfg;
    config_init(&cfg);

    puzzle_state_t puz;
    puzzle_init(&puz, &cfg);

    puzzle_start(&puz, 2);
    assert(puzzle_is_solved(&puz) == false);

    /* Room 2 default answer = "3" (answer[0] = '1' + 2 = '3') */
    bool ok = puzzle_check_answer(&puz, "3");
    assert(ok == true);
    assert(puzzle_is_solved(&puz) == true);

    PASS();
}

static void test_wrong_answer_no_progress(void)
{
    TEST("wrong answer → false, not solved, no digit collected (F2.3)");

    config_store_t cfg;
    config_init(&cfg);

    puzzle_state_t puz;
    puzzle_init(&puz, &cfg);

    puzzle_start(&puz, 0);  /* answer = "1" */

    bool ok = puzzle_check_answer(&puz, "9");
    assert(ok == false);
    assert(puzzle_is_solved(&puz) == false);
    assert(strlen(puzzle_get_collected_code(&puz)) == 0);

    PASS();
}

static void test_correct_answer_collects_digit(void)
{
    TEST("correct answer collects first char of answer as digit");

    config_store_t cfg;
    config_init(&cfg);

    puzzle_state_t puz;
    puzzle_init(&puz, &cfg);

    /* Solve room 0 (answer = "1") */
    puzzle_start(&puz, 0);
    puzzle_check_answer(&puz, "1");

    assert(puzzle_get_collected_digit(&puz, 0) == '1');
    assert(strcmp(puzzle_get_collected_code(&puz), "1") == 0);

    /* Solve room 1 (answer = "2") */
    puzzle_start(&puz, 1);
    puzzle_check_answer(&puz, "2");

    assert(puzzle_get_collected_digit(&puz, 1) == '2');
    assert(strcmp(puzzle_get_collected_code(&puz), "12") == 0);

    PASS();
}

static void test_full_route_collects_5_digits(void)
{
    TEST("solving all 5 rooms collects 5 digits");

    config_store_t cfg;
    config_init(&cfg);

    puzzle_state_t puz;
    puzzle_init(&puz, &cfg);

    /* Default answers: "1","2","3","4","5" */
    for (uint8_t i = 0; i < 5; i++)
    {
        puzzle_start(&puz, i);
        char ans[2] = { '1' + i, '\0' };
        bool ok = puzzle_check_answer(&puz, ans);
        assert(ok == true);
    }

    assert(strcmp(puzzle_get_collected_code(&puz), "12345") == 0);

    PASS();
}

static void test_final_code_matches_route_order(void)
{
    TEST("build_final_code follows route order (F2.4)");

    config_store_t cfg;
    config_init(&cfg);

    /* Custom route: visit rooms in order 4,3,2,1,0 */
    uint8_t custom_route[] = {4, 3, 2, 1, 0};
    config_set_route(&cfg, custom_route, 5);

    puzzle_state_t puz;
    puzzle_init(&puz, &cfg);

    char final_code[6];
    uint8_t len = puzzle_build_final_code(&puz, final_code);

    /* Route order {4,3,2,1,0} → answers[4]='5', answers[3]='4', etc. */
    assert(len == 5);
    assert(strcmp(final_code, "54321") == 0);

    PASS();
}

static void test_final_code_with_custom_answers(void)
{
    TEST("build_final_code uses first char of each room's answer");

    config_store_t cfg;
    config_init(&cfg);

    /* Set custom answers */
    config_room_t r0 = { .major = 0x0B01, .minor = 1, .answer = "ABC" };
    config_room_t r1 = { .major = 0x0B01, .minor = 2, .answer = "XYZ" };
    config_room_t r2 = { .major = 0x0B01, .minor = 3, .answer = "7up" };
    config_room_t r3 = { .major = 0x0B01, .minor = 4, .answer = "Go" };
    config_room_t r4 = { .major = 0x0B01, .minor = 5, .answer = "9" };
    config_set_room(&cfg, 0, &r0);
    config_set_room(&cfg, 1, &r1);
    config_set_room(&cfg, 2, &r2);
    config_set_room(&cfg, 3, &r3);
    config_set_room(&cfg, 4, &r4);

    puzzle_state_t puz;
    puzzle_init(&puz, &cfg);

    char final_code[6];
    puzzle_build_final_code(&puz, final_code);

    /* First chars: 'A', 'X', '7', 'G', '9' → "AX7G9" */
    assert(strcmp(final_code, "AX7G9") == 0);

    PASS();
}

static void test_check_without_start_fails(void)
{
    TEST("check_answer without puzzle_start returns false");

    config_store_t cfg;
    config_init(&cfg);

    puzzle_state_t puz;
    puzzle_init(&puz, &cfg);

    /* No puzzle_start called */
    bool ok = puzzle_check_answer(&puz, "1");
    assert(ok == false);

    PASS();
}

static void test_reset_clears_everything(void)
{
    TEST("reset clears collected digits and active puzzle");

    config_store_t cfg;
    config_init(&cfg);

    puzzle_state_t puz;
    puzzle_init(&puz, &cfg);

    /* Solve 2 rooms */
    puzzle_start(&puz, 0);
    puzzle_check_answer(&puz, "1");
    puzzle_start(&puz, 1);
    puzzle_check_answer(&puz, "2");
    assert(strcmp(puzzle_get_collected_code(&puz), "12") == 0);

    /* Reset */
    puzzle_reset(&puz);
    assert(strlen(puzzle_get_collected_code(&puz)) == 0);
    assert(puzzle_is_solved(&puz) == false);
    assert(puzzle_get_collected_digit(&puz, 0) == '\0');

    PASS();
}

static void test_null_safety(void)
{
    TEST("all functions handle NULL gracefully");

    assert(puzzle_is_solved(NULL) == false);
    assert(puzzle_get_collected_digit(NULL, 0) == '\0');
    assert(strcmp(puzzle_get_collected_code(NULL), "") == 0);
    assert(puzzle_check_answer(NULL, "x") == false);

    puzzle_state_t puz;
    puzzle_init(&puz, NULL);   /* should not crash */
    puzzle_init(NULL, NULL);   /* should not crash */
    puzzle_start(NULL, 0);     /* should not crash */
    puzzle_reset(NULL);        /* should not crash */

    char buf[6];
    assert(puzzle_build_final_code(NULL, buf) == 0);

    PASS();
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void)
{
    printf("\n=== puzzle_engine unit tests ===\n\n");

    test_init_no_puzzle_active();
    test_start_sets_active_room();
    test_wrong_answer_no_progress();
    test_correct_answer_collects_digit();
    test_full_route_collects_5_digits();
    test_final_code_matches_route_order();
    test_final_code_with_custom_answers();
    test_check_without_start_fails();
    test_reset_clears_everything();
    test_null_safety();

    printf("\n=== %d / 10 tests PASSED ===\n\n", tests_passed);

    return (tests_passed == 10) ? 0 : 1;
}
