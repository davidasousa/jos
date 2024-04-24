
#include <inc/lib.h>

void
exit(void)
{
	close_all();  //CHANGE THIS LATER
	sys_env_destroy(0);
}

