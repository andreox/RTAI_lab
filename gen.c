#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai_sched.h>
#include <rtai_shm.h>


#define SHMNAM 1234

#define TICK_PERIOD 1000000 
#define TICK_PERIOD2 10000000

#define STACK_SIZE 2000

#define NTASKS 3

static RT_TASK thread[NTASKS] ;

static int bits[3] ;

static int periodi[3] ;
static int fasi[] = {1,2,4} ;


static int argc = 3 ;


module_param_array( periodi, int, &argc, 0000) ;
//module_param_array( fasi, int, &argc, 0000) ;



struct decimal_value {

	int bit[3] ;

};

static struct decimal_value *decval ;

static void set__bit( int index ) {

	while(1) {
		bits[index] += 1 ;
		bits[index] %= 2 ;
		rt_printk("BIT %d VALUE : %d\n",index,bits[index]) ;
		decval->bit[index] = bits[index] ;
		rt_task_wait_period() ;

	}
	return;

}


int init_module(void) {

	RTIME periodiRT[3] ;
	RTIME fasiRT[3] ;
	RTIME fase_utile ;
	RTIME start = rt_get_time() ;
	int i ;

	decval = rtai_kmalloc( 12345, sizeof(struct decimal_value) );
	for ( i = 0 ; i < 3 ; i++ ) decval->bit[i] = 0 ; 

	fase_utile = nano2count( TICK_PERIOD2 ) ;
	for ( i = 0 ; i < NTASKS ; i++ ) {

		bits[i] = 1 ;
		periodi[i] = periodi[i]*TICK_PERIOD/**fasi[i]*/ ;
		periodiRT[i] = nano2count( periodi[i] ) ;
		fasiRT[i] = periodiRT[i] + start ;
		rt_task_init_cpuid( &thread[i] , (void*)set__bit, i, STACK_SIZE, NTASKS-i-1 , 0, 0, 0) ;
	}





	rt_task_make_periodic( &thread[0], start+(fase_utile*2) , periodiRT[0] ) ;
	rt_task_make_periodic( &thread[1], start+fase_utile , periodiRT[1] ) ;
	rt_task_make_periodic( &thread[2], start , periodiRT[2] ) ;
	

	rt_spv_RMS(0) ;

	return 0 ;
}

void cleanup_module(void)

{

    int i ;

    for ( i = 0 ; i < NTASKS ; i++ ) {

    	rt_task_delete( &thread[i] ) ;
    }
	rt_printk("##FINE ESECUZIONE##\n");
    rtai_kfree(12345) ;
    return;

}