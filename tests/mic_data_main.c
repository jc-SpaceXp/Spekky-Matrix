#include "greatest.h"
#include "mic_data_suite.h"


GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	GREATEST_MAIN_BEGIN();

	RUN_SUITE(i2s_mic_data_processing);

	GREATEST_MAIN_END(); // exapnds to a return statement
}
