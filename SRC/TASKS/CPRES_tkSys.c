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
uint8_t order;

     while ( ! starting_flag )
        vTaskDelay( ( TickType_t)( 100 / portTICK_PERIOD_MS ) );
       
    xprintf_P(PSTR("Starting tkSys..\r\n" ));
        
    systemVars.status_register = byte_clearBit( systemVars.status_register, STATUS_BIT );
    systemVars.status_register = byte_setBit( systemVars.status_register, VALVE_0_UNKNOWN_bp );
    systemVars.status_register = byte_setBit( systemVars.status_register, VALVE_1_UNKNOWN_bp );
    
	for( ;; )
	{
        /*
         * Monitorea el status y el order. Cuando el status != order, ejecuta
         * la accion y actualiza los registros.
         */
        
        order = systemVars.orders_register;
        
        if ( order != CMD_NONE ) {
            
            xprintf_P(PSTR("SYS: working\r\n"));
            xprintf_P(PSTR("SYS process: order=0x%02x, status=0x%02x\r\n"), order, systemVars.status_register);
            
            // Prendo el bit de status WORKING(b7=1)
            systemVars.status_register = byte_setBit( systemVars.status_register, STATUS_BIT );
            
            // Aplico la orden
            switch(order) {
                case CMD_OPEN_V0:
                    xprintf_P(PSTR("SYS: open V0\r\n"));
                    u_valve_0_open();
                    break;
                case CMD_CLOSE_V0:
                    xprintf_P(PSTR("SYS: close V0\r\n"));
                    u_valve_0_close();
                    break;
                case CMD_OPEN_V1:
                    xprintf_P(PSTR("SYS: open V1\r\n"));
                    u_valve_1_open();
                    break;
                case CMD_CLOSE_V1:
                    xprintf_P(PSTR("SYS: close V1\r\n"));
                    u_valve_1_close();
                    break;                
                case CMD_SET_CONSIGNA_DIURNA:
                    xprintf_P(PSTR("SYS: set consigna diurna (V0 open, V1 close)\r\n"));
                    u_set_consigna(CONSIGNA_DIURNA);
                    break;
                case CMD_SET_CONSIGNA_NOCTURNA:
                    xprintf_P(PSTR("SYS: set consigna nocturna (V0 close, V1 open)\r\n"));
                    u_set_consigna(CONSIGNA_NOCTURNA);
                    break;
                default:
                    xprintf_P(PSTR("SYS: Error. CMD no implementado [%d]\r\n"), order);
                    order = CMD_NONE;
                    break;
            }
            
            order = CMD_NONE;
            while ( xSemaphoreTake( sem_SYSVars, ( TickType_t ) 5 ) != pdTRUE )
                vTaskDelay( ( TickType_t)( 10 ) ); 
            systemVars.orders_register = CMD_NONE;
            xSemaphoreGive( sem_SYSVars );
            
            // Paso el bit de status a STANDBY (b7=0)
            systemVars.status_register = word_clearBit( systemVars.status_register, STATUS_BIT );
            xprintf_P(PSTR("SYS: standby\r\n"));
        }
        
        // Duermo hasta 60s o que alguien me despierte.
        ulTaskNotifyTake( pdTRUE, ( TickType_t)( (60000) / portTICK_PERIOD_MS) );
        //xprintf_P(PSTR("SYS: wakeup\r\n"));
        //vTaskDelay( ( TickType_t)( 1000 / portTICK_PERIOD_MS ) );
        
	}
}
//------------------------------------------------------------------------------
