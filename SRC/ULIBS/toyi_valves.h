/* 
 * File:   toyi_valves.h
 * Author: pablo
 *
 * Created on 19 de enero de 2024, 08:51 AM
 */

#ifndef TOYI_VALVES_H
#define	TOYI_VALVES_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include "FreeRTOS.h"
#include "task.h"
    
#define VALVES_EN_PORT      PORTB
#define VALVES_EN_PIN_bm	PIN1_bm
#define VALVES_EN_PIN_bp    PIN1_bp
    
#define ENABLE_VALVES()  ( VALVES_EN_PORT.OUT |= VALVES_EN_PIN_bm )
#define DISABLE_VALVES() ( VALVES_EN_PORT.OUT &= ~VALVES_EN_PIN_bm )

#define VALVE_0_CTRL_PORT       PORTB
#define VALVE_0_CTRL_PIN_bm     PIN0_bm
#define VALVE_0_CTR1_PIN_bp     PIN0_bp
    
#define CLOSE_VALVE_0()  ( VALVE_0_CTRL_PORT.OUT |= VALVE_0_CTRL_PIN_bm )
#define OPEN_VALVE_0() ( VALVE_0_CTRL_PORT.OUT &= ~VALVE_0_CTRL_PIN_bm )

#define VALVE_1_CTRL_PORT       PORTG
#define VALVE_1_CTRL_PIN_bm     PIN0_bm
#define VALVE_1_CTRL_PIN_bp     PIN0_bp
    
#define CLOSE_VALVE_1()  ( VALVE_1_CTRL_PORT.OUT |= VALVE_1_CTRL_PIN_bm )
#define OPEN_VALVE_1() ( VALVE_1_CTRL_PORT.OUT &= ~VALVE_1_CTRL_PIN_bm )

    
typedef enum { VOPEN=0, VCLOSE, VUNKNOWN } t_valves_status;
    
void VALVES_EN_init(void);
void VALVE_0_init(void);
void VALVE_1_init(void);

void VALVES_init(void);


#ifdef	__cplusplus
}
#endif

#endif	/* TOYI_VALVES_H */

