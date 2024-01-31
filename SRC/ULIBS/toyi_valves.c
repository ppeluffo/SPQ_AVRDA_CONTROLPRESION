
#include "toyi_valves.h"

// -----------------------------------------------------------------------------
void VALVES_EN_init(void)
{
    // Configura el pin como output
	VALVES_EN_PORT.DIR |= VALVES_EN_PIN_bm;	
	DISABLE_VALVES();
}
// -----------------------------------------------------------------------------
void VALVE1_init(void)
{
    // Configura el pin como output
	VALVE_CTRL1_PORT.DIR |= VALVE_CTRL1_PIN_bm;	
	CLOSE_VALVE1();
}
// -----------------------------------------------------------------------------
void VALVE2_init(void)
{
    // Configura el pin como output
	VALVE_CTRL2_PORT.DIR |= VALVE_CTRL2_PIN_bm;	
	CLOSE_VALVE2();
}
// -----------------------------------------------------------------------------
void VALVES_init(void)
{
    VALVES_EN_init();
    VALVE1_init();
    VALVE2_init();
}
// -----------------------------------------------------------------------------
t_valves_status *get_valves_status(void)
{
    return &VALVES_STATUS;
}
// -----------------------------------------------------------------------------
