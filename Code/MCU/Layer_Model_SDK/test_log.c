/*! ***************************************************************************
 * \brief  Unit test for log_model — runs on PC with gcc
 * \file   test_log.c
 *
 * Build:  gcc -o test_log test_log.c ../log_model.c -I.. && ./test_log
 * (from the tests/ directory)
 *****************************************************************************/
#include <stdio.h>
#include <assert.h>
#include "log_model.h"

static int tests_passed = 0;

#define TEST(name) printf("  [TEST] %s ... ", name)
#define PASS()     do { printf("PASS\n"); tests_passed++; } while(0)

/* -------------------------------------------------------------------------
 * Test cases
 * ---------------------------------------------------------------------- */

static void test_init_empty(void)
{
    TEST("init: count = 0, get_latest = NULL");

    log_state_t log;
    logm_init(&log);

    assert(logm_get_count(&log) == 0);
    assert(logm_get_latest(&log) == NULL);
    assert(logm_get_entry(&log, 0) == NULL);

    PASS();
}

static void test_append_one(void)
{
    TEST("append 1 entry → count = 1, fields correct");

    log_state_t log;
    logm_init(&log);

    logm_append(&log, 2, 1000, LOG_OUTCOME_ENTER);

    assert(logm_get_count(&log) == 1);

    const log_entry_t *e = logm_get_entry(&log, 0);
    assert(e != NULL);
    assert(e->room_id == 2);
    assert(e->timestamp_ms == 1000);
    assert(e->outcome == LOG_OUTCOME_ENTER);

    PASS();
}

static void test_append_multiple_ordered(void)
{
    TEST("append 3 entries → oldest first, newest last");

    log_state_t log;
    logm_init(&log);

    logm_append(&log, 0, 100, LOG_OUTCOME_ENTER);
    logm_append(&log, 0, 500, LOG_OUTCOME_SOLVED);
    logm_append(&log, 1, 800, LOG_OUTCOME_ENTER);

    assert(logm_get_count(&log) == 3);

    const log_entry_t *oldest = logm_get_entry(&log, 0);
    assert(oldest->timestamp_ms == 100);

    const log_entry_t *newest = logm_get_latest(&log);
    assert(newest->timestamp_ms == 800);

    PASS();
}

static void test_ring_buffer_wraps(void)
{
    TEST("append LOG_MAX_ENTRIES+5 → count stays at max, oldest overwritten");

    log_state_t log;
    logm_init(&log);

    /* Fill buffer + 5 extra */
    for (uint32_t i = 0; i < LOG_MAX_ENTRIES + 5; i++)
    {
        logm_append(&log, (uint8_t)(i % 5), i * 100, LOG_OUTCOME_ENTER);
    }

    assert(logm_get_count(&log) == LOG_MAX_ENTRIES);

    /* Oldest should be entry #5 (first 5 were overwritten) */
    const log_entry_t *oldest = logm_get_entry(&log, 0);
    assert(oldest != NULL);
    assert(oldest->timestamp_ms == 500);  /* entry index 5 → 5*100 = 500 */

    /* Newest should be the last one written */
    const log_entry_t *newest = logm_get_latest(&log);
    assert(newest != NULL);
    assert(newest->timestamp_ms == (LOG_MAX_ENTRIES + 4) * 100);

    PASS();
}

static void test_session_duration(void)
{
    TEST("session duration = newest.timestamp - oldest.timestamp");

    log_state_t log;
    logm_init(&log);

    logm_append(&log, 0, 1000, LOG_OUTCOME_ENTER);
    logm_append(&log, 0, 5000, LOG_OUTCOME_SOLVED);
    logm_append(&log, 1, 8000, LOG_OUTCOME_ENTER);
    logm_append(&log, 1, 12000, LOG_OUTCOME_SOLVED);
    logm_append(&log, 0xFF, 15000, LOG_OUTCOME_FINAL);

    uint32_t duration = logm_get_session_duration_ms(&log);
    assert(duration == 14000);  /* 15000 - 1000 */

    PASS();
}

static void test_room_time(void)
{
    TEST("room_time = SOLVED.timestamp - ENTER.timestamp for that room");

    log_state_t log;
    logm_init(&log);

    logm_append(&log, 0, 1000, LOG_OUTCOME_ENTER);
    logm_append(&log, 0, 4500, LOG_OUTCOME_SOLVED);
    logm_append(&log, 1, 5000, LOG_OUTCOME_ENTER);
    logm_append(&log, 1, 9000, LOG_OUTCOME_SOLVED);

    assert(logm_get_room_time_ms(&log, 0) == 3500);  /* 4500 - 1000 */
    assert(logm_get_room_time_ms(&log, 1) == 4000);  /* 9000 - 5000 */
    assert(logm_get_room_time_ms(&log, 2) == 0);     /* not visited */

    PASS();
}

static void test_room_time_with_failed_attempts(void)
{
    TEST("room_time ignores FAILED events between ENTER and SOLVED");

    log_state_t log;
    logm_init(&log);

    logm_append(&log, 0, 1000, LOG_OUTCOME_ENTER);
    logm_append(&log, 0, 2000, LOG_OUTCOME_FAILED);  /* wrong answer */
    logm_append(&log, 0, 3000, LOG_OUTCOME_FAILED);  /* wrong again */
    logm_append(&log, 0, 5000, LOG_OUTCOME_SOLVED);  /* got it */

    /* Time = SOLVED - ENTER = 5000 - 1000 = 4000 */
    assert(logm_get_room_time_ms(&log, 0) == 4000);

    PASS();
}

static void test_clear(void)
{
    TEST("clear resets count to 0");

    log_state_t log;
    logm_init(&log);

    logm_append(&log, 0, 100, LOG_OUTCOME_ENTER);
    logm_append(&log, 0, 200, LOG_OUTCOME_SOLVED);
    assert(logm_get_count(&log) == 2);

    logm_clear(&log);
    assert(logm_get_count(&log) == 0);
    assert(logm_get_latest(&log) == NULL);

    PASS();
}

static void test_get_entry_out_of_bounds(void)
{
    TEST("get_entry returns NULL for index >= count");

    log_state_t log;
    logm_init(&log);

    logm_append(&log, 0, 100, LOG_OUTCOME_ENTER);

    assert(logm_get_entry(&log, 0) != NULL);
    assert(logm_get_entry(&log, 1) == NULL);
    assert(logm_get_entry(&log, 999) == NULL);

    PASS();
}

static void test_null_safety(void)
{
    TEST("all functions handle NULL gracefully");

    assert(logm_get_count(NULL) == 0);
    assert(logm_get_entry(NULL, 0) == NULL);
    assert(logm_get_latest(NULL) == NULL);
    assert(logm_get_session_duration_ms(NULL) == 0);
    assert(logm_get_room_time_ms(NULL, 0) == 0);

    logm_init(NULL);     /* should not crash */
    logm_append(NULL, 0, 0, 0);  /* should not crash */
    logm_clear(NULL);    /* should not crash */

    PASS();
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void)
{
    printf("\n=== log_model unit tests ===\n\n");

    test_init_empty();
    test_append_one();
    test_append_multiple_ordered();
    test_ring_buffer_wraps();
    test_session_duration();
    test_room_time();
    test_room_time_with_failed_attempts();
    test_clear();
    test_get_entry_out_of_bounds();
    test_null_safety();

    printf("\n=== %d / 10 tests PASSED ===\n\n", tests_passed);

    return (tests_passed == 10) ? 0 : 1;
}
