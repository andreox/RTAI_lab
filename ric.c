#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai_sched.h>
#include <rtai_shm.h>


#define SHMNAM 1234

#define TICK_PERIOD 10000000 

#define STACK_SIZE 2000

static RT_TASK thread ;


struct rec_struct {

	int OK ;
	int count ;

};

static struct rec_struct *rec ;

struct decimal_value {

	int bit[3] ;

};

static struct decimal_value *decval ;

static int val_states[] = {0,3,6,5} ;
static int val_argc = 4 ;
module_param_array( val_states, int, &val_argc, 0000) ;

static void recognize(void) {

	int num ;
	int conteggio=0 ;
	while(1) {
		num = 4*(decval->bit[2]) + 2*(decval->bit[1]) + decval->bit[0] ;

		rt_printk("Numero letto : %d\n Num Seq : %d\n",num,rec->count);
		
		switch(conteggio) {

			case(0) :
				if ( val_states[conteggio] == num )
					conteggio++ ;
				break ;

			case(1) :
				if ( val_states[conteggio] == num )
					conteggio++ ;
				else {
					rec->OK = 0 ;
					conteggio = 0 ;
				}
				break ;
			
			case(2) :

				if ( val_states[conteggio] == num )
					conteggio++ ;
				else {
					rec->OK = 0 ;
					conteggio = 0 ;
				}
				break ;
			case(3) :

				if ( val_states[conteggio] == num ) {
					rec->OK = 1 ;
					rec->count += 1 ;
					rt_printk("OK : %d Conteggio : %d\n",rec->OK, rec->count) ;
					conteggio = 0 ;
				}
				else {
					rec->OK = 0 ;
					conteggio = 0 ;
				}
				break ;
	
			}
					
	
		rt_task_wait_period() ;
	}
	return;
	
}


int init_module(void) {

	RTIME periodo, start ;
	periodo = nano2count( TICK_PERIOD) ;
	rt_task_init_cpuid( &thread, (void*)recognize, 0, STACK_SIZE, 5, 0, 0, 0 ) ;

	start = rt_get_time() ;

	rec = rtai_kmalloc( SHMNAM, sizeof(struct rec_struct) );
	rec->OK = 0 ;
	rec->count = 0 ;

	decval = rtai_kmalloc( 12345, sizeof( struct decimal_value )) ;
	

	rt_task_make_periodic( &thread, start+(periodo*3), periodo) ;


	rt_spv_RMS(0) ;

	return 0 ;

}

void cleanup_module(void) {

	rt_task_delete(&thread) ;
	rtai_kfree(12345) ;
	rtai_kfree(SHMNAM) ;

	return;

}