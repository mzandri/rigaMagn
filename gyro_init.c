/*
 * gyro_init.c
 *
 *  tiva c
 *  Inertial sensors
 *  Author: robocupjr 15
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "gyro_init.h"
#include "uartp/uartstdio.h"
#include "I2C/tiva_i2c.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "gen_def.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "uartp/uartstdio.h"
#include "uartp/uart.h"

void blinkBlueLed(){
	volatile uint32_t i;
	/// per segnalalre la presenza del giroscopio lampeggia 2 volte
	GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED, BLUE_LED);
	for (i = 3000000; i > 0; i--);
	GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED, 0);
	for (i = 3000000; i > 0; i--);
	GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED, BLUE_LED);
	for (i = 3000000; i > 0; i--);
	GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED, 0);
}


void blinkRedLed(){
	volatile uint32_t i;
	/// per segnalalre la presenza dell'accelerometro lampeggia 2 volte
	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, RED_LED);
	for (i = 3000000; i > 0; i--);
	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, 0);
	for (i = 3000000; i > 0; i--);
	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, RED_LED);
	for (i = 3000000; i > 0; i--);
	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, 0);
}


//void initGyro(gyro *G, char assi){
//	volatile uint32_t valore;
//	G->IsOn = OFF;
//	G->yaw = G->roll = G->pitch = 0;
//	//chiedo il dato presente nel registro WHO_AM_I
//	if (I2CReceive(GYRO_ADDR, WHO_AM_I) == 0xD4){
//		blinkBlueLed();
//		G->IsPresent = OK;
//		/// imposta gli assi
//		/// per il drone, stato = ALL (0x7)
//		/// per il rover stato = Z (0x4);
//		setupAssi(G, assi);
//	}
//	else{
//		G->IsPresent = NOT_PRESENT;
//		PRINTF("Giroscopio non presente\n");
//		return;
//	}
//}

///
/// imposta gli assi che dovranno essere attivati e memorizza lo stato
//void setupAssi(gyro *G, char stato){
//
//	char mask;
//	uint32_t valore;
//	G->x0 = G->y0 = G->z0 = 32767;
//	G->pitchF = G->rollF = G->yawF = G->pitchP = G->rollP = G->yawP = 0.0;
//	G->gradiSec = 0;
//	G->kz = 1.1688;
//	/// lo stato e' cosi' interpretato: bit0: x; bit1: y; bit2: z.
//	/// scrivo nel registro 0x20 il valore 0x0C, cioe' banda minima, modulo on e assi on
//	/// sintassi: indirizzo slave, num parm, indirizzo reg, valore da scrivere
//	mask = 0x08| stato;
//	I2CSend(GYRO_ADDR, 2, CTRL_REG1, mask);
//	if(I2CReceive(GYRO_ADDR, CTRL_REG1) == mask){
//		//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
//		G->IsOn = ON;
//		G->asseOn = stato;
//	}
//
//	/// set FS to 500 degree per sec.
//	I2CSend(GYRO_ADDR, 2, CTRL_REG4, FS_500);
//	G->gradiSec = 500;
//	G->FS = (float) 500 / 32768;
//	valore = I2CReceive(GYRO_ADDR,CTRL_REG4);
//	PRINTF("Lettura dal REG4 %d\n", valore);
//}


/// calcola la media degli assi e imposta l'offset
//void azzeraAssi(gyro *G){
//	uint32_t conteggio = 0, valore, i ;
//	int16_t x = 0, y = 0, z = 0;
//	uint8_t buffer[8];
//	switch(G->asseOn){
//	case ALL_AXIS:
//		/// assi ON
//		while(conteggio < 32){
//			/// effettua 32 letture e calcola la media
//			valore = I2CReceive(GYRO_ADDR,STATUS_REG);
//			//PRINTF("REG_STAT 0x%x\n", valore);
//			if (valore != 0){
//				/// tutti gli assi on
//				I2CReceiveN(GYRO_ADDR,OUT_X_L | MUL_READ , 6, buffer);
//				x += (int16_t)((buffer[1]<< 8) + buffer[0]);
//				y += (int)((buffer[3]<< 8) + buffer[2]);
//				z += (int)((buffer[5]<< 8) + buffer[4]);
//				conteggio++;
//				for (i = 0; i < 50000; i++);
//			}
//		}
//		/// calcola la media: divide  per 32
//		x /= 32;
//		y /= 32;
//		z /= 32;
//		/// assegna il valore nella struct G
//		G->x0 = x;
//		G->y0 = y;
//		G->z0 = z;
//	break;
//
//	case Z_AXIS:
//		/// asse z ON
//		while(conteggio < 32){
//			/// effettua 32 letture e calcola la media
//			valore = I2CReceive(GYRO_ADDR,STATUS_REG);
//			//PRINTF("REG_STAT 0x%x\n", valore);
//			if (valore != 0){
//				/// asse z ON
//				I2CReceiveN(GYRO_ADDR,OUT_Z_L | MUL_READ , 2, buffer);
//				z += (int16_t)((buffer[1]<< 8) + buffer[0]);
//				conteggio++;
//				for (i = 0; i < 50000; i++);
//			}
//		}
//		/// calcola la media: divide  per 32
//		z /= 32;
//		/// assegna il valore nella struct G
//		G->z0 = z;
//	break;
//	}
//}

///
/// setup di base
void setupMCU(void){

	volatile uint32_t valore;
	// Set the system clock to run at 80Mhz off PLL with external crystal as
	// reference.
	//
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	/// abilita la porta F
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, BLUE_LED | GREEN_LED | RED_LED);
	/// spegne i led
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED | GREEN_LED | RED_LED, 0);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED | GREEN_LED | RED_LED, GREEN_LED);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED | GREEN_LED | RED_LED, 0);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED | GREEN_LED | RED_LED, BLUE_LED);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED | GREEN_LED | RED_LED, 0);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED | GREEN_LED | RED_LED, RED_LED);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED | GREEN_LED | RED_LED, 0);
	//
	// Set the pin direction and mode.
	//
	/// PF4 e PF0 in ingresso
	//HWREG(GPIO_PORTF_BASE + GPIO_O_DIR) 	&= ~GPIO_PIN_4;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) 	= GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR)		= 0xFF;
	HWREG(GPIO_PORTF_BASE + GPIO_O_DIR) 	&= ~GPIO_PIN_0;
	/// no alternate function
	//HWREG(GPIO_PORTF_BASE + GPIO_O_AFSEL) 	&= ~GPIO_PIN_4;
	HWREG(GPIO_PORTF_BASE + GPIO_O_AFSEL) 	&= ~GPIO_PIN_0;
	/// 2 ma di corrente
	//HWREG(GPIO_PORTF_BASE + GPIO_O_DR2R)	|= GPIO_PIN_4;
	HWREG(GPIO_PORTF_BASE + GPIO_O_DR2R)	|= GPIO_PIN_0;
	/// controllo slew rate off
	//HWREG(GPIO_PORTF_BASE + GPIO_O_SLR)		&= ~GPIO_PIN_4;
	HWREG(GPIO_PORTF_BASE + GPIO_O_SLR)		&= ~GPIO_PIN_0;
	/// pull up
	//HWREG(GPIO_PORTF_BASE + GPIO_O_PUR)		|= GPIO_PIN_4;
	HWREG(GPIO_PORTF_BASE + GPIO_O_PUR)		|= GPIO_PIN_0;
	/// abilitazione della funzione digitale
	//HWREG(GPIO_PORTF_BASE + GPIO_O_DEN)		|= GPIO_PIN_4;
	HWREG(GPIO_PORTF_BASE + GPIO_O_DEN)		|= GPIO_PIN_0;
	/// legge il pin (ricordare lo shift di 2 posizione verso sinistra della maschera di bit
	///  p. 654 U.G.)
	/// attesa per l'attuazione delle impostazioni
	valore = 10;
	while(--valore);
	while(HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) == GPIO_PIN_0);
	//while(HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) == GPIO_PIN_0);



	//setup PB0 per scopi di debug
	/// ******** NON PUO' ESSERE USATO QUANDO SI IMPOSTA LA UART1 *******
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	/// PB0 in uscita
	HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) 	|= GPIO_PIN_5;
	/// no alternate function
	HWREG(GPIO_PORTB_BASE + GPIO_O_AFSEL) 	&= ~GPIO_PIN_5;
	/// 2 ma di corrente
	HWREG(GPIO_PORTB_BASE + GPIO_O_DR2R)	|= GPIO_PIN_5;
	/// controllo slew rate off
	HWREG(GPIO_PORTB_BASE + GPIO_O_SLR)		&= ~GPIO_PIN_5;
	/// pull up
	//HWREG(GPIO_PORTF_BASE + GPIO_O_PUR)		|= GPIO_PIN_4;
	/// abilitazione della funzione digitale
	HWREG(GPIO_PORTB_BASE + GPIO_O_DEN)		|= GPIO_PIN_5;
	/// imposta il pin (ricordare lo shift di 2 posizione verso sinistra della maschera di bit
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_5 << 2))) |=  GPIO_PIN_5;
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_5 << 2))) &=  ~GPIO_PIN_5;

	/// non accendiamo subito il led verde
	//HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + (GPIO_PIN_3 << 2))) |=  GPIO_PIN_3;

}

extern volatile int procCom;
///
/// routine di attesa
void attesa(int delay){

	while (delay > 0)
		if (procCom == 1){
			procCom = 0;
			delay--;
		}

}

///
/// setup uart
extern volatile uint32_t g_ui32Base;
extern volatile const uint32_t g_ui32UARTBase[3];

void setupUART(int num){
	if (num > 1)
		num = 0;
	/// inizializza UART1
	ConfigureUART(115200, UART1);
	/// inizializza UART0
	ConfigureUART(115200, UART0);
	/// seleziona la uart a cui andra' 'uscita di PRINTF
	/// uart0 e' quella su usb, uart1 e' quella sui pin PB0 e PB1
	//g_ui32Base = g_ui32UARTBase[UART0];
	g_ui32Base = g_ui32UARTBase[num];
}

///
/// messaggio di benvenuto
void PRINT_WELCOME(void){
	int i, valore;
	for(i = 0; i < 10000; i++);
	PRINTF("Modulo robotico\n");
	for(i = 0; i < 10000; i++);
	PRINTF("Lettura dal clock\n");
	valore = ROM_SysCtlClockGet();
	PRINTF("Frequenza del clock %d\n", valore);
}


///
/// calcola i minimi quadrati
void minQuad(int16_t *x, int16_t *y, int numS, float *b, float *a){

	float m, q, p1, p2, p3, p4;
	int s1, s2, s3, s4;
	int i;
	s1 = s2 = s3 = s4 = 0;

	for (i = 0; i < numS; i++){
		s1 += x[i] * y[i];
		s2 += x[i];
		s3 += y[i];
		s4 += x[i] * x[i];
	}

	p1 = (float) s1;
	p2 = (float) s2;
	p3 = (float) s3;
	p4 = (float) s4;

	m = (float) (numS * p1 - p2 * p3)/(numS * p4 - p2 * p2);
	q = ((float) p3  - m * (float) p2) / numS;

	*b = m;
	*a = q;

}
