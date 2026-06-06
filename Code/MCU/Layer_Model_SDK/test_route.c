/*! ***************************************************************************
 * \brief  Unit test for route_manager — runs on PC with gcc
 * \file   test_route.c
 *
 * Build:  gcc -o test_route test_route.c ../route_manager.c ../config_store.c -I.. && ./test_route
 * (from the tests/ directory)
 *****************************************************************************/
#include <stdio.h>
#include <assert.h>
#include "config_store.h"
#include "route_manager.h"

static int tests_passed = 0;

#define TEST(name) printf("  [TEST] %s ... ", name)
#define PASS()     do { printf("PASS\n"); tests_passed++; } while(0)

/* -------------------------------------------------------------------------
 * Test cases
 * ---------------------------------------------------------------------- */

static void test_init_progress_zero(void)
{
    TEST("init sets progress to 0");

    config_store_t cfg;
    config_init(&cfg);

    route_state_t state;
    route_init(&state, &cfg);

    assert(route_get_progress(&state) == 0);
    assert(route_is_complete(&state) == false);

    PASS();
}

static void test_init_target_is_first_room(void)
{
    TEST("init target is route[0] = room index 0");

    config_store_t cfg;
    config_init(&cfg);

    route_state_t state;
    route_init(&state, &cfg);

    /* Default route is {0,1,2,3,4}, so first target = 0 */
    assert(route_get_current_target(&state) == 0);

    PASS();
}

static void test_advance_increments_progress(void)
{
    TEST("advance increments k from 0 to 1");

    config_store_t cfg;
    config_init(&cfg);

    route_state_t state;
    route_init(&state, &cfg);

    bool complete = route_advance(&state);
    assert(complete == false);
    assert(route_get_progress(&state) == 1);
    assert(route_get_current_target(&state) == 1);

    PASS();
}

static void test_advance_through_all_rooms(void)
{
    TEST("advance 5 times → complete, target = 0xFF");

    config_store_t cfg;
    config_init(&cfg);

    route_state_t state;
    route_init(&state, &cfg);

    /* Advance through all 5 rooms */
    assert(route_advance(&state) == false);  /* k=1, not done */
    assert(route_advance(&state) == false);  /* k=2 */
    assert(route_advance(&state) == false);  /* k=3 */
    assert(route_advance(&state) == false);  /* k=4 */
    assert(route_advance(&state) == true);   /* k=5, COMPLETE */

    assert(route_get_progress(&state) == 5);
    assert(route_is_complete(&state) == true);
    assert(route_get_current_target(&state) == 0xFF);

    PASS();
}

static void test_advance_past_complete_stays_complete(void)
{
    TEST("advance past complete doesn't overflow");

    config_store_t cfg;
    config_init(&cfg);

    route_state_t state;
    route_init(&state, &cfg);

    for (int i = 0; i < 5; i++) route_advance(&state);

    /* Already complete — advance again should stay at 5 */
    bool result = route_advance(&state);
    assert(result == true);
    assert(route_get_progress(&state) == 5);

    PASS();
}

static void test_custom_route_order(void)
{
    TEST("custom route {4,2,0,3,1} → targets follow that order");

    config_store_t cfg;
    config_init(&cfg);

    uint8_t custom[] = {4, 2, 0, 3, 1};
    config_set_route(&cfg, custom, 5);

    route_state_t state;
    route_init(&state, &cfg);

    assert(route_get_current_target(&state) == 4);
    route_advance(&state);
    assert(route_get_current_target(&state) == 2);
    route_advance(&state);
    assert(route_get_current_target(&state) == 0);
    route_advance(&state);
    assert(route_get_current_target(&state) == 3);
    route_advance(&state);
    assert(route_get_current_target(&state) == 1);
    route_advance(&state);
    assert(route_get_current_target(&state) == 0xFF);

    PASS();
}

static void test_reset_clears_progress(void)
{
    TEST("reset brings progress back to 0");

    config_store_t cfg;
    config_init(&cfg);

    route_state_t state;
    route_init(&state, &cfg);

    route_advance(&state);
    route_advance(&state);
    assert(route_get_progress(&state) == 2);

    route_reset(&state);
    assert(route_get_progress(&state) == 0);
    assert(route_get_current_target(&state) == 0);
    assert(route_is_complete(&state) == false);

    PASS();
}

static void test_null_safety(void)
{
    TEST("all functions handle NULL gracefully");

    assert(route_get_current_target(NULL) == 0xFF);
    assert(route_get_progress(NULL) == 0);
    assert(route_is_complete(NULL) == false);
    assert(route_advance(NULL) == false);

    route_state_t state;
    route_init(&state, NULL);  /* should not crash */
    route_init(NULL, NULL);    /* should not crash */
    route_reset(NULL);         /* should not crash */

    PASS();
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void)
{
    printf("\n=== route_manager unit tests ===\n\n");

    test_init_progress_zero();
    test_init_target_is_first_room();
    test_advance_increments_progress();
    test_advance_through_all_rooms();
    test_advance_past_complete_stays_complete();
    test_custom_route_order();
    test_reset_clears_progress();
    test_null_safety();

    printf("\n=== %d / 8 tests PASSED ===\n\n", tests_passed);

    return (tests_passed == 8) ? 0 : 1;
}
