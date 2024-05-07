#include "greatest.h"
#include "spi_suite.h"


GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	GREATEST_MAIN_BEGIN();

	RUN_SUITE(spi_driver);

	GREATEST_MAIN_END(); // exapnds to a return statement
}
