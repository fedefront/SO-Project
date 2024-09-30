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
    static char rx_buffer[64];
    static uint8_t rx_index = 0;
    char received = UDR0;
    if (received == '\n' || received == '\r') {
        rx_buffer[rx_index] = '\0';
        process_command(rx_buffer);
        rx_index = 0;
    } else if (rx_index < sizeof(rx_buffer) - 1) {
        rx_buffer[rx_index++] = received;
    }
}

ISR(TIMER1_COMPA_vect) {
    //DA FARE...
}

int main(void) {
    UART_Init();
    ADC_Init();
    Timer_Init();
    sei();                    // Abilita le interruzioni globali

    selected_channels[0] = 0; // Canale A0 di default
    num_channels = 1;

    while (1) {
        // Il programma principale rimane in loop
    }

    return 0;
}

void UART_Init(void) {
    UBRR0H  = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L  = (uint8_t)(UBRR_VALUE & 0xFF);
    UCSR0B  = (1 << RXEN0) | (1 << TXEN0);   // Abilita RX e TX
    UCSR0B |= (1 << RXCIE0);                 // Abilita l'interruzione RX
    UCSR0C  = (1 << UCSZ01) | (1 << UCSZ00); // 8 bit di dati
}

void UART_Transmit(uint8_t data) {
    //DA FARE...
}

void UART_SendString(const char *str) {
    //DA FARE...
}

void ADC_Init(void) {
    ADMUX = (1 << REFS1) | (1 << REFS0); // Riferimento interno a 2,56V
    ADCSRA = (1 << ADEN);                // Abilita l'ADC
}

uint16_t ADC_Read(uint8_t channel) {
    if (channel > 7) return 0;                      // Canali validi: 0-7
    ADCSRB &= ~(1 << MUX5);                         // Cancella MUX5 per canali 0-7
    ADMUX = (ADMUX & 0xE0) | (channel & 0x07);      // Mantiene REFS1, REFS0, imposta MUX4:0
    ADCSRA |= (1 << ADSC);                          // Inizia la conversione
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void Timer_Init(void) {
    TCCR1B = 0; // Ferma il timer
    TCCR1A = 0;
    TCCR1C = 0;
    // Prescaler = 8
    uint32_t ocr1a_value = ((uint32_t)F_CPU / 8) * sampling_interval / 1000000UL - 1;
    OCR1A = (uint16_t)ocr1a_value;
    TCCR1B |= (1 << WGM12);  // Modalità CTC
    TCCR1B |= (1 << CS11);   // Prescaler 8
    TIMSK1 |= (1 << OCIE1A); // Abilita l'interruzione del timer
}

void process_command(char *command) {
    //DA FARE...
}