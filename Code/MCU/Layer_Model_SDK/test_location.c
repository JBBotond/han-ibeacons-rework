/*! ***************************************************************************
 * \brief  Unit test for location_engine — runs on PC with gcc
 * \file   test_location.c
 *
 * Build:  gcc -o test_location test_location.c ../location_engine.c ../config_store.c -I.. && ./test_location
 * (from the tests/ directory)
 *
 * Tests replay the exact scan data captured from the real HM-10:
 *   OK+DISC:4C000215:74278BDAB64445208F0C720EAF059935:0B010002C5:685E1C1C31BE:-066
 *   → Major=0x0B01, Minor=0x0002, RSSI=-66
 *****************************************************************************/
#include <stdio.h>
#include <assert.h>
#include "config_store.h"
#include "location_engine.h"

static int tests_passed = 0;

#define TEST(name) printf("  [TEST] %s ... ", name)
#define PASS()     do { printf("PASS\n"); tests_passed++; } while(0)

/* -------------------------------------------------------------------------
 * Test cases
 * ---------------------------------------------------------------------- */

static void test_init_no_room(void)
{
    TEST("init: no room detected, signal not low");

    config_store_t cfg;
    config_init(&cfg);

    loc_state_t loc;
    loc_init(&loc, &cfg);

    assert(loc_get_room(&loc) == LOC_NO_ROOM);
    assert(loc_is_signal_low(&loc) == false);

    PASS();
}

static void test_single_beacon_detected(void)
{
    TEST("feed beacon Minor=0x0002 → room index 1");

    config_store_t cfg;
    config_init(&cfg);
    /* Default: rooms[0]=Minor 1, rooms[1]=Minor 2, ... */

    loc_state_t loc;
    loc_init(&loc, &cfg);

    /* Simulate one scan with one beacon: Major=0x0B01, Minor=0x0002, RSSI=-66 */
    loc_feed_beacon(&loc, 0x0B01, 0x0002, -66);
    loc_end_scan(&loc);

    assert(loc_get_room(&loc) == 1);  /* rooms[1] has Minor=0x0002 */
    assert(loc_get_rssi(&loc) != 0);  /* some filtered value */
    assert(loc_is_signal_low(&loc) == false);

    PASS();
}

static void test_rssi_filter_converges(void)
{
    TEST("RSSI filter: 3 scans at -60 → converges toward -60");

    config_store_t cfg;
    config_init(&cfg);

    loc_state_t loc;
    loc_init(&loc, &cfg);

    /* Feed same beacon 3 times (3 scan cycles) */
    for (int i = 0; i < 3; i++)
    {
        loc_feed_beacon(&loc, 0x0B01, 0x0001, -60);
        loc_end_scan(&loc);
    }

    /* IIR with α=0.5 starting from -100:
       scan 1: (-60 + -100)/2 = -80
       scan 2: (-60 + -80)/2  = -70
       scan 3: (-60 + -70)/2  = -65
    */
    int8_t rssi = loc_get_rssi(&loc);
    assert(rssi >= -66 && rssi <= -64);  /* should be -65 */

    PASS();
}

static void test_multi_beacon_selects_strongest(void)
{
    TEST("two beacons in range → selects highest filtered RSSI (F1.4)");

    config_store_t cfg;
    config_init(&cfg);

    loc_state_t loc;
    loc_init(&loc, &cfg);

    /* Prime both rooms with a few scans so filter stabilizes */
    for (int i = 0; i < 4; i++)
    {
        loc_feed_beacon(&loc, 0x0B01, 0x0001, -70);  /* room 0: weaker */
        loc_feed_beacon(&loc, 0x0B01, 0x0003, -50);  /* room 2: stronger */
        loc_end_scan(&loc);
    }

    /* Room 2 (Minor=3) should win because -50 > -70 */
    assert(loc_get_room(&loc) == 2);

    PASS();
}

static void test_signal_low_after_3_misses(void)
{
    TEST("3 empty scans → signal_low = true (F1.3)");

    config_store_t cfg;
    config_init(&cfg);

    loc_state_t loc;
    loc_init(&loc, &cfg);

    /* First: detect a room */
    loc_feed_beacon(&loc, 0x0B01, 0x0001, -60);
    loc_end_scan(&loc);
    assert(loc_get_room(&loc) == 0);
    assert(loc_is_signal_low(&loc) == false);

    /* Now 3 empty scans (no beacons fed) */
    loc_end_scan(&loc);  /* miss 1 */
    assert(loc_is_signal_low(&loc) == false);

    loc_end_scan(&loc);  /* miss 2 */
    assert(loc_is_signal_low(&loc) == false);

    loc_end_scan(&loc);  /* miss 3 → SIGNAL_LOW */
    assert(loc_is_signal_low(&loc) == true);

    /* Room is RETAINED (F1.3) */
    assert(loc_get_room(&loc) == 0);

    PASS();
}

