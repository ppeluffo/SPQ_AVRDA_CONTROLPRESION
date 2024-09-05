#include "CPRES.h"
#include "frtos_cmd.h"
        
#define RS485_RX_BUFFER_SIZE 128
uint8_t rs485_rx_buffer[RS485_RX_BUFFER_SIZE];
lBuffer_s rs485_rx_lbuffer;

/*
char tmpLocalStr[64] = { 0 };
char *rs485_argv[16];
void rs485_print_cmdline(void);
uint8_t rs485_makeArgv(void);
void rs485_execCmd(void);
*/

typedef struct {
    bool enabled;
    uint8_t slave_address;		// Direccion del dispositivo en el bus.
    uint16_t reg_address;		// Direccion de la posicion de memoria donde leer.
    uint8_t nro_regs; 			// Cada registro son 2 bytes por lo que siempre leemos 2 x N.
    uint8_t fcode;
	uint8_t divisor_p10;		// factor de potencia de 10 por el que dividimos para tener la magnitud
	
} modbus_channel_conf_t;

typedef union {
	uint8_t raw_value[4];
	// Interpretaciones
	uint16_t u16_value;
	int16_t i16_value;
	uint32_t u32_value;
	int32_t i32_value;
	float float_value;
	char str_value[4];		// Almaceno NaN cuando hay un error.
} modbus_hold_t; // (4)

#define MBUS_TXMSG_LENGTH	16
#define MBUS_RXMSG_LENGTH	32


typedef struct {
	modbus_channel_conf_t channel;
	uint8_t tx_buffer[MBUS_TXMSG_LENGTH];
	uint8_t rx_buffer[MBUS_RXMSG_LENGTH];
	uint8_t tx_size;		// Cantidad de bytes en el txbuffer para transmitir
	uint8_t rx_size;		// Cantidad de bytes leidos en el rxbufer.
	uint8_t payload_ptr;	// Indice al primer byte del payload.
	modbus_hold_t udata;	// estructura para interpretar los distintos formatos de los datos.
	bool io_status;			// Flag que indica si la operacion de lectura fue correcta o no
} mbus_CONTROL_BLOCK_t;

uint8_t mbus_fsm_state;
typedef enum { FSM_IDLE=0, FCN_CODE, IN_FRAME } fsm_states_t;

#define LOCAL_ADDRESS 0xFE

void MODBUS_fsm_init(void);
void MODBUS_fsm(uint8_t c);

void MODBUS_fsm_process_frame(void);
void MODBUS_fsm_process_frame03(void);
void MODBUS_fsm_process_frame06(void);

void MODBUS_txmit_ADU( mbus_CONTROL_BLOCK_t *mbus_cb );

mbus_CONTROL_BLOCK_t mbus_cb;

