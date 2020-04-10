#include <linux/module.h>
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai_sched.h>
#include <rtai_shm.h>
#include <stdbool.h>
#include <stdlib.h>


#define SHMNAM 1234

#define TICK_PERIOD 10000000 // 10^7 ns = 1 * 10^-3 s = 10 ms

#define STACK_SIZE 2000

#define NTASKS 4

static RT_TASK thread[NTASKS] ;

static int bits[3] ;
static bool state[3] ;
static int val_states[] = { 0 , 3 , 6 , 5 } ;

struct rec_struct {

	int OK ;
	int count ;
};

static struct rec_struct *rec ;



static void set_bit( int index ) {

	if ( bits[index] == 1 ) bits[index] = 0 ;
	else bits[index] = 1 ;
	rt_printk("BIT %d VALUE : %d\n",index,bits[index]) ;

	return;

}

static void recognize() {

	int num = 2*2*bits[2] + 2*bits[1] + bits[0] ;

	int j,k ;
	for ( j = 0 ; j < 3 && state[j] ; j++ ) ;

	if ( j == 3 ) {
		rec->OK = 1 ;
		rec->count++ ;
		rt_printk(" REC->OK : 1, REC->COUNT : %d\n\n",rec->count) ;

		for ( k = 0 ; k < 3 ; k++ ) state[k] = false ;
	}

	else if ( !state[j] && num == val_states[j] ) state[j] = true ;

	else { //ho riconosciuto 0,3 e poi un numero diverso, devo risettare gli stati

		for ( k = 0 ; k < 3 ; k++ ) state[k] = false ;
	}
	
	return;
	
}

int init_module(void) {

	RTIME periodo ;

	int i ;

	for ( i = 0 ; i < NTASKS-1 ; i++ ) {

		bits[i] = 0 ;
		rt_task_init( &thread[i] , fun, i, STACK_SIZE, NTASKS - i , 0, 0) ;
	}


	rt_task_init( &thread[3], recognize, NULL, STACK_SIZE, 5, 0, 0 ) ;
	for ( i = 0 ; i < 3 ; i++ ) state[i] = false ;
	rec = rtai_kmalloc( SHMNAM, sizeof(struct rec_struct) );
	rec->OK = 0 ;
	rec->count = 0 ;

	periodo = nano2count( TICK_PERIOD ) ;

	for ( i = 0 ; i < NTASKS-1 ; i++ ) {

		rt_make_periodic( &thread[i], rt_get_time()+(periodo*(i+1)) , periodo*(i+1)) ;
	}

	rt_make_periodic( &thread[3], rt_get_time()+(periodo*6), periodo) ;

	rt_spv_RMS(0) ;

	return 0 ;
}

void cleanup_module(void)

{

    int i ;

    for ( i = 0 ; i < NTASKS ; i++ ) {

    	rt_task_delete( &thread[i] ) ;
    }

    rtai_kfree(SHMNAM) ;
    return;

}