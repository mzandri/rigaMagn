/*
 * xbee.c
 *
 *  Created on: 08/mar/2015
 *      Author: massimo
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/rom.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "../init.h"

#include "xbee.h"

extern volatile uint8_t uart1buffer[16], RX_PTR1, READ_PTR1;
extern volatile int procCom, tick100;


///
/// funzioncina che prova se il modulo XBEE e' presente
uint32_t testXbee(){
	uint32_t i, stato = 0;
	READ_PTR1 = RX_PTR1 = 0;
	for (i = 0; i < 3; i++)
		while (ROM_UARTCharPutNonBlocking(UART1_BASE, '+') == false);
	i = 0;
	while (i < 100){
		if (procCom == 1){
			i++;
			procCom = 0;
		}
		if (READ_PTR1 != RX_PTR1){
			if (uart1buffer[READ_PTR1] == 'O' && stato == 0){
				/// e' arrivato il primo carattere della risposta
				stato = 1;
				READ_PTR1++;
			}
			if (uart1buffer[READ_PTR1] == 'K' && stato == 1){
				/// e' arrivato il secondo carattere della rispota
				stato = 2;
				READ_PTR1++;
				/// il modul e' presente
				return 0;
			}
			else{
				stato = 4;
				i = 100; /// esce forzatamente
				READ_PTR1++;
			}
		}
	}
	return 1;
}


xBee::xBee(){
	///
	/// usa l'uscita PB.5 come reset del chip, attivo basso.
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5);
	/// pin PB.5 sullo stato 0
	ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);

	present = XBEE_NOT_PRESENT;
	RX_PTR1 = READ_PTR1 = 0;
	for(int i = 0; i < 4; i++){
		m_ADD_H[i] = 0;
		m_ADD_L[i] = 0;
	}
}

///
/// test sulla presenza del modulo radio
///
void xBee::test(){
	/// bufferRX0[BUFF_DIM], Wptr0 = 0, Rptr0 = 0, endMess0 = 0;
	volatile uint32_t rit;
	char buffLoc[3] = {'+', '+', '+'};
	uint32_t i, stato = 0;
	/// toglie il reset
	/// pin PB.5 sullo stato 1
	ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 1);
	/// inizializza la seriale 1 alla velocita' indicata
	//initUART1(115200, FDCO_INT);
	m_Baud = 115200;
	for (rit = 1000; rit > 0; rit--);
	/// invia la richiesta di poll al modulo
	READ_PTR1 = RX_PTR1 = 0;
	for (i = 0; i < 3; i++){
		/// scrive nella uart.
		UARTCharPut(UART1_BASE, buffLoc[i]);
	}
	i = 0;
	status = CMD;
	/// gira per 2 s in attesa della risposta
	tick100 = 0;
	while (tick100 < 200){

		if (READ_PTR1 != RX_PTR1 && uart1buffer[READ_PTR1] == 'O' && stato == 0){
			/// e' arrivato il primo carattere della risposta
			stato = 1;
			READ_PTR1++;
			READ_PTR1 &= DIM_READ_BUFF - 1;
		}
		if (READ_PTR1 != RX_PTR1 && uart1buffer[READ_PTR1] == 'K' && stato == 1){
			/// e' arrivato il secondo carattere della rispota
			stato = 2;
			READ_PTR1++;
			READ_PTR1 &= DIM_READ_BUFF - 1;

		}
		if (READ_PTR1 != RX_PTR1 && uart1buffer[READ_PTR1] == 0xD && stato == 2){
			/// finito il messaggio di risposta
			/// il modulo e' presente
			present = XBEE_PRESENT;
			NumUart = 1;
			READ_PTR1++;
			READ_PTR1 &= DIM_READ_BUFF - 1;
			/// legge l'indirizzo proprio del modulo
			//readMyAdd();
			/// deve inviare la chiusura dei messaggi: ATCN
			sendString ("ATCN", 4);
			sendChar(0x0D);
			/// e' nello stato ricezione trasmissione
			status = TXRX;

			return;
		}
	}
	present = XBEE_NOT_PRESENT;
}


///
/// invia un carattere tramite modulo radio
inline void xBee::sendChar(char c){
	UARTCharPut(UART1_BASE, c);
}

/// invia n caratteri tramite modulo radio
void xBee::sendString (char *s, int n){
	int i  = 0;

	while((*s) != 0 && i < n){
		sendChar(*s);
		s++;
	}
}


///
/// carica il vettore dell'indirizzo
void xBee::readReg(uint8_t *reg){
	/// l'indirizzo SH e' lungo 4 bytes ma viene restituito come
	/// 8 caratteri per ciascun semibyte
	uint8_t i = 0, val = 0;
	while(READ_PTR1 != RX_PTR1 && i < 8){
		uint8_t tmp = uart1buffer[READ_PTR1];
		if ((tmp - '0') > 9){
			val |= tmp - '0' + 9;
		}
		else
			val |= tmp - '0';

		if (i % 2 == 0){
			/// sposto il semibyte verso sinistra
			val <<= 4;
		}
		else{
			reg[i / 2] = val;
			val = 0;
		}
		i++;
		READ_PTR1 = upRptr(READ_PTR1);
	}
	/// adesso deve eliminare il carattere 'carriage return'
	READ_PTR1 = RX_PTR1;
}



///
/// legge l'indirizzo proprio del modulo
void xBee::readMyAdd(){

	/// puo' essere usato solo quando il modulo e' nello stato comando e non TXRX
	if (status == CMD){
		int endMess0;
		endMess0 = 0;
		sendString ("ATSH", 4);
		sendChar(0x0D);
		tick100 = 0;
		while (endMess0 == 0 && tick100 < 100);
		/// arrivata la fine messaggio
		if(endMess0){
			endMess0 = 0;
			readReg(m_ADD_H);
		}
		else{
			for(int i = 0; i < 4; i++)
				m_ADD_H[i] = 255;
		}

		/// legge la parte bassa dell'indirizzo
		endMess0 = 0;
		sendString ("ATSL", 4);
		sendChar(0x0D);
		tick100 = 0;
		while (endMess0 == 0 && tick100 < 100);
		/// arrivata la fine messaggio
		if(endMess0){
			endMess0 = 0;
			readReg(m_ADD_L);
		}
		else{
			for(int i = 0; i < 4; i++)
				m_ADD_L[i] = 255;
		}

	}
}

///
/// funzioncina statica che aggiorna una variabile globale
unsigned char xBee::upRptr(unsigned char p){

	p++;
	p &= DIM_READ_BUFF - 1;
	return p;

}

///
/// neorizza la velocita' di trasmissione
void xBee::setBaud(uint32_t valore){
	m_Baud = valore;
}

