#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct { int x; int y; } Posicion;

typedef struct {
    Posicion jugador;
    Posicion moneda;
    Posicion obstaculos[5];
    int vidas;
    int puntaje;
} Juego;

typedef enum { ARRIBA, ABAJO, IZQUIERDA, DERECHA, SALIR } Direccion;

char getch() {
    char buf = 0;
    struct termios old = {0};
    tcgetattr(0, &old);
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &old);
    read(0, &buf, 1);
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    tcsetattr(0, TCSADRAIN, &old);
    return buf;
}

void inicializarJuego(Juego *j, int ancho, int alto) {
    srand(time(NULL));
    j->jugador.x = ancho / 2;
    j->jugador.y = alto / 2;
    j->moneda.x = rand() % ancho;
    j->moneda.y = rand() % alto;
    j->puntaje = 0;
    j->vidas = 3;
    for (int i = 0; i < 5; i++) {
        j->obstaculos[i].x = rand() % ancho;
        j->obstaculos[i].y = rand() % alto;
    }
}

void mostrarTablero(Juego *j, int ancho, int alto) {
    system("clear");
    for (int y = 0; y < alto; y++) {
        for (int x = 0; x < ancho; x++) {
            if (x == j->jugador.x && y == j->jugador.y)
                printf("☺");
            else if (x == j->moneda.x && y == j->moneda.y)
                printf("$");
            else {
                int esObstaculo = 0;
                for (int k = 0; k < 5; k++) {
                    if (x == j->obstaculos[k].x && y == j->obstaculos[k].y) {
                        printf("#");
                        esObstaculo = 1;
                        break;
                    }
                }
                if (!esObstaculo) printf(".");
            }
        }
        printf("\n");
    }
    printf("\nPuntaje: %d  |  Vidas: %d\n", j->puntaje, j->vidas);
    printf("Mover: w/a/s/d | Salir: x\n");
}

Direccion leerEntradaUsuario() {
    char input = getch();
    switch (input) {
        case 'w': return ARRIBA;
        case 's': return ABAJO;
        case 'a': return IZQUIERDA;
        case 'd': return DERECHA;
        case 'x': return SALIR;
        default:  return -1;
    }
}

void moverJugador(Juego *j, Direccion dir, int ancho, int alto) {
    switch (dir) {
        case ARRIBA:    if (j->jugador.y > 0) j->jugador.y--; break;
        case ABAJO:     if (j->jugador.y < alto-1) j->jugador.y++; break;
        case IZQUIERDA: if (j->jugador.x > 0) j->jugador.x--; break;
        case DERECHA:   if (j->jugador.x < ancho-1) j->jugador.x++; break;
        default: break;
    }
}

void verificarColision(Juego *j, int ancho, int alto) {
    // Moneda
    if (j->jugador.x == j->moneda.x && j->jugador.y == j->moneda.y) {
        j->puntaje++;
        j->moneda.x = rand() % ancho;
        j->moneda.y = rand() % alto;
    }

    // Obstáculos
    for (int i = 0; i < 5; i++) {
        if (j->jugador.x == j->obstaculos[i].x && j->jugador.y == j->obstaculos[i].y) {
            j->vidas--;
            printf("\n¡Cuidado! Has perdido una vida. Vidas restantes: %d\n", j->vidas);
            j->jugador.x = ancho / 2;
            j->jugador.y = alto / 2;
            if (j->vidas <= 0) {
                printf("\nHas perdido todas tus vidas. Fin del juego.\n");
                exit(0);
            }
        }
    }
}

int main() {
    const int ancho = 20, alto = 10;
    Juego juego;
    Direccion dir;
    inicializarJuego(&juego, ancho, alto);

    while (1) {
        mostrarTablero(&juego, ancho, alto);
        dir = leerEntradaUsuario();
        if (dir == SALIR) break;
        if (dir != -1) {
            moverJugador(&juego, dir, ancho, alto);
            verificarColision(&juego, ancho, alto);
        }
    }

    printf("\nJuego terminado. Puntaje final: %d\n", juego.puntaje);
    return 0;
}
