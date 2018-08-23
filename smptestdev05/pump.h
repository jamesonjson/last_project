#include "tmacros.h"
//#include "system.h"

typedef struct{
    bool state;
    bool state_before;
    rtems_interval last_state_change;
} Pump;

static Pump pump; //singleton

static void set_pump_state(bool new_state){
    rtems_interval ct;
    //adquirir o mutex
    ct = rtems_clock_get_ticks_since_boot();
    pump.state = new_state;
    pump.last_state_change = ct;
    //libertar o mutex
}


