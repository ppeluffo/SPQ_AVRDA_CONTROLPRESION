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
    
typedef enum { VALVE_OPEN=0, VALVE_CLOSE } t_valve_status;
    
typedef struct {
    t_valve_status V1_status;
    t_valve_status V2_status;
} t_valves_status;

t_valves_status VALVES_STATUS;

#define VALVES_EN_PORT      PORTB
#define VALVES_EN_PIN_bm	PIN1_bm
#define VALVES_EN_PIN_bp    PIN1_bp
    
#define ENABLE_VALVES()  ( VALVES_EN_PORT.OUT |= VALVES_EN_PIN_bm )
#define DISABLE_VALVES() ( VALVES_EN_PORT.OUT &= ~VALVES_EN_PIN_bm )

#define VALVE_CTRL1_PORT       PORTB
#define VALVE_CTRL1_PIN_bm     PIN0_bm
#define VALVE_CTRL1_PIN_bp     PIN0_bp
    
#define OPEN_VALVE1()  ( VALVE_CTRL1_PORT.OUT |= VALVE_CTRL1_PIN_bm ); VALVES_STATUS.V1_status = VALVE_OPEN;
#define CLOSE_VALVE1() ( VALVE_CTRL1_PORT.OUT &= ~VALVE_CTRL1_PIN_bm ); VALVES_STATUS.V1_status = VALVE_CLOSE;

#define VALVE_CTRL2_PORT       PORTG
#define VALVE_CTRL2_PIN_bm     PIN0_bm
#define VALVE_CTRL2_PIN_bp     PIN0_bp
    
#define OPEN_VALVE2()  ( VALVE_CTRL2_PORT.OUT |= VALVE_CTRL2_PIN_bm ); VALVES_STATUS.V2_status = VALVE_OPEN;
#define CLOSE_VALVE2() ( VALVE_CTRL2_PORT.OUT &= ~VALVE_CTRL2_PIN_bm ); VALVES_STATUS.V2_status = VALVE_CLOSE;
    
void VALVES_EN_init(void);
void VALVE1_init(void);
void VALVE2_init(void);
void VALVES_init(void);
t_valves_status *get_valves_status(void);




#ifdef	__cplusplus
}
#endif

#endif	/* TOYI_VALVES_H */

