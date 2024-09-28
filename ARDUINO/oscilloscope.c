#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define F_CPU 16000000UL  // Frequenza del clock della CPU
#define BAUD 38400      // Baud rate per la comunicazione seriale


#define MAX_CHANNELS 8
#define BUFFER_SIZE 128   // Numero di campioni per canale in modalità buffered

void UART_Init(void);
void UART_Transmit(uint8_t data);
void UART_SendString(const char *str);

void ADC_Init(void);
uint16_t ADC_Read(uint8_t channel);

void Timer_Init(void);
void process_command(char *command);


ISR(USART0_RX_vect) {
    //DA FARE...
}

ISR(TIMER1_COMPA_vect) {
    //DA FARE...
}

int main(void) {
    UART_Init();
    ADC_Init();
    Timer_Init();
    sei(); // Abilita le interruzioni globali

    selected_channels[0] = 0; // Canale A0 di default
    num_channels = 1;

    while (1) {
        // Il programma principale rimane in loop
    }

    return 0;
}

void UART_Init(void) {
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)(UBRR_VALUE & 0xFF);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Abilita RX e TX
    UCSR0B |= (1 << RXCIE0); // Abilita l'interruzione RX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bit di dati
}

void UART_Transmit(uint8_t data) {
    //DA FARE...
}

void UART_SendString(const char *str) {
    //DA FARE...
}

void ADC_Init(void) {
    //DA FARE...
}

uint16_t ADC_Read(uint8_t channel) {
    //DA FARE...
}

void Timer_Init(void) {
    //DA FARE...
}

void process_command(char *command) {
    //DA FARE...
}