/* 
 * File:   frtos20_utils.c
 * Author: pablo
 *
 * Created on 22 de diciembre de 2021, 07:34 AM
 */

#include "CPRES.h"
#include "pines.h"

//------------------------------------------------------------------------------
int8_t WDT_init(void);
int8_t CLKCTRL_init(void);
uint8_t checksum( uint8_t *s, uint16_t size );

//-----------------------------------------------------------------------------
void system_init()
{

    // Init OUT OF RTOS !!!
    
	CLKCTRL_init();
    //WDT_init();
    LED_init();
    XPRINTF_init();
    CONFIG_RTS_485();
    CLEAR_RTS_RS485();
    VALVES_init();
    FCx_init();
    //TMC2209_init();
   
}
//-----------------------------------------------------------------------------
int8_t WDT_init(void)
{
	/* 8K cycles (8.2s) */
	/* Off */
	ccp_write_io((void *)&(WDT.CTRLA), WDT_PERIOD_8KCLK_gc | WDT_WINDOW_OFF_gc );  
	return 0;
}
//-----------------------------------------------------------------------------
int8_t CLKCTRL_init(void)
{
	// Configuro el clock para 24Mhz
                                               	
	ccp_write_io((void *)&(CLKCTRL.OSCHFCTRLA), CLKCTRL_FRQSEL_24M_gc        /* 24 */
	| 0 << CLKCTRL_AUTOTUNE_bp /* Auto-Tune enable: disabled */
	| 0 << CLKCTRL_RUNSTDBY_bp /* Run standby: disabled */);

	// ccp_write_io((void*)&(CLKCTRL.MCLKCTRLA),CLKCTRL_CLKSEL_OSCHF_gc /* Internal high-frequency oscillator */
	//		 | 0 << CLKCTRL_CLKOUT_bp /* System clock out: disabled */);

	// ccp_write_io((void*)&(CLKCTRL.MCLKLOCK),0 << CLKCTRL_LOCKEN_bp /* lock enable: disabled */);

	return 0;
}
//-----------------------------------------------------------------------------
void reset(void)
{
    xprintf_P(PSTR("ALERT !!!. Going to reset...\r\n"));
    vTaskDelay( ( TickType_t)( 100 / portTICK_PERIOD_MS ) );
	/* Issue a Software Reset to initilize the CPU */
	ccp_write_io( (void *)&(RSTCTRL.SWRR), RSTCTRL_SWRST_bm ); 
                                           
}
//------------------------------------------------------------------------------
void kick_wdt( uint8_t bit_pos)
{
    // Pone el bit correspondiente en 0.
    sys_watchdog &= ~ (1 << bit_pos);
    
}
//------------------------------------------------------------------------------
void config_default(void)
{

}
//------------------------------------------------------------------------------
bool config_debug( char *tipo, char *valor)
{

    return (true);
}
//------------------------------------------------------------------------------
bool save_config_in_NVM(void)
{    
    return(true); 
}
//------------------------------------------------------------------------------
bool load_config_from_NVM(void)
{
    
    return(true);
}
//------------------------------------------------------------------------------
uint8_t checksum( uint8_t *s, uint16_t size )
{
	/*
	 * Recibe un puntero a una estructura y un tamaño.
	 * Recorre la estructura en forma lineal y calcula el checksum
	 */

uint8_t *p = NULL;
uint8_t cks = 0;
uint16_t i = 0;

	cks = 0;
	p = s;
	for ( i = 0; i < size ; i++) {
		 cks = (cks + (int)(p[i])) % 256;
	}

	return(cks);
}
//------------------------------------------------------------------------------
void u_update_valve_status(void)
{
    if ( ( systemVars.valve_0_status == VUNKNOWN ) || ( systemVars.valve_1_status == VUNKNOWN ) ) {
        systemVars.status_register = word_setBit(systemVars.status_register, VALVES_UNKNOWN_bp );
    } else {
        systemVars.status_register = word_clearBit(systemVars.status_register, VALVES_UNKNOWN_bp );
    }
}
//------------------------------------------------------------------------------
void u_valve_0_open(void)
{
    OPEN_VALVE_0();
    systemVars.valve_0_status = VOPEN;
    systemVars.status_register = word_setBit(systemVars.status_register, VALVE_0_bp);
    u_update_valve_status();
   
}
//------------------------------------------------------------------------------
void u_valve_0_close(void)
{
    CLOSE_VALVE_0();
    systemVars.valve_0_status = VCLOSE;
    systemVars.status_register = word_clearBit(systemVars.status_register, VALVE_0_bp);
    u_update_valve_status();
    
}
//------------------------------------------------------------------------------
void u_valve_1_open(void)
{
    OPEN_VALVE_1();
    systemVars.valve_1_status = VOPEN;
    systemVars.status_register = word_setBit(systemVars.status_register, VALVE_1_bp);
    u_update_valve_status();
    
}
//------------------------------------------------------------------------------
void u_valve_1_close(void)
{
    CLOSE_VALVE_1();
    systemVars.valve_1_status = VCLOSE;
    systemVars.status_register = word_clearBit(systemVars.status_register, VALVE_1_bp);
    u_update_valve_status();
    
}
//------------------------------------------------------------------------------
void u_set_consigna( consigna_t consigna)
{
    
    ENABLE_VALVES();
    
    if (consigna == CONSIGNA_DIURNA ) {
        u_valve_0_open();
        u_valve_1_close();
        goto exit;
    }
    
    if (consigna == CONSIGNA_NOCTURNA) {
        u_valve_0_close();
        u_valve_1_open();
        goto exit;        
        
    }
    
exit:

    vTaskDelay( ( TickType_t)( 10000 / portTICK_PERIOD_MS ) );   
    DISABLE_VALVES();
}
//------------------------------------------------------------------------------
bool test_set_consigna(char *s_modo,  char *s_action)
{
    
    if (!strcmp_P( strupr(s_modo), PSTR("CMD"))  ) {
        
        if (!strcmp_P( strupr(s_action), PSTR("DIURNA"))  ) {
            u_set_consigna(CONSIGNA_DIURNA);
            return (true);
        }
    
        if (!strcmp_P( strupr(s_action), PSTR("NOCTURNA"))  ) {
            u_set_consigna(CONSIGNA_NOCTURNA);
            return (true);
        }
    
        return(false);        
        
    }
    
    if (!strcmp_P( strupr(s_modo), PSTR("SYS"))  ) {
        
        if (!strcmp_P( strupr(s_action), PSTR("DIURNA"))  ) {
            systemVars.orders_register = 0x01;
            return (true);
        }
    
        if (!strcmp_P( strupr(s_action), PSTR("NOCTURNA"))  ) {
            systemVars.orders_register = 0x02;
            return (true);
        }
    
        return(false);        
        
    }
    
    return(false);
    

}
//------------------------------------------------------------------------------
bool test_valve( uint8_t vid , char *s_action )
{
    
bool retS = false;
    
    ENABLE_VALVES();
    
    switch(vid) {
        case 0:
            if (!strcmp_P( strupr(s_action), PSTR("OPEN"))  ) {
                u_valve_0_open();
                retS = true;
            } else if (!strcmp_P( strupr(s_action), PSTR("CLOSE"))  ) {
                u_valve_0_close();
                retS = true;
            }
            break;
            
        case 1:
            if (!strcmp_P( strupr(s_action), PSTR("OPEN"))  ) {
                u_valve_1_open();
                retS = true;
            } else if (!strcmp_P( strupr(s_action), PSTR("CLOSE"))  ) {
                u_valve_1_close();
                retS = true;
            }
            break;
    }
    
    
    if (retS) {
        // Debo actualizar el status register
        vTaskDelay( ( TickType_t)( 10000 / portTICK_PERIOD_MS ) );
    }
    DISABLE_VALVES();
    return(retS);
    
}
//------------------------------------------------------------------------------
