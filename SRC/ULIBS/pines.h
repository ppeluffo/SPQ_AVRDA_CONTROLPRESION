    /* 
 * File:   pines.h
 * Author: pablo
 *
 * Created on 11 de febrero de 2022, 06:02 PM
 */

#ifndef PINES_H
#define	PINES_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include "stdbool.h"
    
//--------------------------------------------------------------------------

#define RTS_RS485_PORT         PORTC
#define RTS_RS485              2
#define RTS_RS485_PIN_bm       PIN2_bm
#define RTS_RS485_PIN_bp       PIN2_bp
#define SET_RTS_RS485()        ( RTS_RS485_PORT.OUT |= RTS_RS485_PIN_bm )
#define CLEAR_RTS_RS485()      ( RTS_RS485_PORT.OUT &= ~RTS_RS485_PIN_bm )
#define CONFIG_RTS_485()       RTS_RS485_PORT.DIR |= RTS_RS485_PIN_bm;

// Los pines de FinCarrera son entradas
#define FC1_PORT      PORTE    
#define FC1           6
#define FC1_PIN_bm    PIN6_bm
#define FC1_PIN_bp    PIN6_bp
#define CONFIG_FC1()    ( FC1_PORT.DIR &= ~FC1_PIN_bm )

#define FC2_PORT      PORTA     
#define FC2           7
#define FC2_PIN_bm    PIN7_bm
#define FC2_PIN_bp    PIN7_bp
#define CONFIG_FC2()    ( FC2_PORT.DIR &= ~FC2_PIN_bm )

uint8_t FC1_read(void);
uint8_t FC2_read(void);

#define FC_alta_read() FC1_read()
#define FC_baja_read() FC2_read()

void FCx_init(void);

#ifdef	__cplusplus
}
#endif

#endif	/* PINES_H */

