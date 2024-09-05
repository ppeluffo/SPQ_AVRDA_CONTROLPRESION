
#include "tmc2209.h"
#include "porthardware.h"
#include "avr/interrupt.h"


uint16_t tmc_2209_counter;
t_tmc2209_status tmc2209_status;

// ---------------------------------------------------------------
void TMC2209_EN_init(void)
{
	// Configura el pin del led como output
	TMC2209_EN_PORT.DIR |= TMC2209_EN_PIN_bm;	
	TMC2209_DISABLE();
}
// ---------------------------------------------------------------
void TMC2209_DIR_init(void)
{
	// Configura el pin del led como output
	TMC2209_DIR_PORT.DIR |= TMC2209_DIR_PIN_bm;	
	TMC2209_DIR_FORWARD();
}
// ---------------------------------------------------------------
void TMC2209_STEP_init(void)
{
 
	// Configura el pin del led como output
	TMC2209_STEP_PORT.DIR |= TMC2209_STEP_PIN_bm;	
	TMC2209_STEP_OFF();
}
// ---------------------------------------------------------------
void TMC2209_init(void)
{
    TMC2209_EN_init();
    TMC2209_DIR_init();
    TMC2209_STEP_init();
    
    TCB1.CCMP = configCPU_CLOCK_HZ / (configTICK_RATE_HZ ); 
    TCB1.INTCTRL = TCB_CAPT_bm;                          
    //TCB1.CTRLA = TCB_ENABLE_bm; 
    

}
// ---------------------------------------------------------------
void TMC2209_TIMER_start(void)
{
    TCB1.INTFLAGS = TCB_CAPT_bm;                         
    TCB1.CTRLA = TCB_ENABLE_bm;  
}
// ---------------------------------------------------------------
void TMC2209_TIMER_stop(void)
{
    TCB1.CTRLA = 0x00;
    TMC2209_STEP_OFF();
}
// ---------------------------------------------------------------
void TMC2209_TIMER_freq(uint16_t freq)
{
    TMC2209_TIMER_stop();
    TCB1.CCMP = configCPU_CLOCK_HZ / (freq );
}
// ---------------------------------------------------------------

ISR( TCB1_INT_vect )
{
    CLR_INT( TCB1_INTFLAGS, TCB_CAPT_bm );

    TMC2209_STEP_TOGGLE();
    //PORTB.OUTTGL = 0x03;
    
    //TCB1_INTFLAGS = TCB_CAPT_bm;
    //INT_FLAGS = INT_MASK;
    

}

        