#include "stm32f10x.h"
#include "stdbool.h"
#include "stdlib.h"

#define Try_Key True
#define Calibrate True

#define SYSCLK 72000000
#define PRESCALER 72

#define FINGER_UP 900     // PWM wide for finger up state
#define FINGER_DOWN 1200  // PWM wide for finger donw state
#define MOTOR_LEFT 0  
#define MOTOR_RIGHT 1
#define END_OF_NOTES 30   // byte used to show end of notes array
#define C2_STEPS 1800     // calculated step value for C2

void delay(uint32_t time);
void servo_init(void);
void step_motor_init(void);
void usart_init();
void move_motor(uint16_t direction, uint16_t dly);
void move_finger(uint16_t angle);
void go_to_key(uint8_t key, uint16_t dly);
void try_keys(void);
void calibrate(void);
void create_keys_array(void);
void get_byte(uint16_t* byte);
void receive_music(uint16_t * notes, uint16_t * delays, uint16_t length);
void play_music(uint16_t * notes, uint16_t * delays, uint8_t length);
