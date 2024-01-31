
#include "CPRES.h"
#include "frtos_cmd.h"

static void cmdClsFunction(void);
static void cmdHelpFunction(void);
static void cmdResetFunction(void);
static void cmdStatusFunction(void);
static void cmdWriteFunction(void);
static void cmdReadFunction(void);
static void cmdConfigFunction(void);
static void cmdTestFunction(void);

static void pv_snprintfP_OK(void );
static void pv_snprintfP_ERR(void );

static bool test_valves( char *vid, char *action);

//------------------------------------------------------------------------------
void tkCmd(void * pvParameters)
{

	// Esta es la primer tarea que arranca.

( void ) pvParameters;

    while ( ! starting_flag )
        vTaskDelay( ( TickType_t)( 100 / portTICK_PERIOD_MS ) );

	//vTaskDelay( ( TickType_t)( 500 / portTICK_PERIOD_MS ) );

uint8_t c = 0;
//uint16_t sleep_timeout;

    FRTOS_CMD_init();

    FRTOS_CMD_register( "cls", cmdClsFunction );
	FRTOS_CMD_register( "help", cmdHelpFunction );
    FRTOS_CMD_register( "reset", cmdResetFunction );
    FRTOS_CMD_register( "status", cmdStatusFunction );
    FRTOS_CMD_register( "write", cmdWriteFunction );
    FRTOS_CMD_register( "read", cmdReadFunction );
    FRTOS_CMD_register( "config", cmdConfigFunction );
    FRTOS_CMD_register( "test", cmdTestFunction );
    
    xprintf_P(PSTR("Starting tkCmd..\r\n" ));
    xprintf_P(PSTR("Spymovil %s %s %s %s \r\n") , HW_MODELO, FRTOS_VERSION, FW_REV, FW_DATE);
      
	// loop
	for( ;; )
	{
        kick_wdt(CMD_WDG_bp);
         
		c = '\0';	// Lo borro para que luego del un CR no resetee siempre el timer.
		// el read se bloquea 10ms. lo que genera la espera.
		//while ( frtos_read( fdTERM, (char *)&c, 1 ) == 1 ) {
        while ( xgetc( (char *)&c ) == 1 ) {
            FRTOS_CMD_process(c);
        }
        
        // Espero 10ms si no hay caracteres en el buffer
        vTaskDelay( ( TickType_t)( 10 / portTICK_PERIOD_MS ) );
               
	}    
}
//------------------------------------------------------------------------------
static void cmdTestFunction(void)
{

    FRTOS_CMD_makeArgv();

    // tmc2209 {en,step} {on|off}
    //         dir {fw|rev}\r\n"));
    //         run {fw|rev} period pulses
    if (!strcmp_P( strupr(argv[1]), PSTR("TMC2209"))  ) {
        
        if (!strcmp_P( strupr(argv[2]), PSTR("RUN"))  ) {
            if (!strcmp_P( strupr(argv[3]), PSTR("FW"))  ) {
                tmc2209_start(DIR_FW, atoi(argv[4]), atoi(argv[5]));
                pv_snprintfP_OK();
                return;
            }
            if (!strcmp_P( strupr(argv[3]), PSTR("REV"))  ) {
                tmc2209_start(DIR_REV, atoi(argv[4]), atoi(argv[5]));
                pv_snprintfP_OK();
                return;
            }
            pv_snprintfP_ERR();
            return;
            
            pv_snprintfP_OK();
            return;
        }
        
        if (!strcmp_P( strupr(argv[2]), PSTR("EN"))  ) {
            if (!strcmp_P( strupr(argv[3]), PSTR("ON"))  ) {
                TMC2209_ENABLE();
                pv_snprintfP_OK();
                return;
            }
            if (!strcmp_P( strupr(argv[3]), PSTR("OFF"))  ) {
                TMC2209_DISABLE();
                pv_snprintfP_OK();
                return;
            }
            pv_snprintfP_ERR();
            return;
        }
        
        if (!strcmp_P( strupr(argv[2]), PSTR("STEP"))  ) {
            if (!strcmp_P( strupr(argv[3]), PSTR("ON"))  ) {
                TMC2209_STEP_ON();
                pv_snprintfP_OK();
                return;
            }
            if (!strcmp_P( strupr(argv[3]), PSTR("OFF"))  ) {
                TMC2209_STEP_OFF();
                pv_snprintfP_OK();
                return;
            }
            pv_snprintfP_ERR();
            return;
        }
        
        if (!strcmp_P( strupr(argv[2]), PSTR("DIR"))  ) {
            if (!strcmp_P( strupr(argv[3]), PSTR("FW"))  ) {
                TMC2209_DIR_FORWARD();
                pv_snprintfP_OK();
                return;
            }
            if (!strcmp_P( strupr(argv[3]), PSTR("REV"))  ) {
                TMC2209_DIR_REVERSE();
                pv_snprintfP_OK();
                return;
            }
            pv_snprintfP_ERR();
            return;
        }

        pv_snprintfP_ERR();
        return;
    }
    
    // test kill {sys}
    if (!strcmp_P( strupr(argv[1]), PSTR("KILL"))  ) {
               
        if (!strcmp_P( strupr(argv[2]), PSTR("SYS"))  ) {
            if ( xHandle_tkSys != NULL ) {
                vTaskSuspend( xHandle_tkSys );
                xHandle_tkSys = NULL;
            }
            pv_snprintfP_OK();
            return;
        }        

        pv_snprintfP_ERR();
        return;
    }
    
    pv_snprintfP_ERR();
    return;
       
}
//------------------------------------------------------------------------------
static void cmdHelpFunction(void)
{

    FRTOS_CMD_makeArgv();
        
    if ( !strcmp_P( strupr(argv[1]), PSTR("WRITE"))) {
		xprintf_P( PSTR("-write:\r\n"));
        xprintf_P( PSTR("  nvmee {pos string} {debug}\r\n"));
        
    }  else if ( !strcmp_P( strupr(argv[1]), PSTR("READ"))) {
		xprintf_P( PSTR("-read:\r\n"));
        xprintf_P( PSTR("  nvmee {pos} {lenght} {debug}\r\n"));
        xprintf_P( PSTR("  avrid,rtc {long,short}\r\n"));
        xprintf_P( PSTR("  fc1,fc2\r\n"));
        
    }  else if ( !strcmp_P( strupr(argv[1]), PSTR("CONFIG"))) {
		xprintf_P( PSTR("-config:\r\n"));
        xprintf_P( PSTR("  save\r\n"));
 
    	// HELP RESET
	} else if (!strcmp_P( strupr(argv[1]), PSTR("RESET"))) {
		xprintf_P( PSTR("-reset\r\n"));
        xprintf_P( PSTR("  memory {soft|hard}\r\n"));
		return;
        
    } else if (!strcmp_P( strupr(argv[1]), PSTR("TEST"))) {
		xprintf_P( PSTR("-test\r\n"));
        xprintf_P( PSTR("  kill {wan,sys}\r\n"));
        xprintf_P( PSTR("  valve {1|2} {open|close}\r\n"));
        xprintf_P( PSTR("        {enable|disable}\r\n"));
        xprintf_P( PSTR("  tmc2209 {en,step} {on|off}\r\n"));
        xprintf_P( PSTR("          dir {fw|rev}\r\n"));
        xprintf_P( PSTR("          run {fw|rev} period pulses\r\n"));
        return;
        
    }  else {
        // HELP GENERAL
        xprintf("Available commands are:\r\n");
        xprintf("-cls\r\n");
        xprintf("-help\r\n");
        xprintf("-status\r\n");
        xprintf("-reset\r\n");
        xprintf("-write...\r\n");
        xprintf("-config...\r\n");
        xprintf("-read...\r\n");

    }
   
	xprintf("Exit help \r\n");

}
//------------------------------------------------------------------------------
static void cmdReadFunction(void)
{
    
    FRTOS_CMD_makeArgv();       
    
    
    // Fines de carrera FC1,FC2
    // read {fc1|fc2}
	if (!strcmp_P( strupr(argv[1]), PSTR("FC1")) ) {
		xprintf_P(PSTR("FC1=%d\r\n"), FC1_read());
        pv_snprintfP_OK();
		return;
	}    

	if (!strcmp_P( strupr(argv[1]), PSTR("FC2")) ) {
		xprintf_P(PSTR("FC2=%d\r\n"), FC2_read());
        pv_snprintfP_OK();
		return;
	}
    
    // NVMEE
	// read nvmee address length
	if (!strcmp_P( strupr(argv[1]), PSTR("NVMEE")) ) {
		NVMEE_test_read ( argv[2], argv[3] );
		return;
	}

	// AVRID
	// read avrid
	if (!strcmp_P( strupr(argv[1]), PSTR("AVRID"))) {
		//nvm_read_print_id();
        xprintf_P(PSTR("ID: %s\r\n"), NVM_id2str() );
        xprintf_P(PSTR("SIGNATURE: %s\r\n"), NVM_signature2str() );
		return;
	}
    
    
    // CMD NOT FOUND
	xprintf("ERROR\r\nCMD NOT DEFINED\r\n\0");
	return;
 
}
//------------------------------------------------------------------------------
static void cmdClsFunction(void)
{
	// ESC [ 2 J
	xprintf("\x1B[2J\0");
}
//------------------------------------------------------------------------------
static void cmdResetFunction(void)
{
    
    FRTOS_CMD_makeArgv();
    
    xprintf("Reset..\r\n");
    reset();
}
//------------------------------------------------------------------------------
static void cmdStatusFunction(void)
{

    // https://stackoverflow.com/questions/12844117/printing-defined-constants

t_valves_status valves_status;

    xprintf("Spymovil %s %s TYPE=%s, VER=%s %s \r\n" , HW_MODELO, FRTOS_VERSION, FW_TYPE, FW_REV, FW_DATE);
      
    memcpy( &valves_status, get_valves_status(), sizeof(t_valves_status));
    if ( valves_status.V1_status == VALVE_OPEN ) {
        xprintf_P(PSTR("V1 = OPEN\r\n"));
    } else {
        xprintf_P(PSTR("V1 = CLOSE\r\n"));
    }
    
    if ( valves_status.V2_status == VALVE_OPEN ) {
        xprintf_P(PSTR("V2 = OPEN\r\n"));
    } else {
        xprintf_P(PSTR("V2 = CLOSE\r\n"));
    }    
    
    xprintf_P(PSTR("Config:\r\n"));
    
}
//------------------------------------------------------------------------------
static void cmdWriteFunction(void)
{

    FRTOS_CMD_makeArgv();
        
    // NVMEE
	// write nvmee pos string
	if ( (strcmp_P( strupr(argv[1]), PSTR("NVMEE")) == 0)) {
		NVMEE_test_write ( argv[2], argv[3] );
		pv_snprintfP_OK();
		return;
	}
    
    // CMD NOT FOUND
	xprintf("ERROR\r\nCMD NOT DEFINED\r\n\0");
	return;
 
}
//------------------------------------------------------------------------------
static void cmdConfigFunction(void)
{
    
    FRTOS_CMD_makeArgv();

	// SAVE
	// config save
	if (!strcmp_P( strupr(argv[1]), PSTR("SAVE"))) {       
		save_config_in_NVM();
		pv_snprintfP_OK();
		return;
	}
    
    // LOAD
	// config load
	if (!strcmp_P( strupr(argv[1]), PSTR("LOAD"))) {
		load_config_from_NVM();
		pv_snprintfP_OK();
		return;
	}
    
    // CMD NOT FOUND
	xprintf("ERROR\r\nCMD NOT DEFINED\r\n\0");
	return;
 
}
//------------------------------------------------------------------------------
static void pv_snprintfP_OK(void )
{
	xprintf("ok\r\n\0");
}
//------------------------------------------------------------------------------
static void pv_snprintfP_ERR(void)
{
	xprintf("error\r\n\0");
}
//------------------------------------------------------------------------------
static bool test_valves( char *vid, char *action)
{
    
uint8_t valve_id;

    // enable, disable
    if (!strcmp_P( strupr(vid), PSTR("ENABLE"))  ) {
        ENABLE_VALVES();
        return (true);
    }

    if (!strcmp_P( strupr(vid), PSTR("DISABLE"))  ) {
        DISABLE_VALVES();
        return (true);
    }

    // {1|2} {open|close}
    valve_id = atoi(vid);
    if (valve_id > 2 ) {
        return(false);
    } 
    
    switch (valve_id) {
        case 1:
            if (!strcmp_P( strupr(action), PSTR("OPEN"))  ) {
                OPEN_VALVE1();
                return (true);
            }
            if (!strcmp_P( strupr(action), PSTR("CLOSE"))  ) {
                CLOSE_VALVE1();
                return (true);
            }
            return(false);
            break;
            
        case 2:
            if (!strcmp_P( strupr(action), PSTR("OPEN"))  ) {
                OPEN_VALVE2();
                return (true);
            }
            if (!strcmp_P( strupr(action), PSTR("CLOSE"))  ) {
                CLOSE_VALVE2();
                return (true);
            }
            return(false);
            break;            
    }
    return(false);
}
//------------------------------------------------------------------------------