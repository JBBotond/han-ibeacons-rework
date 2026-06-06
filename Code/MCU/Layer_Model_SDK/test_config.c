/*! ***************************************************************************
 * \brief  Unit test for config_store — runs on PC with gcc
 * \file   test_config.c
 *
 * Build:  gcc -o test_config test_config.c ../config_store.c -I.. && ./test_config
 * (from the tests/ directory)
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "config_store.h"

static int tests_passed = 0;

#define TEST(name) printf("  [TEST] %s ... ", name)
#define PASS()     do { printf("PASS\n"); tests_passed++; } while(0)

/* -------------------------------------------------------------------------
 * Test cases
 * ---------------------------------------------------------------------- */

static void test_init_defaults(void)
{
    TEST("init loads 5 rooms with correct Major/Minor");

    config_store_t cfg;
    config_init(&cfg);

    assert(config_get_room_count(&cfg) == 5);

    for (uint8_t i = 0; i < 5; i++)
    {
        const config_room_t *r = config_get_room(&cfg, i);
        assert(r != NULL);
        assert(r->major == 0x0B01);
        assert(r->minor == (uint16_t)(i + 1));
    }

    PASS();
}

static void test_init_route_sequential(void)
{
    TEST("init route is sequential 0,1,2,3,4");

    config_store_t cfg;
    config_init(&cfg);

    const uint8_t *route = config_get_route(&cfg);
    assert(route != NULL);
    for (uint8_t i = 0; i < 5; i++)
    {
        assert(route[i] == i);
    }

    PASS();
}

static void test_init_secret(void)
{
    TEST("init secret is ADMIN");

    config_store_t cfg;
    config_init(&cfg);

    const char *s = config_get_secret(&cfg);
    assert(s != NULL);
    assert(strcmp(s, "ADMIN") == 0);

    PASS();
}

static void test_init_answers(void)
{
    TEST("init puzzle answers are 1,2,3,4,5");

    config_store_t cfg;
    config_init(&cfg);

    for (uint8_t i = 0; i < 5; i++)
    {
        const config_room_t *r = config_get_room(&cfg, i);
        char expected[2] = { '1' + i, '\0' };
        assert(strcmp(r->answer, expected) == 0);
    }

    PASS();
}

static void test_get_room_out_of_bounds(void)
{
    TEST("get_room returns NULL for index >= room_count");

    config_store_t cfg;
    config_init(&cfg);

    assert(config_get_room(&cfg, 5) == NULL);
    assert(config_get_room(&cfg, 255) == NULL);

    PASS();
}

static void test_set_room_valid(void)
{
    TEST("set_room updates Major/Minor/answer");

    config_store_t cfg;
    config_init(&cfg);

    config_room_t new_room = { .major = 0x0B02, .minor = 0x000A, .answer = "42" };
    bool ok = config_set_room(&cfg, 2, &new_room);
    assert(ok == true);

    const config_room_t *r = config_get_room(&cfg, 2);
    assert(r->major == 0x0B02);
    assert(r->minor == 0x000A);
    assert(strcmp(r->answer, "42") == 0);

    /* Other rooms unchanged */
    const config_room_t *r0 = config_get_room(&cfg, 0);
    assert(r0->major == 0x0B01);
    assert(r0->minor == 0x0001);

    PASS();
}

static void test_set_room_out_of_bounds(void)
{
    TEST("set_room rejects index >= room_count");

    config_store_t cfg;
    config_init(&cfg);

    config_room_t new_room = { .major = 0xFFFF, .minor = 0xFFFF, .answer = "X" };
    bool ok = config_set_room(&cfg, 5, &new_room);
    assert(ok == false);

    PASS();
}

static void test_set_secret_valid(void)
{
    TEST("set_secret updates the secret");

    config_store_t cfg;
    config_init(&cfg);

    bool ok = config_set_secret(&cfg, "NEWPASS");
    assert(ok == true);
    assert(strcmp(config_get_secret(&cfg), "NEWPASS") == 0);

    PASS();
}

static void test_set_secret_too_long(void)
{
    TEST("set_secret rejects string > CONFIG_SECRET_LEN");

    config_store_t cfg;
    config_init(&cfg);

    bool ok = config_set_secret(&cfg, "TOOLONGPASSWORD");
    assert(ok == false);
    /* Secret unchanged */
    assert(strcmp(config_get_secret(&cfg), "ADMIN") == 0);

    PASS();
}

static void test_set_route_valid(void)
{
    TEST("set_route accepts valid reordering");

    config_store_t cfg;
    config_init(&cfg);

    uint8_t new_order[] = {4, 3, 2, 1, 0};
    bool ok = config_set_route(&cfg, new_order, 5);
    assert(ok == true);

    const uint8_t *route = config_get_route(&cfg);
    assert(route[0] == 4);
    assert(route[4] == 0);

    PASS();
}

static void test_set_route_invalid_index(void)
{
    TEST("set_route rejects index >= room_count");

    config_store_t cfg;
    config_init(&cfg);

    uint8_t bad_order[] = {0, 1, 2, 3, 7};  /* 7 is out of range */
    bool ok = config_set_route(&cfg, bad_order, 5);
    assert(ok == false);

    PASS();
}

static void test_null_safety(void)
{
    TEST("all functions handle NULL gracefully");

    assert(config_get_room_count(NULL) == 0);
    assert(config_get_room(NULL, 0) == NULL);
    assert(config_get_route(NULL) == NULL);
    assert(config_get_secret(NULL) == NULL);
    assert(config_set_room(NULL, 0, NULL) == false);
    assert(config_set_secret(NULL, "X") == false);
    assert(config_set_route(NULL, NULL, 0) == false);

    PASS();
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void)
{
    printf("\n=== config_store unit tests ===\n\n");

    test_init_defaults();
    test_init_route_sequential();
    test_init_secret();
    test_init_answers();
    test_get_room_out_of_bounds();
    test_set_room_valid();
    test_set_room_out_of_bounds();
    test_set_secret_valid();
    test_set_secret_too_long();
    test_set_route_valid();
    test_set_route_invalid_index();
    test_null_safety();

    printf("\n=== %d / 12 tests PASSED ===\n\n", tests_passed);

    return (tests_passed == 12) ? 0 : 1;
}
