#include "main.h"

static int32_t steps = 0;			// counter for steps made by step motor
static int32_t keys_array[25];      // array that contains keys that should be played

int main(void)
{
	// initialization
    step_motor_init();
    servo_init();
	usart_init();

	create_keys_array();
	
#ifdef Calibrate
	// blink to show that program is ready for calibration
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
	delay(20000000);
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);

	calibrate();
#endif
	uint16_t length = 0;
	get_byte(&length);
	uint16_t notes[length];
	uint16_t delays[length];
	receive_music(notes, delays, length);
	// wait for finger button pushed to start playing music
	while (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)){
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
		delay(1000000);
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
		delay(1000000);
	}
	play_music(notes, delays, length);
	
	// light LED up to show that track is over
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);

	while (1){}

	return 0;
}

void delay(uint32_t time){
	for (uint32_t i = 0; i <= time; i ++){}
}

void servo_init(void) {
    GPIO_InitTypeDef port;
    TIM_TimeBaseInitTypeDef timer;
    TIM_OCInitTypeDef timerPWM; 

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	// B7- PWM
	GPIO_StructInit(&port);
	port.GPIO_Mode = GPIO_Mode_AF_PP;
	port.GPIO_Pin = GPIO_Pin_7;
	port.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &port);

	// TIM4 init
	TIM_TimeBaseStructInit(&timer);
	timer.TIM_Prescaler = PRESCALER;
	timer.TIM_Period = SYSCLK / PRESCALER / 50;
	timer.TIM_ClockDivision = 0;
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &timer);

	// PWM channel2 init
	TIM_OCStructInit(&timerPWM);
	timerPWM.TIM_Pulse = FINGER_UP;
	timerPWM.TIM_OCMode = TIM_OCMode_PWM1;
	timerPWM.TIM_OutputState = TIM_OutputState_Enable;
	timerPWM.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM4, &timerPWM);

    TIM_Cmd(TIM4, ENABLE);
}

void step_motor_init(void){
    GPIO_InitTypeDef GPIOC_init;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);
	
	// C13 - LED
    GPIOC_init.GPIO_Pin = GPIO_Pin_13;
    GPIOC_init.GPIO_Speed = GPIO_Speed_10MHz;
    GPIOC_init.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIOC_init);

	GPIO_InitTypeDef GPIOA_init;

	// A0 - STEP
	GPIOA_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIOA_init.GPIO_Speed = GPIO_Speed_10MHz;
	GPIOA_init.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIOA_init);

	// A1- DIR
	GPIOA_init.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIOA_init.GPIO_Speed = GPIO_Speed_10MHz;
	GPIOA_init.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIOA_init);

}

void usart_init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitTypeDef port;
	GPIO_StructInit(&port);

    // A10 - RX
	port.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	port.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &port);

    //A9 - TX
    port.GPIO_Mode = GPIO_Mode_AF_PP;
    port.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOA, &port);

    // USART Init
    USART_InitTypeDef usart;
    USART_StructInit(&usart);
    USART_Init(USART1, &usart);

    USART_Cmd(USART1, ENABLE);

}

void move_motor(uint16_t direction, uint16_t dly){
	if (direction == 1){
		GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
		steps --;
	}
	else{
		GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);
		steps ++;
	}
	// make one step
	GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
	delay(dly);
	GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
	delay(dly);
}

void move_finger(uint16_t angle){
    TIM4->CCR2 = angle;
}

void go_to_key(uint8_t key, uint16_t dly){
	if (keys_array[key] > steps){
		while(steps != keys_array[key])
			move_motor(MOTOR_LEFT, dly);
	}
	else if (keys_array[key] < steps)
		while(steps != keys_array[key])
			move_motor(MOTOR_RIGHT, dly);
}

/* going to every key and push it */
void try_keys(void){
	for (uint8_t i = 0; i < 25; i++)
	{
		go_to_key(i, 6000);
		move_finger(FINGER_DOWN);
		delay(2000000);
		move_finger(FINGER_UP);
		delay(30000);
	}
	
}

/* reset step value and starts checking all keys */
void calibrate(void){
	while(1){
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)){
			move_motor(MOTOR_RIGHT, 8000);
		}
		else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)){
			move_motor(MOTOR_LEFT, 8000);
		}

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)){
            move_finger(FINGER_DOWN);
			steps = C2_STEPS;
			delay(10000000);
			move_finger(FINGER_UP);
		#ifdef Try_Key
			try_keys();
		#endif
			return;
		}
	}
}

void create_keys_array(void){
	for (uint8_t i = 0; i < 24; i++)
	{
		keys_array[i] = i * 150;
	}
	keys_array[24] = keys_array[23] + 200;
}

/* geting one byte via USART
   if received byte is unexpected, requesting second attempt
   if second attempt is failed, blocks program and blinking C13 LED*/
void get_byte(uint16_t* byte){
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE)){}
	USART_SendData(USART1, 0xAA);

	while(!USART_GetFlagStatus(USART1, USART_FLAG_RXNE)){}
	*byte = (uint16_t) USART_ReceiveData(USART1);
	
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE)){}
	USART_SendData(USART1, 0xAB);
}

/*fills notes and delays array with bytes received via USART*/
void receive_music(uint16_t * notes, uint16_t * delays, uint16_t length){
	for (uint8_t i = 0; i < length; i++)
	{
		get_byte(&notes[i]);
	}

	uint16_t end_byte = 0;
	get_byte(&end_byte);

	if (end_byte != END_OF_NOTES){
		while (1){
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
			delay(20000000);
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
			delay(10000000);
		}
	}

	for (uint8_t i = 0; i < length; i++)
	{
		get_byte( &delays[i]);
	}
}

void play_music(uint16_t * notes, uint16_t * delays, uint8_t length){
	for (uint8_t i = 0; i < length; i++)
	{
		go_to_key(notes[i] - 1, 2000);
		move_finger(FINGER_DOWN);
		delay(delays[i] * 5000000);
		move_finger(FINGER_UP);
		delay(500000);
	}
}