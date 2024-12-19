#include "CPRES.h"
#include "frtos_cmd.h"
#include "modbus_slave.h" 

bool f_debug_rs485;

//------------------------------------------------------------------------------
void tkRS485RX(void * pvParameters)
{

	/*
     * Esta tarea se encarga de recibir datos del modem LTE y
     * guardarlos en un buffer lineal.
     * Si se llena, BORRA EL BUFFER Y SE PIERDE TODO
     * No debería pasar porque antes lo debería haber procesado y flusheado
     * pero por las dudas. 
     */

( void ) pvParameters;
uint8_t c = 0;

    while ( ! starting_flag )
        vTaskDelay( ( TickType_t)( 100 / portTICK_PERIOD_MS ) );

    xprintf_P(PSTR("Starting tkRS485RX..\r\n" ));
    
    modbus_slave_fsm_init();
    
	// loop
	for( ;; )
	{
      // u_kick_wdt(RS485RX_WDG_gc);
       
		c = '\0';	// Lo borro para que luego del un CR no resetee siempre el timer.
		// el read se bloquea 50ms. lo que genera la espera.
        while ( xfgetc( fdRS485, (char *)&c ) == 1 ) {
            modbus_slave_fsm(c);
        } 
        
        vTaskDelay( ( TickType_t)( 10 / portTICK_PERIOD_MS ) );
	}    
}
//------------------------------------------------------------------------------
