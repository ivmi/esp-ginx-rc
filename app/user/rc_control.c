#include "c_types.h"
#include "os_type.h"
#include "osapi.h"

#include "http/ws_gamepad_app.h"

#define SERVO_BIT BIT12
#define SERVO_MUX PERIPHS_IO_MUX_MTDI_U
#define SERVO_FUNC FUNC_GPIO12


os_timer_t servoUpdateTimer;
os_timer_t servoTimer;

static void servoTimerCb(void *arg)
{
    gpio_output_set(0, SERVO_BIT, SERVO_BIT, 0);    
    ets_intr_unlock();

}

static void servoUpdateTimerCb(void *arg)
{
    // update pwm
    //uint32 d = gamepad.y * 222;
    uint32 d = gpDataCmds.tPad[1] * 222;
    
    if (d > 22222) d = 22222;
    if (d < 0) d = 0;
    
    //pwm_set_duty(d, 0);
    //pwm_start();

    // update timer
    
    //int t0 = 1500 + gamepad.x*25;
    int t0 = 1500 + gpDataCmds.accel[1]*15;
    if (t0 < 1000) t0 = 1000;
    if (t0 > 2000) t0 = 2000;
    
    ets_intr_lock();
    os_timer_arm_us(&servoTimer, t0, 0);
    gpio_output_set(SERVO_BIT, 0, SERVO_BIT, 0);
	
    //NODE_DBG("SERVO_PIN: %d", gpDataCmds.buttons & 0x1);
    //if (gpDataCmds.buttons & 0x1)
    //    gpio_output_set(0, SERVO_BIT, SERVO_BIT, 0);
    //else
    //    gpio_output_set(SERVO_BIT, 0, SERVO_BIT, 0);

}

void rc_control_init(void)
{
    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(SERVO_MUX, SERVO_FUNC);
     //Set GPIO2 low
    gpio_output_set(0, SERVO_BIT, SERVO_BIT, 0);    


    //arm servo timer
    os_memset(&servoUpdateTimer,0,sizeof(os_timer_t));
    os_timer_disarm(&servoUpdateTimer);
    os_timer_setfn(&servoUpdateTimer, (os_timer_func_t *)servoUpdateTimerCb, NULL);

    os_memset(&servoTimer,0,sizeof(os_timer_t));
    os_timer_disarm(&servoTimer);
    os_timer_setfn(&servoTimer, (os_timer_func_t *)servoTimerCb, NULL);

    os_timer_arm(&servoUpdateTimer, 40, 1);
}