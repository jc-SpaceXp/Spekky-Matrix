#include "greatest.h"
#include "suite_001.h"


GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	GREATEST_MAIN_BEGIN();

	RUN_SUITE(suite_001);

	GREATEST_MAIN_END(); // exapnds to a return statement
}
