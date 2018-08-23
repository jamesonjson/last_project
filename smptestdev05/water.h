#include "tmacros.h"
//#include "pump.h"
#include "test_support.h"

#define WATER_LVL_DEFAULT 500
#define WATER_HIGH_CRIT 2000
#define WATER_LOW_CRIT 200

typedef struct{
    uint32_t water_lvl;
} WaterData;

typedef struct{
    bool state;
    bool last_state;
    uint32_t last_state_change;
} PumpData;

typedef struct{
    bool state;
    bool p_last_state;
} CH4;

static WaterData water_ctx; //singleton


rtems_task initial_water_setup(){
    water_ctx.water_lvl = WATER_LVL_DEFAULT;
    //pump.p_last_state = false; //starts off
}