uint16_t crc16( uint8_t *msg, uint8_t msg_size );

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

    lBchar_CreateStatic ( &rs485_rx_lbuffer, rs485_rx_buffer, RS485_RX_BUFFER_SIZE );
 
    xprintf_P(PSTR("Starting tkRS485RX..\r\n" ));
  
    RS485_flush_buffer();
    
    MODBUS_fsm_init();
    
	// loop
	for( ;; )
	{
      // u_kick_wdt(RS485RX_WDG_gc);
       
		c = '\0';	// Lo borro para que luego del un CR no resetee siempre el timer.
		// el read se bloquea 50ms. lo que genera la espera.
        while ( xfgetc( fdRS485, (char *)&c ) == 1 ) {
            MODBUS_fsm(c);
        } 
        
        vTaskDelay( ( TickType_t)( 10 / portTICK_PERIOD_MS ) );
	}    
}
//------------------------------------------------------------------------------
void RS485_flush_buffer(void)
{
    lBchar_Flush(&rs485_rx_lbuffer);
}
//------------------------------------------------------------------------------
void RS485_print_buffer(void)
{
 
char *p;
uint8_t count = lBchar_GetCount(&rs485_rx_lbuffer);
uint8_t i;

    xprintf_P(PSTR("COUNT=%d\r\n"), count  );
     
    p = lBchar_get_buffer(&rs485_rx_lbuffer);
    //p = &rs485_rx_buffer[0];
    
    xprintf_P(PSTR("RCVD:"));
    for (i=0; i < count; i++) {
        xprintf_P(PSTR("[0x%02x]"), *p++);
    }
    
    xprintf_P(PSTR("\r\n"));
       
}
//------------------------------------------------------------------------------
void RS485_config_debug(bool debug )
{
    if ( debug ) {
        f_debug_rs485 = true;
    } else {
        f_debug_rs485 = false;
    }
    
}
//------------------------------------------------------------------------------
void MODBUS_fsm_init(void)
{
    mbus_fsm_state = FSM_IDLE;
}
//------------------------------------------------------------------------------
void MODBUS_fsm(uint8_t c)
{
    
    // Almaceno.
    if ( ! lBchar_Put(&rs485_rx_lbuffer, c) ) {
        lBchar_Flush(&rs485_rx_lbuffer);
        return;
    }
    
    // Ejecuto.
    switch(mbus_fsm_state) {
        
        case FSM_IDLE:
            // Estando en idle, el primer caracter debe ser la local address
            // si el frame es para mi
            // Borro el buffer y guardo el byte.
            if ( c == LOCAL_ADDRESS ) {
                // Inicializo
                lBchar_Flush(&rs485_rx_lbuffer);
                lBchar_Put(&rs485_rx_lbuffer, c);
                mbus_fsm_state = FCN_CODE;   
            }
            break;
            
        case FCN_CODE:
            // Solo proceso los frames 3 y 6. El resto los descarto
            // Ya los guarde en el buffer al entrar.
            if ( ( c == 0x03) || ( c == 0x06 ) ) {
                mbus_fsm_state = IN_FRAME;
            } else {
                // Function codes no implementados
                mbus_fsm_state = FSM_IDLE;
                lBchar_Flush(&rs485_rx_lbuffer);  
            }
            break;
        
        case IN_FRAME:
            // Tanto el fcn 3 o 6 son frames de 8 bytes. Controlo esto para
            // ver cuando termino el frame y debo procesarlo
            if (lBchar_GetCount(&rs485_rx_lbuffer) == 8 ) {
                MODBUS_fsm_process_frame();
                mbus_fsm_state = FSM_IDLE;
                lBchar_Flush(&rs485_rx_lbuffer);   
            }
            break;
            
        default:
            mbus_fsm_state = FSM_IDLE;
            lBchar_Flush(&rs485_rx_lbuffer);    
      
    }  
}
//------------------------------------------------------------------------------
void MODBUS_fsm_process_frame(void)
{
    /*
     * El frame recibido esta en rs485_rx_buffer
     */
    
uint16_t crc_rcvd;
uint16_t crc_calculated;
    
    if (f_debug_rs485 ) {
        xprintf_P(PSTR("MODBUS_fsm_process_frame\r\n"));
    }

    // Primero chequeo el CRC
    crc_rcvd = rs485_rx_buffer[7] << 8 | rs485_rx_buffer[6];
    crc_calculated = crc16(&rs485_rx_buffer, 6);
    
    if ( crc_rcvd != crc_calculated ) {
        xprintf_P(PSTR("MBUS RCVD CRC ERROR\r\n"));
        xprintf_P(PSTR("CRC_RX=0x%04x, CRC_CALC=0x%04x\r\n"), crc_rcvd, crc_calculated);
        xprintf_P(PSTR("CRC1=0x%02x, CRC2=0x%02x\r\n"), rs485_rx_buffer[6], rs485_rx_buffer[7]);
        RS485_print_buffer();
        RS485_flush_buffer();
        return;
    }
    
    // Proceso el frame
    switch(rs485_rx_buffer[1]) {
        case 0x03:
            MODBUS_fsm_process_frame03();
            break;
        case 0x06:
            MODBUS_fsm_process_frame06();
            break;
        default:
            xprintf_P(PSTR("MODBUS FCODE ERROR (0x%02x)\r\n"),rs485_rx_buffer[1] );
            break;
    }
    
}
//------------------------------------------------------------------------------
void MODBUS_fsm_process_frame03(void)
{
    /*
     * El frame 03 se usa para leer una direccion
     * FE 03 00 00 00 01 90 05
     */

uint16_t reg_address;
uint16_t nro_regs;
uint8_t size = 0;
uint16_t crc;

    if (f_debug_rs485 ) {
        xprintf_P(PSTR("MODBUS_fsm_process_frame03\r\n"));
        RS485_print_buffer();
    }
   
    reg_address = rs485_rx_buffer[2] << 8 | rs485_rx_buffer[3];
    nro_regs = rs485_rx_buffer[4] << 8 | rs485_rx_buffer[5];
    
    if (f_debug_rs485 ) {
        xprintf_P(PSTR("reg_addr=%u, nro_regs=%u\r\n"), reg_address, nro_regs);
    }
    
    if (reg_address == 0x0) {
        // Retorno el STATUS_REGISTER
        mbus_cb.tx_buffer[0] = LOCAL_ADDRESS;          // [0xFE]
        mbus_cb.tx_buffer[1] = 0x3;                    // FCODE=0x03
        mbus_cb.tx_buffer[2] = 0x2;                    // Byte count ( 2 bytes )
        
        mbus_cb.tx_buffer[3] = (uint8_t) (( systemVars.status_register & 0xFF00  ) >> 8);// DST_ADDR_H
        mbus_cb.tx_buffer[4] = (uint8_t) ( systemVars.status_register & 0x00FF );		 // DST_ADDR_L
    
        size = 5;
        crc = crc16( mbus_cb.tx_buffer, size );
        mbus_cb.tx_buffer[size++] = (uint8_t)( crc & 0x00FF );			// CRC Low
        mbus_cb.tx_buffer[size++] = (uint8_t)( (crc & 0xFF00) >> 8 );	// CRC High
        mbus_cb.tx_size = size;
    
        MODBUS_txmit_ADU( &mbus_cb );
    }
    
    return;
}
//------------------------------------------------------------------------------
void MODBUS_fsm_process_frame06(void)
{
    /*
     * FE 06 00 01 00 01 0D C5  // V1: Close, V2: Open
     * FE 06 00 01 00 02 4D C4  // V1: Open, V2: Close
     * 
     */
    
uint16_t reg_address;
uint16_t reg_value;
uint8_t size = 0;
uint16_t crc;

    if (f_debug_rs485 ) {
        xprintf_P(PSTR("MODBUS_fsm_process_frame06\r\n"));
        RS485_print_buffer();
    }
   
    reg_address = rs485_rx_buffer[2] << 8 | rs485_rx_buffer[3];
    reg_value = rs485_rx_buffer[4] << 8 | rs485_rx_buffer[5];
    
    if (f_debug_rs485 ) {
        xprintf_P(PSTR("reg_addr=%u, reg_value=%u\r\n"), reg_address, reg_value);
    }
    
    if (reg_address == 0x1) {
        
        xprintf_P(PSTR("MODBUS order rcvd: [0x%02x]\r\n"), reg_value );
        systemVars.orders_register = reg_value;
        
        // Retorno lo mismo que recibi.
        mbus_cb.tx_buffer[0] = LOCAL_ADDRESS;          // [0xFE]
        mbus_cb.tx_buffer[1] = 0x6;                    // FCODE=0x06
        
        mbus_cb.tx_buffer[2] = rs485_rx_buffer[2];
        mbus_cb.tx_buffer[3] = rs485_rx_buffer[3];
        
        mbus_cb.tx_buffer[4] = rs485_rx_buffer[4];
        mbus_cb.tx_buffer[5] = rs485_rx_buffer[5];
        
        size = 6;
        crc = crc16( mbus_cb.tx_buffer, size );
        mbus_cb.tx_buffer[size++] = (uint8_t)( crc & 0x00FF );			// CRC Low
        mbus_cb.tx_buffer[size++] = (uint8_t)( (crc & 0xFF00) >> 8 );	// CRC High
        mbus_cb.tx_size = size;
    
        MODBUS_txmit_ADU( &mbus_cb );
    }
    
    return; 
}
//------------------------------------------------------------------------------
void MODBUS_txmit_ADU( mbus_CONTROL_BLOCK_t *mbus_cb )
{
	// Transmite el frame modbus almcenado en mbus_cb.tx_buffer
	//
	// OJO: En MODBUS, los bytes pueden ser 0x00 y no deben ser interpretados como NULL
	// al trasmitir por lo tanto hay que usar el data_size
	// Debemos cumplir extrictos limites de tiempo por lo que primero logueo y luego
	// muestro el debug.

uint8_t i;

   
	// Transmito
	// borro buffers y espero 3.5T (9600) = 3.5ms ( START )
	vTaskDelay( ( TickType_t)( 10 / portTICK_PERIOD_MS ) );

    if (f_debug_rs485 ) {
        xprintf_P(PSTR("MODBUS_txmit_ADU\r\n"));
        xprintf_P( PSTR("len=%d\r\n"), mbus_cb->tx_size);
		for ( i = 0 ; i < mbus_cb->tx_size ; i++ ) {
			xprintf_P( PSTR("[0x%02X]"), mbus_cb->tx_buffer[i]);
		}
		xprintf_P( PSTR("\r\n"));
        
    }
	// La funcion xnprintf_MBUS maneja el control de flujo.
	i = xnprintf( fdRS485, (const char *)mbus_cb->tx_buffer, mbus_cb->tx_size );

}
//------------------------------------------------------------------------------
uint16_t crc16( uint8_t *msg, uint8_t msg_size )
{

uint16_t crc = 0xFFFF;
uint16_t pos;
uint16_t data;
int i;

	for ( pos = 0; pos < msg_size; pos++) {
		data = (uint16_t)*msg++;
        //xprintf_P(PSTR("DATA=0x%02x\r\n"), data);
        
		crc ^= data;          // XOR byte into least sig. byte of crc

		for (i = 8; i != 0; i--) {          // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                  // Shift right and XOR 0xA001
				crc ^= 0xA001;
			} else {                            // Else LSB is not set
				crc >>= 1;                    // Just shift right
			}
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)

    //xprintf_P(PSTR("CRC=0x%04x\r\n"), crc);

	return crc;
}
