/*
 * frtos-io.c
 *
 *  Created on: 11 jul. 2018
 *      Author: pablo
 */

#include "frtos-io.h"
#include "pines.h"

#define USART_IsTXDataRegisterEmpty(_usart) (((_usart)->STATUS & USART_DREIF_bm) != 0)
#define USART_IsTXShiftRegisterEmpty(_usart) (((_usart)->STATUS & USART_TXCIF_bm) != 0)
#define USART_PutChar(_usart, _data) ((_usart)->TXDATAL = _data)

//------------------------------------------------------------------------------------
int16_t frtos_open( file_descriptor_t fd, uint32_t flags)
{
	// Funcion general para abrir el puerto que invoca a una mas
	// especializada para c/periferico.
	// Es la que invoca la aplicacion.
	// Retorna -1 en error o un nro positivo ( fd )

int16_t xRet = -1;

	switch(fd) {
                
	case fdTERM:
		frtos_open_uart2( flags );
        xRet=0;
		break;
        
	case fdRS485:
		frtos_open_uart1( flags );
        xRet=0;
		break;
        
    case fdNVM:
		xRet = frtos_open_nvm( &xNVM, fd, &NVM_xMutexBuffer, flags );
		break;
                
	default:
		break;
	}


	return(xRet);
}
//------------------------------------------------------------------------------------
int16_t frtos_ioctl( file_descriptor_t fd, uint32_t ulRequest, void *pvValue )
{

int16_t xRet = -1;

	switch(fd) {
                
        case fdTERM:
            xRet = frtos_ioctl_uart2( ulRequest, pvValue );
            break;

        case fdRS485:
            xRet = frtos_ioctl_uart1( ulRequest, pvValue );
            break;
            
        case fdNVM:
            xRet = frtos_ioctl_nvm( &xNVM, ulRequest, pvValue );
            break;
 
        
        default:
            break;
	}

	return(xRet);
}
//------------------------------------------------------------------------------------
int16_t frtos_write( file_descriptor_t fd ,const char *pvBuffer, const uint16_t xBytes )
{

int16_t xRet = -1;

	switch(fd) {
                
	case fdTERM:
		xRet = frtos_write_uart2( pvBuffer, xBytes );
		break;

    case fdRS485:
		xRet = frtos_write_uart1( pvBuffer, xBytes );
		break;
        
    case fdNVM:
		xRet = frtos_write_nvm( &xNVM, pvBuffer, xBytes );
		break;
                
	default:
		break;
	}

	return(xRet);
}
//------------------------------------------------------------------------------
int16_t frtos_read( file_descriptor_t fd , char *pvBuffer, uint16_t xBytes )
{

int16_t xRet = -1;

	switch(fd) {
    
	case fdTERM:
		xRet = frtos_read_uart2( pvBuffer, xBytes );
		break;
   
    case fdRS485:
		xRet = frtos_read_uart1( pvBuffer, xBytes );
		break;
        
    case fdNVM:
		xRet = frtos_read_nvm( &xNVM, pvBuffer, xBytes );
		break;
   
 	default:
		break;
	}

	return(xRet);
}
//------------------------------------------------------------------------------
// FUNCIONES ESPECIFICAS DE UART's

