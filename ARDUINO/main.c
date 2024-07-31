#include <avr/io.h>
#include <avr/interrupt.h>

//definizione dei pin di arduino usati

#define ENCODER_PIN_A  PD2   // È il pin 2 su arduino 2560
#define ENCODER_PIN_B  PD3   // È il pin 3 su arduino 2560
#define MOTOR_PIN_1    PD4   // È il pin 4 su arduino 2560 per l'H-bridge
#define MOTOR_PIN_2    PD5   // È il pin 5 su arduino 2560 per l'H-bridge
#define PWM_PIN        PH3   // E il pin 6 per la PWM (OC4A)

//creo una variabile globale per l'encoder

volatile int encoder_count = 0;

// funzioni per inizializzare motore ed encoder

void encoder_init();
void motor_init();

int main(void){
    //setup delle periferiche
    encoder_init();
    motor_init();

    //abilito gli interrupts globali
    sei();

    //loop principale del programma
    while(1){
        //DA FARE
    }
}

void encoder_init(){
    //configurazione dei pin dell'encoder come input
    DDRD &= ~((1 << ENCODER_PIN_A) | (1 << ENCODER_PIN_B));

    //abilito il pull up
    PORTD |= (1 << ENCODER_PIN_A) | (1 << ENCODER_PIN_B);

    //configuro gli interrupt esterni
    EICRA |= (1 << ISC00) | (1 << ISC10); //interrupt su ogni cambio di stato
    EIMSK |= (1 << INT 0) | (1 << INT1); //abilito INT0 e INT1
}

void motor_init(){
    //configuro in pin del motore come output
    DDRD |= (1 << MOTOR_PIN_1) | (1 << MOTOR_PIN_2);

    //configuro il pin PWM come output
    DDRH |= (1 << PWM_PIN);

    //setuppo il timer per la PWM
    //prescaler=64
    TCCR4A = (1 << COM4A1) | (1 << WGM40) | (1 << WGM41);
    TCCR4B = (1 << CS41) | (CS40);
}

//Interrupt Service Routine per l'encoder
ISR(INT0_vect){

    //incremento/decremento il conto in base alla direzione
    if (PIND & (1 << ENCODER_PIN_B)){
        encoder_count++;
    } else {
        encoder_count--;
    }
    
}

ISR(INT1_vect){
    
    //incremento/decremento il conto in base alla direzione
    if (PIND & (1 << ENCODER_PIN_A)){
        encoder_count++;
    } else {
        encoder_count--;
        }
}