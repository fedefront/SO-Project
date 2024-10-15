#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define F_CPU 16000000UL  // Frequenza del clock della CPU
#define BAUD 38400        // Baud rate per la comunicazione seriale
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

#define MAX_CHANNELS 8
#define BUFFER_SIZE 128   // Numero di campioni per canale in modalità buffered

volatile uint8_t selected_channels[MAX_CHANNELS];
volatile uint8_t num_channels = 1;
volatile uint32_t sampling_interval = 1000;    // Intervallo di campionamento in microsecondi
volatile uint8_t mode = 0;                     // 0: continuous, 1: buffered
volatile uint8_t trigger_channel = 0;
volatile uint16_t trigger_level = 512;
volatile uint8_t trigger_edge = 0;             // 0: rising, 1: falling

volatile uint16_t buffer[BUFFER_SIZE * MAX_CHANNELS];
volatile uint16_t buffer_index = 0;
volatile uint8_t sampling = 0;
volatile uint8_t triggered = 0;
volatile uint16_t last_sample = 0;

volatile uint8_t continuos_sampling = 0;
volatile uint8_t triggered_sampling = 0;

void UART_Init(void);
void UART_Transmit(uint8_t data);
void UART_SendString(const char *str);

void continuos(void);
void buffered(void);
//funzione per il debug del codice..
void UART_Debug(const char *message) {
    UART_SendString(message);
    UART_Transmit('\n');
}

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
     if (sampling) {
        if (mode == 0) {                            // Modalità continua
            continuos_sampling = 1;            
        } else if (mode == 1) {                     // Modalità buffered con trigger
            triggered_sampling = 1;
        }
    }
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
        if (continuos_sampling == 1){
            continuos();
        } else if (triggered_sampling == 1){
            buffered();
        } else continue;
    }

    return 0;
}

void continuos(void){
    UART_Transmit(0xAA);         // Byte di sincronizzazione per il campione
    for (uint8_t i = 0; i < num_channels; i++) {
        uint16_t value = ADC_Read(selected_channels[i]);
        UART_Transmit((value >> 8) & 0xFF);
        UART_Transmit(value & 0xFF);
    }
    continuos_sampling = 0;
}

void buffered(void){
    if (!triggered) {
        uint16_t trigger_value = ADC_Read(trigger_channel);
        if ((trigger_edge == 0 && trigger_value >= trigger_level && last_sample < trigger_level) ||
        (trigger_edge == 1 && trigger_value <= trigger_level && last_sample > trigger_level)) {
            // Trigger rilevato
            triggered = 1;
            buffer_index = 0;
        }
    last_sample = trigger_value;
    } else {
                if (buffer_index < BUFFER_SIZE * num_channels) {
                    for (uint8_t i = 0; i < num_channels; i++) {
                        uint16_t value = ADC_Read(selected_channels[i]);
                        buffer[buffer_index++] = value;
                    }
                } else {
                    // Buffer pieno, invia i dati al PC
                    for (uint16_t i = 0; i < buffer_index; i += num_channels) {
                        UART_Transmit(0xAA); // Byte di sincronizzazione per il campione
                        for (uint8_t ch = 0; ch < num_channels; ch++) {
                            uint16_t value = buffer[i + ch];
                            UART_Transmit((value >> 8) & 0xFF);
                            UART_Transmit(value & 0xFF);
                        }
                    }
                    buffer_index = 0; // Reset per la prossima acquisizione
                    triggered = 0;    // Reset per cercare nuovamente il trigger
                }
            }
    triggered_sampling  = 0;
}

void UART_Init(void) {
    UBRR0H  = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L  = (uint8_t)(UBRR_VALUE & 0xFF);
    UCSR0B  = (1 << RXEN0) | (1 << TXEN0);   // Abilita RX e TX
    UCSR0B |= (1 << RXCIE0);                 // Abilita l'interruzione RX
    UCSR0C  = (1 << UCSZ01) | (1 << UCSZ00); // 8 bit di dati
}

void UART_Transmit(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_SendString(const char *str) {
     while (*str) {
        UART_Transmit(*str++);
    }
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
    if (strncmp(command, "START", 5) == 0) {
        sampling = 1;
        buffer_index = 0;
        triggered = 0;
        //UART_Debug("Sampling started");
    } else if (strncmp(command, "STOP", 4) == 0) {
        sampling = 0;
        //UART_Debug("Sampling stopped");
    } else if (strncmp(command, "SET_FREQ ", 9) == 0) {
        sampling_interval = atoi(&command[9]);
        Timer_Init();
        //UART_Debug("Frequency set");
    } else if (strncmp(command, "SET_CHANNELS ", 13) == 0) {
       char *token = strtok(&command[13], ",");
        num_channels = 0;
        while (token != NULL && num_channels < MAX_CHANNELS) {
            int ch = atoi(token);
            if (ch >= 0 && ch <= 7) {
                selected_channels[num_channels++] = ch;
            }
            token = strtok(NULL, ",");
        }
        //UART_Debug("Channels set");
    } else if (strncmp(command, "SET_MODE ", 9) == 0) {
        if (command[9] == '0') {
            mode = 0;
        } else if (command[9] == '1') {
            mode = 1;
        }
         //UART_Debug("Mode set");
    } else if (strncmp(command, "SET_TRIGGER ", 12) == 0) {
        char *params = &command[12];
        char *token = strtok(params, " ");
        if (token != NULL) {
            trigger_channel = atoi(token);
            token = strtok(NULL, " ");
            if (token != NULL) {
                trigger_level = atoi(token);
                token = strtok(NULL, " ");
                if (token != NULL) {
                    trigger_edge = atoi(token); // 0 per RISING, 1 per FALLING
                }
            }
        }
        //UART_Debug("Trigger set");
    } 
      //else {
        //UART_Debug("Unknown command");
           //}
}