//------------------------------------------------------------------------------
void frtos_open_uart1(uint32_t baudrate)
{
    drv_uart1_init(baudrate);
    CLEAR_RTS_RS485();
}
//------------------------------------------------------------------------------
void frtos_open_uart2(uint32_t baudrate)
{
    drv_uart2_init(baudrate);
}
//------------------------------------------------------------------------------
int16_t frtos_write_uart1_old( const char *pvBuffer, const uint16_t xBytes )
{
    
uint16_t i;
    
    // Transmision x poleo ( No hablito al INT x DRIE )
    //for( i = 0; i < strlen(pvBuffer); i++) {
    for( i = 0; i < xBytes; i++) {
        while(! USART_IsTXDataRegisterEmpty(&USART1) )
            ;
        USART_PutChar(&USART1, pvBuffer[i]);
    }
    vTaskDelay( ( TickType_t)( 1 ) );
    return(xBytes);   
}
//------------------------------------------------------------------------------
int16_t frtos_write_uart1_deprecated( const char *pvBuffer, const uint16_t xBytes )
{
   // Hago control de flujo ya que el SP3485 debe operarse HALF-DUPLEX !!
	// Trasmite el buffer sin considerar si tiene NULL 0x00 en el medio.
	// Transmite en forma transparente los xBytes por poleo de modo que controlo exactamente
	// cuando termino de transmitir c/byte.
	//
char cChar = '\0';
char *p = NULL;
int16_t wBytes = 0;
uint16_t i;

	// RTS ON. Habilita el sentido de trasmision del chip.
	SET_RTS_RS485();
	vTaskDelay( ( TickType_t)( 5 ) );
    p = (char *)pvBuffer;  
    
    // Transmision x poleo ( No hablito al INT x DRIE )
    taskENTER_CRITICAL();
    for( i = 0; i < xBytes; i++) {
        while(! USART_IsTXDataRegisterEmpty(&USART1) )
            ;
        // Voy cargando la cola de a uno.
		cChar = *p;
		// Delay inter chars.(Shinco, Taosonic = 2)
		//vTaskDelay( ( TickType_t)( 2 ) );
        //_delay_us (1750);
        USART_PutChar(&USART1, cChar );
		p++;
		wBytes++;	// Cuento los bytes que voy trasmitiendo
        while(! USART_IsTXDataRegisterEmpty(&USART1) )
            ;
    }
    
    taskEXIT_CRITICAL();
    // Para evitar el loopback del puerto RS485
    frtos_ioctl( fdRS485, ioctl_UART_CLEAR_RX_BUFFER, NULL );
    vTaskDelay( ( TickType_t)( 2 ) );
	// RTS OFF: Habilita la recepcion del chip
	CLEAR_RTS_RS485();
    
	return (wBytes);
    
}
//------------------------------------------------------------------------------
int16_t frtos_write_uart1( const char *pvBuffer, const uint16_t xBytes )
{
   // Hago control de flujo ya que el SP3485 debe operarse HALF-DUPLEX !!
	// Trasmite el buffer sin considerar si tiene NULL 0x00 en el medio.
	// Transmite en forma transparente los xBytes por poleo de modo que controlo exactamente
	// cuando termino de transmitir c/byte.
	//
char cChar = '\0';
char *p = NULL;
int16_t wBytes = 0;
uint16_t i;

	// RTS ON. Habilita el sentido de trasmision del chip.
	SET_RTS_RS485();
	vTaskDelay( ( TickType_t)( 5 ) );
    p = (char *)pvBuffer;  
    
    // Transmision x poleo ( No hablito al INT x DRIE )
    taskENTER_CRITICAL();
    for( i = 0; i < xBytes; i++) {
        
        // Espero que el TXDATA reg. este vacio.
        while( (USART1.STATUS & USART_DREIF_bm) == 0 )
            ;
        
        // Voy cargando la cola de a uno.
		cChar = *p;
		// Delay inter chars.(Shinco, Taosonic = 2)
		//vTaskDelay( ( TickType_t)( 2 ) );
        //_delay_us (1750);
        USART_PutChar(&USART1, cChar );
		p++;
		wBytes++;	// Cuento los bytes que voy trasmitiendo

    }

    // Espero que salga el ultimo byte
    while( ( USART1.STATUS &  USART_TXCIF_bm) == 0 )
            ;
    // Borro la flag
    USART1.STATUS |= ( 1 << USART_TXCIF_bp);
    
    vTaskDelay( ( TickType_t)( 1 ) );
    
    taskEXIT_CRITICAL();
    // Para evitar el loopback del puerto RS485
    frtos_ioctl( fdRS485, ioctl_UART_CLEAR_RX_BUFFER, NULL );
    vTaskDelay( ( TickType_t)( 2 ) );
	// RTS OFF: Habilita la recepcion del chip
	CLEAR_RTS_RS485();
    
	return (wBytes);
    
}
//------------------------------------------------------------------------------
int16_t frtos_write_uart2( const char *pvBuffer, const uint16_t xBytes )
{
    
uint16_t i;
    
    // Transmision x poleo ( No hablito al INT x DRIE )
    //for( i = 0; i < strlen(pvBuffer); i++) {
    for( i = 0; i < xBytes; i++) {
        while(! USART_IsTXDataRegisterEmpty(&USART2) )
            ;
        USART_PutChar(&USART2, pvBuffer[i]);
    }
    vTaskDelay( ( TickType_t)( 1 ) );
    return(xBytes);   
}
//------------------------------------------------------------------------------
int16_t frtos_ioctl_uart1( uint32_t ulRequest, void *pvValue )
{

int16_t xReturn = 0;

	switch( ulRequest )
	{
		case ioctl_UART_CLEAR_TX_BUFFER:
			rBchar_Flush(&TXRB_uart1);
			break;

        case ioctl_UART_CLEAR_RX_BUFFER:
			rBchar_Flush(&RXRB_uart1);
			break;
            
		default :
			xReturn = -1;
			break;
	}

	return xReturn;

}
//------------------------------------------------------------------------------
int16_t frtos_ioctl_uart2( uint32_t ulRequest, void *pvValue )
{

int16_t xReturn = 0;

	switch( ulRequest )
	{
		case ioctl_UART_CLEAR_TX_BUFFER:
			rBchar_Flush(&TXRB_uart2);
			break;
            
        case ioctl_UART_CLEAR_RX_BUFFER:
			rBchar_Flush(&RXRB_uart2);
			break;

		default :
			xReturn = -1;
			break;
	}

	return xReturn;

}
//------------------------------------------------------------------------------
int16_t frtos_read_uart1( char *pvBuffer, uint16_t xBytes )
{
	// Lee caracteres de la cola de recepcion y los deja en el buffer.
	// El timeout lo fijo con ioctl.

int16_t xBytesReceived = 0U;
TickType_t xTicksToWait = 10;
TimeOut_t xTimeOut;

     /* Initialize xTimeOut.  This records the time at which this function was
        entered. 
      */
	vTaskSetTimeOutState( &xTimeOut );

	// Are there any more bytes to be received?
	while( xBytesReceived < xBytes )
	{

        if( rBchar_Pop( &RXRB_uart1, &((char *)pvBuffer)[ xBytesReceived ] ) == true ) {
			xBytesReceived++;
            /*
             Recibi un byte. Re-inicio el timeout.
             */
            vTaskSetTimeOutState( &xTimeOut );
			//taskYIELD();
            //vTaskDelay( ( TickType_t)( 1 ) );
		} else {
			// Espero xTicksToWait antes de volver a chequear
			vTaskDelay( ( TickType_t)( 1 ) );

            // Time out has expired ?
            if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
            {
                break;
            }
        }

    }

	return ( xBytesReceived );

}
//------------------------------------------------------------------------------
int16_t frtos_read_uart2( char *pvBuffer, uint16_t xBytes )
{
	// Lee caracteres de la cola de recepcion y los deja en el buffer.
	// El timeout lo fijo con ioctl.

int16_t xBytesReceived = 0U;
TickType_t xTicksToWait = 10;
TimeOut_t xTimeOut;

     /* Initialize xTimeOut.  This records the time at which this function was
        entered. 
      */
	vTaskSetTimeOutState( &xTimeOut );

	// Are there any more bytes to be received?
	while( xBytesReceived < xBytes )
	{

        if( rBchar_Pop( &RXRB_uart2, &((char *)pvBuffer)[ xBytesReceived ] ) == true ) {
			xBytesReceived++;
            /*
             Recibi un byte. Re-inicio el timeout.
             */
            vTaskSetTimeOutState( &xTimeOut );
			//taskYIELD();
            //vTaskDelay( ( TickType_t)( 1 ) );
		} else {
			// Espero xTicksToWait antes de volver a chequear
			vTaskDelay( ( TickType_t)( 1 ) );

            // Time out has expired ?
            if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
            {
                break;
            }
        }

    }

	return ( xBytesReceived );

}
//------------------------------------------------------------------------------
// FUNCIONES ESPECIFICAS DE NVM
//------------------------------------------------------------------------------
int16_t frtos_open_nvm( periferico_nvm_t *xNVM, file_descriptor_t fd, StaticSemaphore_t *nvm_semph, uint32_t flags)
{
	// Asigno las funciones particulares ed write,read,ioctl
	xNVM->fd = fd;
	xNVM->xBusSemaphore = xSemaphoreCreateMutexStatic( nvm_semph );
	xNVM->xBlockTime = (10 / portTICK_PERIOD_MS );
	//
	// Abro e inicializo el puerto I2C solo la primera vez que soy invocado
	//drv_I2C_init();
	return(1);
}
//------------------------------------------------------------------------------
int16_t frtos_ioctl_nvm( periferico_nvm_t *xNVM, uint32_t ulRequest, void *pvValue )
{

int16_t xReturn = 0;
    
uint32_t *p = NULL;

	p = pvValue;

	switch( ulRequest )
	{
		case ioctl_OBTAIN_BUS_SEMPH:
			// Espero el semaforo en forma persistente.
			while ( xSemaphoreTake(xNVM->xBusSemaphore, ( TickType_t ) 5 ) != pdTRUE )
				taskYIELD();
			break;
			case ioctl_RELEASE_BUS_SEMPH:
				xSemaphoreGive( xNVM->xBusSemaphore );
				break;
			case ioctl_SET_TIMEOUT:
				xNVM->xBlockTime = *p;
				break;
			case ioctl_NVM_SET_EEADDRESS:
				xNVM->eeAddress = *p;
				break;                
			default :
				xReturn = -1;
				break;
		}

	return xReturn;

}
//------------------------------------------------------------------------------
int16_t frtos_read_nvm( periferico_nvm_t *xNVM, char *pvBuffer, const uint16_t xBytes )
{

int16_t xReturn = xBytes;

    FLASH_0_read_eeprom_block( xNVM->eeAddress, (uint8_t *)pvBuffer, xBytes );
    return(xReturn);
}
//------------------------------------------------------------------------------
int16_t frtos_write_nvm( periferico_nvm_t *xNVM, const char *pvBuffer, const uint16_t xBytes )
{

nvmctrl_status_t nvm_status;

    nvm_status = FLASH_0_write_eeprom_block( xNVM->eeAddress, (uint8_t *)pvBuffer, xBytes);
    if ( nvm_status == NVM_OK) {
        return(xBytes);
    }
    return(-1);

}
//------------------------------------------------------------------------------
// FUNCIONES ESPECIFICAS DEL BUS I2C/TWI
//------------------------------------------------------------------------------
