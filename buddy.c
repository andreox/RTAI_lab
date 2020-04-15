//---------------------- BUDDY.C ----------------------------

#include <stdio.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/mman.h>

#include <sys/stat.h>

#include <fcntl.h>

#include <signal.h>

#include "/usr/realtime/include/rtai_shm.h"

#define SHMNAM 1234

struct rec_struct {

	int OK ;
	int count ;
};
	


static int end;

static void endme(int dummy) { end=1; }

 

int main (void)

{

    struct rec_struct *data;

    signal(SIGINT, endme);

    data = rtai_malloc (SHMNAM,1);

	
    while (1) {

        printf(" OK : %d Count : %d \n", data->OK, data->count);
	
    }

    rtai_free (SHMNAM, &data);

    return 0;

}
