#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai_sched.h>
#include <rtai_shm.h>
#include <stdbool.h>



#define SHMNAM 1234

#define TICK_PERIOD 1000000 // 10^6 ns = 1 * 10^-3 s = 1 ms

#define STACK_SIZE 2000

#define NTASKS 4

static RT_TASK thread[NTASKS] ;

static int bits[3] ;
static bool state[4] ;
static int val_states[4];
static int periodi[3] ;
static int fasi[3] ;

static int arr_argc = 3 ;
static int arr_argc_values = 4 ;

module_param_array( periodi, int, &arr_argc, 0000) ;
module_param_array( fasi, int, &arr_argc, 0000) ;
module_param_array( val_states,int, &arr_argc_values, 0000) ;

struct rec_struct {

	int OK ;
	int count ;

};

static struct rec_struct *rec ;



static void set__bit( int index ) {

	while(1) {
		if ( bits[index] == 1 ) bits[index] = 0 ;
		else bits[index] = 1 ;
		rt_printk("BIT %d VALUE : %d\n",index,bits[index]) ;
		rt_task_wait_period() ;
	}
	return;

}

static void reset_states(void) {

	int k ;
	for ( k = 0 ; k < 4 ; k++ ) state[k] = false ;

}

static void recognize(void) {

	int num,j ;
	while(1) {
		num = 4*bits[2] + 2*bits[1] + bits[0] ;

		rt_printk("Numero letto : %d\n N.Seq ric : %d\n",num,rec->count);
		
		for ( j = 0 ; j < 4 && state[j] ; j++ ) {}

		if ( j == 4 ) {
			rec->OK = 1 ;
			rec->count = rec->count + 1 ;
			rt_printk(" REC->OK : 1, REC->COUNT : %d\n\n",rec->count) ;

			reset_states() ;
		}

		else if ( !state[j] && num == val_states[j] ) {

			state[j] = true ;
			if ( j == 3 ) {

				rec->OK = 1 ;
				rec->count = rec->count + 1 ;
				rt_printk("REC->OK : 1, REC->COUNT : %d\n\n",rec->count) ;
				reset_states() ;
			}

		}

		else { //ho riconosciuto 0,3 e poi un numero diverso, devo risettare gli stati

			reset_states() ;
		}
	
		rt_task_wait_period() ;
	}
	return;
	
}

int init_module(void) {

	RTIME periodo_cnt[3] ;
	RTIME fasi_cnt[3] ;
	

	int i ;

	for ( i = 0 ; i < NTASKS-1 ; i++ ) {

		bits[i] = 0 ;
		periodi[i] *= TICK_PERIOD ;
		periodo_cnt[i] = nano2count(periodi[i]) ;
		fasi_cnt[i] = periodo_cnt[i] + rt_get_time() ;
		rt_task_init( &thread[i] , (void*)set__bit, i, STACK_SIZE, NTASKS - i , 0, 0) ;
	}


	rt_task_init( &thread[3], (void*)recognize, 0, STACK_SIZE, 5, 0, 0 ) ;
	reset_states() ;
	rec = rtai_kmalloc( SHMNAM, sizeof(struct rec_struct) );
	rec->OK = 0 ;
	rec->count = 0 ;

	for ( i = 0 ; i < 3 ; i++ ) {
		rt_task_make_periodic( &thread[i], fasi_cnt[i] , periodo_cnt[i]) ;

	}

	rt_task_make_periodic( &thread[3], rt_get_time()+(periodo_cnt[0]*5), periodo_cnt[0]) ;

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
