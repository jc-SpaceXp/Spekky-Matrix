#include "greatest.h"
#include "suite_001.h"

TEST test_PASS(void)
{
	ASSERT_EQ(1, 1);
	PASS();
}

TEST test_SKIP(void)
{
	// Must place skip @ start otherwise failure reported
	SKIPm("TODO"); // PASS/SKIP/FAIL just return the result
	ASSERT_EQ(12, 1);
}

TEST test_FAIL(void)
{
	ASSERT_EQ(11, 1);
	PASS();
}

SUITE(suite_001)
{
	RUN_TEST(test_PASS);
	RUN_TEST(test_SKIP);
	RUN_TEST(test_FAIL);
}

