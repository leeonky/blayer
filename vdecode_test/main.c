#include <stdio.h>
#include <stdlib.h>
#include "testutil/testutil.h"

/*find video track*/
/*docode*/
/*out put*/

extern test_vdecode_main();
extern test_vdecode_args();

int main() {
	init_test();

	test_vdecode_main();
	test_vdecode_args();

	return run_test();
}
