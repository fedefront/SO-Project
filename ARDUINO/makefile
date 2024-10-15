# Nome del file principale (senza estensione) per il microcontrollore
MCU_TARGET = oscilloscope

# Nome del file principale (senza estensione) per il client PC
PC_TARGET = pc_client

# Microcontrollore
MCU = atmega2560

# Frequenza della CPU
F_CPU = 16000000UL

# Porta seriale per il caricamento (sostituisci con la tua)
PORT = /dev/ttyACM0

# Programmatore e velocità di baud
PROGRAMMER = wiring
BAUDRATE = 115200

# File sorgente per il microcontrollore
MCU_SRC = $(MCU_TARGET).c

# File sorgente per il client PC
PC_SRC = $(PC_TARGET).c

# File oggetto per il microcontrollore
MCU_OBJ = $(MCU_SRC:.c=.o)

# File oggetto per il client PC
PC_OBJ = $(PC_SRC:.c=.o)

# Flags del compilatore per il microcontrollore
MCU_CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall

# Flags del compilatore per il client PC
PC_CFLAGS = -Wall -O2

# Flags di linker per il microcontrollore
MCU_LDFLAGS = -mmcu=$(MCU)

# Comandi
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# Compilatore per il client PC
PC_CC = gcc

# Regola predefinita
all: $(MCU_TARGET).hex $(PC_TARGET)

# Compilazione del file oggetto per il microcontrollore
%.o: %.c
	$(CC) $(MCU_CFLAGS) -c -o $@ $<

# Creazione del file ELF per il microcontrollore
$(MCU_TARGET).elf: $(MCU_OBJ)
	$(CC) $(MCU_LDFLAGS) -o $@ $^

# Creazione del file HEX per il microcontrollore
$(MCU_TARGET).hex: $(MCU_TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Compilazione del client PC
$(PC_TARGET): $(PC_SRC)
	$(PC_CC) $(PC_CFLAGS) -o $@ $<

# Caricamento sulla scheda
upload: $(MCU_TARGET).hex
	$(AVRDUDE) -v -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUDRATE) -D -U flash:w:$(MCU_TARGET).hex:i

# Pulizia dei file generati
clean:
	rm -f *.o *.elf *.hex voltage.txt $(PC_TARGET) 
plot:
	gnuplot plot_data.gp 
# Phony targets
.PHONY: all upload cleanp