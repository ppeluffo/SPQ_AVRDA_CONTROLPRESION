
#include "toyi_valves.h"

// -----------------------------------------------------------------------------
void VALVES_EN_init(void)
{
    // Configura el pin como output
	VALVES_EN_PORT.DIR |= VALVES_EN_PIN_bm;	
	DISABLE_VALVES();
}
// -----------------------------------------------------------------------------
void VALVE_0_init(void)
{
    // Configura el pin como output
	VALVE_0_CTRL_PORT.DIR |= VALVE_0_CTRL_PIN_bm;	
}
// -----------------------------------------------------------------------------
void VALVE_1_init(void)
{
    // Configura el pin como output
	VALVE_1_CTRL_PORT.DIR |= VALVE_1_CTRL_PIN_bm;	
}
// -----------------------------------------------------------------------------
void VALVES_init(void)
{
    VALVES_EN_init();
    VALVE_0_init();
    VALVE_1_init();
}
// -----------------------------------------------------------------------------
