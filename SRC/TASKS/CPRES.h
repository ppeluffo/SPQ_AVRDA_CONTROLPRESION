/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#ifndef F_CPU
#define F_CPU 24000000
#endif


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "croutine.h"
#include "semphr.h"
#include "timers.h"
#include "limits.h"
#include "portable.h"

#include "protected_io.h"
#include "ccp.h"

//#include <avr/io.h>
//#include <avr/builtins.h>
#include <avr/wdt.h> 
//#include <avr/pgmspace.h>
//#include <avr/fuse.h>
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
//#include "math.h"

#include "frtos-io.h"
#include "xprintf.h"
#include "xgetc.h"
#include "nvm.h"
#include "led.h"
#include "pines.h"
#include "toyi_valves.h"
#include "linearBuffer.h"
#include "tmc2209.h"
#include "bits.h"

#define FW_REV "1.0.0"
#define FW_DATE "@ 20240905"
#define HW_MODELO "CPRES FRTOS R001 HW:AVR128DA64"
#define FRTOS_VERSION "FW:FreeRTOS V202111.00"
#define FW_TYPE "AUXBOARD"

#define SYSMAINCLK 24

#define tkCtl_TASK_PRIORITY	 	( tskIDLE_PRIORITY + 1 )
#define tkCmd_TASK_PRIORITY 	( tskIDLE_PRIORITY + 1 )
#define tkSys_TASK_PRIORITY 	( tskIDLE_PRIORITY + 1 )
#define tkRS485_TASK_PRIORITY 	( tskIDLE_PRIORITY + 1 )

#define tkCtl_STACK_SIZE		384
#define tkCmd_STACK_SIZE		384
#define tkSys_STACK_SIZE		384
#define tkRS485_STACK_SIZE		384

StaticTask_t tkCtl_Buffer_Ptr;
StackType_t tkCtl_Buffer [tkCtl_STACK_SIZE];

StaticTask_t tkCmd_Buffer_Ptr;
StackType_t tkCmd_Buffer [tkCmd_STACK_SIZE];

StaticTask_t tkSys_Buffer_Ptr;
StackType_t tkSys_Buffer [tkSys_STACK_SIZE];

StaticTask_t tkRS485_Buffer_Ptr;
StackType_t tkRS485_Buffer [tkRS485_STACK_SIZE];

SemaphoreHandle_t sem_SYSVars;
StaticSemaphore_t SYSVARS_xMutexBuffer;
#define MSTOTAKESYSVARSSEMPH ((  TickType_t ) 10 )

TaskHandle_t xHandle_tkCtl, xHandle_tkCmd, xHandle_tkSys, xHandle_tkRS485;

void tkCtl(void * pvParameters);
void tkCmd(void * pvParameters);
void tkSys(void * pvParameters);
void tkRS485RX(void * pvParameters);

void system_init();
void reset(void);

// Mensajes entre tareas
#define SIGNAL_ACTION		0x01

void kick_wdt( uint8_t bit_pos);

uint8_t u_hash(uint8_t seed, char ch );
void config_default(void);
bool config_debug( char *tipo, char *valor);
bool save_config_in_NVM(void);
bool load_config_from_NVM(void);

void RS485_print_buffer(void);
void RS485_flush_buffer(void);
void RS485_config_debug(bool debug );

typedef enum { CONSIGNA_DIURNA = 0, CONSIGNA_NOCTURNA } consigna_t;

void u_set_consigna( consigna_t consigna);
void u_valve_0_open(void);
void u_valve_0_close(void);
void u_valve_1_open(void);
void u_valve_1_close(void);
void u_update_valve_status(void);

bool test_set_consigna(char *s_modo, char *s_action);
bool test_valve( uint8_t vid , char *s_action );

bool starting_flag;

struct {   
    bool debug;
    float battery;
    uint16_t status_register;
    uint16_t orders_register;
    t_valves_status valve_0_status;
    t_valves_status valve_1_status;
} systemVars;


#define VALVE_0_bp          0
#define VALVE_1_bp          1
#define VALVES_UNKNOWN_bp   2

#define CONSIGNA_DIURNA         0x01
#define CONSIGNA_NOCTURNA       0x02

#define STATUS_BIT 15

uint8_t sys_watchdog;

#define CMD_WDG_bp    0
#define SYS_WDG_bp    1
#define XCMA_WDG_bp   2
#define RS485RX_WDG_gc   3

// No habilitado PLT_WDG !!!
#define WDG_bm 0x3

#define WDG_INIT() ( sys_watchdog = WDG_bm )

#endif	/* XC_HEADER_TEMPLATE_H */

