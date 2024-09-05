
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
        
    }  else if ( !strcmp_P( strupr(argv[1]), PSTR("CONFIG"))) {
		xprintf_P( PSTR("-config:\r\n"));
        xprintf_P( PSTR("  debug {rs485} {true/false}\r\n"));
        xprintf_P( PSTR("  save\r\n"));
 
    	// HELP RESET
	} else if (!strcmp_P( strupr(argv[1]), PSTR("RESET"))) {
		xprintf_P( PSTR("-reset\r\n"));
        xprintf_P( PSTR("  memory {soft|hard}\r\n"));
		return;
        
    } else if (!strcmp_P( strupr(argv[1]), PSTR("TEST"))) {
		xprintf_P( PSTR("-test\r\n"));
        xprintf_P( PSTR("  kill {wan,sys}\r\n"));
        xprintf_P( PSTR("  rs485 {read, write}\r\n"));
        xprintf_P( PSTR("  statusreg {val}\r\n"));
        xprintf_P( PSTR("  valve {0|1} {open|close}\r\n"));
        xprintf_P( PSTR("  venable {on|off}\r\n"));
        xprintf_P( PSTR("  consigna {cmd|sys} {diurna|nocturna}\r\n"));
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
static void cmdTestFunction(void)
{

    FRTOS_CMD_makeArgv();

    // test venable {on|off}
    if (!strcmp_P( strupr(argv[1]), PSTR("VENABLE"))  ) {
        if (!strcmp_P( strupr(argv[2]), PSTR("ON"))  ) {
            ENABLE_VALVES();
            pv_snprintfP_OK();
            return;
        }
        
        if (!strcmp_P( strupr(argv[2]), PSTR("OFF"))  ) {
            DISABLE_VALVES();
            pv_snprintfP_OK();
            return;
        }
        pv_snprintfP_ERR();
        return;
    }
            
    // test valve {0|1} {open|close}
    if (!strcmp_P( strupr(argv[1]), PSTR("VALVE"))  ) {
        test_valve( atoi(argv[2]) , argv[3]) ? pv_snprintfP_OK() : pv_snprintfP_ERR();
        return;
    }
    
    // test consigna {cmd|sys} {diurna|nocturna}
    if (!strcmp_P( strupr(argv[1]), PSTR("CONSIGNA"))  ) {
        test_set_consigna( argv[2], argv[3]) ? pv_snprintfP_OK() : pv_snprintfP_ERR();
        return;
    }

    if (!strcmp_P( strupr(argv[1]), PSTR("STATUSREG"))  ) {
        systemVars.status_register = atoi(argv[2]);
        return;       
    }
    
    if (!strcmp_P( strupr(argv[1]), PSTR("RS485"))  ) {

        if (!strcmp_P( strupr(argv[2]), PSTR("READ"))  ) {
            RS485_print_buffer();
            RS485_flush_buffer();
            return;
        }
        
        if (!strcmp_P( strupr(argv[2]), PSTR("WRITE"))  ) {
            xfprintf_P(fdRS485, PSTR("The quick brown fox jumps over the lazy dog 0123456789\r\n"));
            return;
        }
        
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
static void cmdReadFunction(void)
{
    
    FRTOS_CMD_makeArgv();       
        
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


    xprintf("Spymovil %s %s TYPE=%s, VER=%s %s \r\n" , HW_MODELO, FRTOS_VERSION, FW_TYPE, FW_REV, FW_DATE);
         
    xprintf_P(PSTR("Status_reg = 0x%04x\r\n"), systemVars.status_register);
    xprintf_P(PSTR("Orders_reg = 0x%04x\r\n"), systemVars.orders_register);
        
        
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

    
    // DEBUG
    // config debug (rs485) (true,false)
    if (!strcmp_P( strupr(argv[1]), PSTR("DEBUG")) ) {
        if (!strcmp_P( strupr(argv[2]), PSTR("RS485")) ) {
            if (!strcmp_P( strupr(argv[3]), PSTR("TRUE")) ) {
                RS485_config_debug(true);
                pv_snprintfP_OK();
                return;
            }
        
            if (!strcmp_P( strupr(argv[3]), PSTR("FALSE")) ) {
                RS485_config_debug(false);
                pv_snprintfP_OK();
                return;
            }
            
        }
        pv_snprintfP_ERR();
		return;
    }
    
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
