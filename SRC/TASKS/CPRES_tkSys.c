/*
 * File:   tkSys.c
 * Author: pablo
 *
 * Created on 25 de octubre de 2021, 12:50 PM
 */


#include "CPRES.h"

//------------------------------------------------------------------------------
void tkSys(void * pvParameters)
{

	// Esta es la primer tarea que arranca.

( void ) pvParameters;
uint8_t order, status;

     while ( ! starting_flag )
        vTaskDelay( ( TickType_t)( 100 / portTICK_PERIOD_MS ) );
       
    xprintf_P(PSTR("Starting tkSys..\r\n" ));
        
    systemVars.status_register = 0x4;   // standby, valve_status=unknown
            
	for( ;; )
	{
        /*
         * Monitorea el status y el order. Cuando el status != order, ejecuta
         * la accion y actualiza los registros.
         */
        
        order = (uint8_t) systemVars.orders_register;
        status = (uint8_t) systemVars.status_register;
        
        if ( order != status ) {
            xprintf_P(PSTR("SYS: working\r\n"));
            xprintf_P(PSTR("SYS process: order=0x%02x, status=0x%02x\r\n"), order, status);
            // Prendo el bit de status WORKING(b15=1)
            systemVars.status_register = word_setBit( systemVars.status_register, STATUS_BIT );
            
            // Aplico la orden
            switch(order) {
                case 0x01:
                    xprintf_P(PSTR("SYS process: set consigna diurna (V0 open, V1 close)\r\n"));
                    u_set_consigna(CONSIGNA_DIURNA);
                    break;
                case 0x02:
                    xprintf_P(PSTR("SYS process: set consigna nocturna (V0 close, V1 open)\r\n"));
                    u_set_consigna(CONSIGNA_NOCTURNA);
                    break;
                default:
                    xprintf_P(PSTR("SYS: Error. CMD no implementado [%d]\r\n"), order);
                    order = 0x00;
                    break;
            }
            
            // Actualizo el status.
            systemVars.status_register = order;
            
            // Paso el bit de status a STANDBY (b15=0)
            systemVars.status_register = word_clearBit( systemVars.status_register, STATUS_BIT );
            xprintf_P(PSTR("SYS: standby\r\n"));
        }
        
        //ulTaskNotifyTake( pdTRUE, ( TickType_t)( (60000) / portTICK_PERIOD_MS) );
        vTaskDelay( ( TickType_t)( 5000 / portTICK_PERIOD_MS ) );
        
	}
}
//------------------------------------------------------------------------------
