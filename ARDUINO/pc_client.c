#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 1024
#define BUFFER_SIZE 128 // come quello definito in oscilloscope.c

int open_serial_port(const char *device);
void configure_serial_port(int fd);
void send_command(int fd, const char *command);
void receive_data(int fd, const char *filename, int num_channels, int mode);

int main(int argc, char *argv[]) {
    char device[64] = "/dev/ttyACM0"; // Modificare con la porta seriale giusta
    int fd = open_serial_port(device);
    if (fd == -1) {
        perror("Errore nell'apertura della porta seriale");
        return -1;
    }
    configure_serial_port(fd);

    // Variabili per i parametri
    int sampling_frequency;
    int mode;
    int num_channels;
    char channels_list[64];
    char command[128];

    // Chiedo all'utente la frequenza di campionamento
    printf("Inserisci la frequenza di campionamento in Hz (es. 1000): ");
    if (scanf("%d", &sampling_frequency) != 1) {
        fprintf(stderr, "Errore nella lettura della frequenza di campionamento.\n");
        return -1;
    }
    // Converto in intervallo di campionamento in microsecondi
    int sampling_interval_us = 1000000 / sampling_frequency;

    // Chiedo all'utente la modalità (0 o 1)
    printf("Scegli la modalità (0: CONTINUOUS, 1: BUFFERED): ");
    if (scanf("%d", &mode) != 1 || (mode != 0 && mode != 1)) {
        fprintf(stderr, "Modalità non valida. Inserisci 0 per CONTINUOUS o 1 per BUFFERED.\n");
        return -1;
    }

    // Chiedo all'utente il numero di canali
    printf("Inserisci il numero di canali da usare (1-16): ");
    if (scanf("%d", &num_channels) != 1 || num_channels < 1 || num_channels > 16) {
        fprintf(stderr, "Numero di canali non valido. Deve essere tra 1 e 16.\n");
        return -1;
    }

    // Chiedo all'utente quali canali usare
    printf("Inserisci i numeri dei canali separati da virgola (es. 0,1,2...8): ");
    if (scanf("%63s", channels_list) != 1) {
        fprintf(stderr, "Errore nella lettura dei canali.\n");
        return -1;
    }

// Invio i comandi ad Arduino
    snprintf(command, sizeof(command), "SET_FREQ %d", sampling_interval_us);
    send_command(fd, command);

    snprintf(command, sizeof(command), "SET_CHANNELS %s", channels_list);
    send_command(fd, command);

    snprintf(command, sizeof(command), "SET_MODE %d", mode);
    send_command(fd, command);

    if (mode == 1) {
        // Chiedo informazioni sul trigger
        int trigger_channel, trigger_level, trigger_edge;
        printf("Inserisci il canale per il trigger (0-7): ");
        if (scanf("%d", &trigger_channel) != 1 || trigger_channel < 0 || trigger_channel > 7) {
            fprintf(stderr, "Errore nella lettura del canale di trigger.\n");
            return -1;
        }
        printf("Inserisci il livello di trigger (0-1023): ");
        if (scanf("%d", &trigger_level) != 1 || trigger_level < 0 || trigger_level > 1023) {
            fprintf(stderr, "Errore nella lettura del livello di trigger.\n");
            return -1;
        }
        printf("Inserisci il tipo di fronte (0: RISING, 1: FALLING): ");
        if (scanf("%d", &trigger_edge) != 1 || (trigger_edge != 0 && trigger_edge != 1)) {
            fprintf(stderr, "Errore nella lettura del tipo di fronte.\n");
            return -1;
        }

        snprintf(command, sizeof(command), "SET_TRIGGER %d %d %d", trigger_channel, trigger_level, trigger_edge);
        send_command(fd, command);
    }

    // Inizio il campionamento
    send_command(fd, "START");

    // Ricevo i dati per 10 secondi
    receive_data(fd, "voltage.txt", num_channels, mode);

    // Fermo il campionamento
    send_command(fd, "STOP");

    close(fd);
    printf("Campionamento terminato. Dati salvati in 'voltage.txt'.\n");
    return 0;
}

int open_serial_port(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        return -1;
    }
    fcntl(fd, F_SETFL, 0);
    return fd;
}

void configure_serial_port(int fd) {
     struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B38400);
    cfsetospeed(&options, B38400);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;   // Maschera per i bit di dati 
    options.c_cflag |= CS8;      // 8 bit di dati 
    options.c_cflag &= ~PARENB;  // Nessuna parità 
    options.c_cflag &= ~CSTOPB;  // 1 bit di stop 
    options.c_cflag &= ~CRTSCTS; // Nessun controllo di flusso hardware 
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disabilita il controllo di flusso software 
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
    options.c_oflag &= ~OPOST;   
    tcsetattr(fd, TCSANOW, &options);
    tcflush(fd, TCIOFLUSH);      // Pulisci i buffer 
}

void send_command(int fd, const char *command) {
   char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s\n", command);
    ssize_t bytes_written = write(fd, buffer, strlen(buffer));
    if (bytes_written < 0) {
        perror("Errore nella scrittura sulla porta seriale");
        exit(EXIT_FAILURE);
    } else if ((size_t)bytes_written != strlen(buffer)) {
        fprintf(stderr, "Non tutti i bytes sono stati scritti sulla porta seriale.\n");
    }
}

void receive_data(int fd, const char *filename, int num_channels, int mode) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Errore nell'apertura del file");
        return;
    }
    //DA fare...
    fclose(file);
}