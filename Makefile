# Nome eseguibile
TARGET = spmv

# Compilatore
CC = gcc

# Flag di compilazione generali
CFLAGS = -Wall -O2

# File oggetto da costruire
OBJS = main.o CSR_Matrix.o verify.o mmio.o

# Compilazione target principale
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Regola generale per i file .c (escluso mmio.c)
%.o: %.c
	@if [ "$<" != "mmio.c" ]; then \
		$(CC) $(CFLAGS) -c -o $@ $<; \
	fi

# Regola specifica per mmio.c (disattiva i warning)
mmio.o: mmio.c
	$(CC) -w -O2 -c -o mmio.o mmio.c

# Pulizia
clean:
	rm -f $(OBJS) $(TARGET)
