#ifndef _GPIO_H
#define _GPIO_H

#include "sys.h"

// typedef enum {FALSE = 0,TRUE = 1} bool;

#define HIGH_LIGHT_PORT 		GPIOH
#define HIGH_LIGHT_PIN 			GPIO_PIN_4
#define LOW_LIGHT_PORT 			GPIOH
#define LOW_LIGHT_PIN 			GPIO_PIN_5
#define LEFT_LIGHT_PORT 		GPIOA
#define LEFT_LIGHT_PIN 			GPIO_PIN_4
#define RIGHT_LIGHT_PORT 		GPIOA
#define RIGHT_LIGHT_PIN 		GPIO_PIN_5
#define WARN_LIGHT_PORT 		GPIOD
#define WARN_LIGHT_PIN 			GPIO_PIN_14
#define DRIVING_LIGHT_PORT 		GPIOA
#define DRIVING_LIGHT_PIN 		GPIO_PIN_6
#define BREAK_LIGHT_PORT		GPIOA
#define BREAK_LIGHT_PIN			GPIO_PIN_7
#define REAR_LIGHT_PORT 		GPIOE
#define REAR_LIGHT_PIN 			GPIO_PIN_14
#define HORN_PORT 				GPIOE
#define HORN_PIN 				GPIO_PIN_15
#define RESERVE_RELAY_PORT 		GPIOD
#define RESERVE_RELAY_PIN 		GPIO_PIN_15

#define HIGH_LIGHT 				PHout(4)
#define LOW_LIGHT 				PHout(5)
#define LEFT_LIGHT 				PAout(4)
#define RIGHT_LIGHT 			PAout(5)
#define WARN_LIGHT				PDout(14)
#define DRIVING_LIGHT			PAout(6)
#define BREAK_LIGHT				PAout(7)
#define REAR_LIGHT				PEout(14)
#define HORN					PEout(15)
#define RESERVE_RELAY			PDout(15)



#define BUMPERE0_PORT			GPIOE
#define BUMPERE0_PIN 			GPIO_PIN_0
#define BUMPERE1_PORT			GPIOE
#define BUMPERE1_PIN 			GPIO_PIN_1
#define BUMPERI4_PORT			GPIOI
#define BUMPERI4_PIN 			GPIO_PIN_4
#define BUMPERI5_PORT			GPIOI
#define BUMPERI5_PIN 			GPIO_PIN_5
#define BUMPERI6_PORT			GPIOI
#define BUMPERI6_PIN 			GPIO_PIN_6
#define BUMPERI7_PORT			GPIOI
#define BUMPERI7_PIN 			GPIO_PIN_7


#define BUMPERE0				PEin(0)
#define BUMPERE1				PEin(1)
#define BUMPERI4				PIin(4)
#define BUMPERI5				PIin(5)
#define BUMPERI6				PIin(6)
#define BUMPERI7				PIin(7)

extern u8 rece_light_state1;
extern u8 rece_light_state2;
extern u8 send_light_state1;

extern u8 BREAK_LIGHT_SLAVE;

void MX_GPIO_Init(void);
void update_light_state(void);
void complex_light_function(void);
void horn_1_second(void);



#endif  //_GPIO_H


