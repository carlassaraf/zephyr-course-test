/*
 * Ring Buffer Module - Homework Test Skeleton
 *
 * test_fresh_state is provided as a worked example. Fill in the remaining
 * 7 ZTEST bodies according to TEST_SPEC.md. Stubs call ztest_test_skip()
 * so the binary builds and runs cleanly before each test is implemented.
 *
 * Run:
 *   west twister -T tests/ring_buf -p native_sim
 */

#include <zephyr/ztest.h>
#include <errno.h>

#include "ring_buf.h"

/*
 * Shared before hook: every suite reinitialises the ring buffer with a
 * capacity of 4 so tests start from a clean, known state. Capacity 4 is
 * enough to exercise FIFO order (push 1, 2, 3) and overflow (full at 4).
 */
static void before(void *f)
{
	ARG_UNUSED(f);
	rb_init(4);
}

/*
 * ============================================================================
 * Test Suite: ring_buf_init
 *
 * Initial state and re-initialization behaviour.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_init, NULL, NULL, before, NULL, NULL);

/* PROVIDED — study this test before writing the rest. */
ZTEST(ring_buf_init, test_fresh_state)
{
	zassert_true(rb_is_empty(), "Fresh buffer must be empty");
	zassert_equal(rb_count(), 0, "Fresh buffer count must be 0");
}

ZTEST(ring_buf_init, test_reinit_clears_state)
{
	zassert_ok(rb_push(1));
	zassert_ok(rb_init(4));
	zassert_true(rb_is_empty(), "Re-init must empty the buffer");
	zassert_equal(rb_count(), 0, "Re-init must reset the count");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_push_pop
 *
 * Single push/pop round-trip, FIFO order, full error path.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_push_pop, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_push_pop, test_single_push_pop)
{
	int v = 0;

	zassert_ok(rb_push(42));
	zassert_ok(rb_pop(&v));
	zassert_equal(v, 42, "Popped value must be the pushed one");
	zassert_true(rb_is_empty(), "Buffer must be empty after the pop");
}

ZTEST(ring_buf_push_pop, test_fifo_order)
{
	int v = 0;

	zassert_ok(rb_push(1));
	zassert_ok(rb_push(2));
	zassert_ok(rb_push(3));
	for (int expected = 1; expected <= 3; expected++) {
		zassert_ok(rb_pop(&v));
		zassert_equal(v, expected, "Values must come out in FIFO order");
	}
}

ZTEST(ring_buf_push_pop, test_push_full_returns_enospc)
{
	for (int i = 0; i < 4; i++) {
		zassert_ok(rb_push(i));
	}
	zassert_equal(rb_push(99), -ENOSPC, "Push on a full buffer must fail with -ENOSPC");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_boundaries
 *
 * Peek semantics and NULL-pointer boundary conditions.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_boundaries, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_boundaries, test_peek_does_not_consume)
{
	int v = 0;

	zassert_ok(rb_push(7));
	zassert_ok(rb_peek(&v));
	zassert_equal(v, 7, "Peek must return the oldest value");
	zassert_ok(rb_peek(&v));
	zassert_equal(v, 7, "A second peek must return the same value");
	zassert_equal(rb_count(), 1, "Peek must not consume the value");
}

ZTEST(ring_buf_boundaries, test_pop_null_returns_einval)
{
	zassert_equal(rb_pop(NULL), -EINVAL, "Pop into NULL must fail with -EINVAL");
}

ZTEST(ring_buf_boundaries, test_is_full_after_fill)
{
	for (int i = 0; i < 4; i++) {
		zassert_ok(rb_push(i));
	}
	zassert_true(rb_is_full(), "Buffer must be full after 4 pushes");
	zassert_equal(rb_count(), 4, "Count must be 4 after 4 pushes");
}
