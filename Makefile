# Nombre del ejecutable
TARGET = juego
# Archivo
SRC = juego_v1.c
# Flags de compilación
CFLAGS = -Wall -Wextra -O2
# Librerías 
SDL_FLAGS = $(shell sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_image

all:
	gcc $(SRC) -o $(TARGET) $(CFLAGS) $(SDL_FLAGS)

clean:
	rm -f $(TARGET)