static void test_signal_recovers_after_detection(void)
{
    TEST("signal_low clears when beacon seen again");

    config_store_t cfg;
    config_init(&cfg);

    loc_state_t loc;
    loc_init(&loc, &cfg);

    /* Detect, then lose signal */
    loc_feed_beacon(&loc, 0x0B01, 0x0001, -60);
    loc_end_scan(&loc);
    loc_end_scan(&loc);
    loc_end_scan(&loc);
    loc_end_scan(&loc);  /* 3 misses → signal_low */
    assert(loc_is_signal_low(&loc) == true);

    /* Beacon comes back */
    loc_feed_beacon(&loc, 0x0B01, 0x0001, -55);
    loc_end_scan(&loc);
    assert(loc_is_signal_low(&loc) == false);
    assert(loc_get_room(&loc) == 0);

    PASS();
}

static void test_unknown_beacon_ignored(void)
{
    TEST("beacon with unknown Major/Minor is ignored");

    config_store_t cfg;
    config_init(&cfg);

    loc_state_t loc;
    loc_init(&loc, &cfg);

    /* Feed a beacon that doesn't match any configured room */
    loc_feed_beacon(&loc, 0xFFFF, 0xFFFF, -40);
    loc_end_scan(&loc);

    assert(loc_get_room(&loc) == LOC_NO_ROOM);

    PASS();
}

static void test_rssi_weak_threshold(void)
{
    TEST("RSSI below -85 → rssi_weak = true (F3.5)");

    config_store_t cfg;
    config_init(&cfg);

    loc_state_t loc;
    loc_init(&loc, &cfg);

    /* Feed beacon with very weak signal, let filter converge */
    for (int i = 0; i < 10; i++)
    {
        loc_feed_beacon(&loc, 0x0B01, 0x0001, -90);
        loc_end_scan(&loc);
    }

    assert(loc_get_room(&loc) == 0);
    assert(loc_is_rssi_weak(&loc) == true);

    PASS();
}

static void test_real_scan_data(void)
{
    TEST("replay real HM-10 capture: Major=0x0B01 Minor=0x0002 RSSI=-66");

    config_store_t cfg;
    config_init(&cfg);

    loc_state_t loc;
    loc_init(&loc, &cfg);

    /* Exact values from your bench capture:
       OK+DISC:4C000215:74278BDAB64445208F0C720EAF059935:0B010002C5:685E1C1C31BE:-066
       Major=0x0B01, Minor=0x0002, RSSI=-66 */
    loc_feed_beacon(&loc, 0x0B01, 0x0002, -66);
    loc_end_scan(&loc);

    /* rooms[1] has Minor=0x0002 in default config */
    assert(loc_get_room(&loc) == 1);
    assert(loc_is_signal_low(&loc) == false);

    PASS();
}

static void test_null_safety(void)
{
    TEST("all functions handle NULL gracefully");

    assert(loc_get_room(NULL) == LOC_NO_ROOM);
    assert(loc_get_rssi(NULL) == 0);
    assert(loc_is_signal_low(NULL) == false);
    assert(loc_is_rssi_weak(NULL) == false);

    loc_state_t loc;
    loc_init(&loc, NULL);   /* should not crash */
    loc_init(NULL, NULL);   /* should not crash */
    loc_feed_beacon(NULL, 0, 0, 0);  /* should not crash */
    loc_end_scan(NULL);     /* should not crash */

    PASS();
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void)
{
    printf("\n=== location_engine unit tests ===\n\n");

    test_init_no_room();
    test_single_beacon_detected();
    test_rssi_filter_converges();
    test_multi_beacon_selects_strongest();
    test_signal_low_after_3_misses();
    test_signal_recovers_after_detection();
    test_unknown_beacon_ignored();
    test_rssi_weak_threshold();
    test_real_scan_data();
    test_null_safety();

    printf("\n=== %d / 10 tests PASSED ===\n\n", tests_passed);

    return (tests_passed == 10) ? 0 : 1;
}
