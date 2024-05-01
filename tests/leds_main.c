#include "greatest.h"
#include "leds_suite.h"


GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	GREATEST_MAIN_BEGIN();

	RUN_SUITE(leds_driver);

	GREATEST_MAIN_END(); // exapnds to a return statement
}
