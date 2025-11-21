#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

#define WIDTH 1000
#define HEIGHT 800
#define TILE 100
#define NUM_OBS 12

typedef struct { int x, y; } Posicion;

typedef struct {
    Posicion jugador, moneda, obstaculos[NUM_OBS];
    int vidas, puntaje;
} Juego;

void inicializar(Juego *j) {
    srand(time(NULL));
    j->jugador.x = WIDTH / (2 * TILE);
    j->jugador.y = HEIGHT / (2 * TILE);
    j->moneda.x = rand() % (WIDTH / TILE);
    j->moneda.y = rand() % (HEIGHT / TILE);
    j->vidas = 3;
    j->puntaje = 0;

    // No superposición, arreglado
    for (int i = 0; i < NUM_OBS; i++) {
        int valido;
        do {
            valido = 1;
            j->obstaculos[i].x = rand() % (WIDTH / TILE);
            j->obstaculos[i].y = rand() % (HEIGHT / TILE);

            // No sobre jugador
            if (colision(j->obstaculos[i], j->jugador))
                valido = 0;

            // No sobre moneda
            if (colision(j->obstaculos[i], j->moneda))
                valido = 0;

            // No sobre otros obstáculos
            for (int k = 0; k < i; k++) {
                if (colision(j->obstaculos[i], j->obstaculos[k])) {
                    valido = 0;
                    break;
                }
            }
        } while (!valido);
    }
}


// Funcion para mover obst. aleatoriamente
void mover_obstaculos(Juego *j) {
    for (int i = 0; i < NUM_OBS; i++) {
        int valido;
        do {
            valido = 1;
            j->obstaculos[i].x = rand() % (WIDTH / TILE);
            j->obstaculos[i].y = rand() % (HEIGHT / TILE);

            // No sobre jugador
            if (colision(j->obstaculos[i], j->jugador))
                valido = 0;

            // No sobre moneda
            if (colision(j->obstaculos[i], j->moneda))
                valido = 0;

            // No sobre otros obstáculos
            for (int k = 0; k < i; k++) {
                if (colision(j->obstaculos[i], j->obstaculos[k])) {
                    valido = 0;
                    break;
                }
            }
        } while (!valido);
    }
}

void mover(Juego *j, SDL_Keycode key) {
    switch (key) {
        case SDLK_w: if (j->jugador.y > 0) j->jugador.y--; break;
        case SDLK_s: if (j->jugador.y < HEIGHT / TILE - 1) j->jugador.y++; break;
        case SDLK_a: if (j->jugador.x > 0) j->jugador.x--; break;
        case SDLK_d: if (j->jugador.x < WIDTH / TILE - 1) j->jugador.x++; break;
    }
}

int colision(Posicion a, Posicion b) { return a.x == b.x && a.y == b.y; }

void verificar(Juego *j, int colisionActiva) {
    // Jugador atrapa moneda
    if (colisionActiva && colision(j->jugador, j->moneda)) {
        j->puntaje++;

        int valido;
        do {
            valido = 1;
            j->moneda.x = rand() % (WIDTH / TILE);
            j->moneda.y = rand() % (HEIGHT / TILE);

            // No sobre jugador
            if (colision(j->moneda, j->jugador))
                valido = 0;

            // No sobre obstáculos
            for (int i = 0; i < NUM_OBS; i++) {
                if (colision(j->moneda, j->obstaculos[i])) {
                    valido = 0;
                    break;
                }
            }
        } while (!valido);

        // Mover obstaculos al atrapar una moneda
        mover_obstaculos(j);
    }

    // Jugador choca con obstculo
    if (colisionActiva) {
        for (int i = 0; i < NUM_OBS; i++) {
            if (colision(j->jugador, j->obstaculos[i])) {
                j->vidas--;
                j->jugador.x = WIDTH / (2 * TILE);
                j->jugador.y = HEIGHT / (2 * TILE);
                break;
            }
        }
    }
}


void render_text(SDL_Renderer *ren, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(ren, texture, NULL, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


SDL_Texture* cargar_textura(SDL_Renderer *ren, const char *archivo) {
    SDL_Surface *img = IMG_Load(archivo);
    if (!img) {
        printf("Error cargando imagen %s: %s\n", archivo, IMG_GetError());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, img);
    SDL_FreeSurface(img);
    return tex;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error inicializando SDL: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        printf("Error inicializando SDL_ttf: %s\n", TTF_GetError());
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("Error inicializando SDL_image: %s\n", IMG_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Atrapa la Moneda",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    // Fuente
    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);
    if (!font) {
        printf("Error cargando fuente: %s\n", TTF_GetError());
        return 1;
    }

    // Cargar imágenes
    SDL_Texture *texJugador = cargar_textura(ren, "ddragon.png");
    SDL_Texture *texMoneda = cargar_textura(ren, "moneda.png");
    SDL_Texture *texObstaculo = cargar_textura(ren, "espadaa.png");
    SDL_Texture *texFondo = cargar_textura(ren, "fonndo.png");

    Juego j;
    inicializar(&j);

    int running = 1;
    SDL_Event e;
    Uint32 lastTime = 0;

    while (running && j.vidas > 0) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_x) running = 0;
                else mover(&j, e.key.keysym.sym);
            }
        }

        verificar(&j, 1);

        // Fondo
        SDL_SetRenderDrawColor(ren, 30, 30, 30, 255);
        SDL_RenderClear(ren);
        if (texFondo)
            SDL_RenderCopy(ren, texFondo, NULL, NULL);

        // Moneda
        SDL_Rect coin = {j.moneda.x * TILE, j.moneda.y * TILE, TILE, TILE};
        SDL_RenderCopy(ren, texMoneda, NULL, &coin);

        // Obstáculos
        for (int i = 0; i < NUM_OBS; i++) {
            SDL_Rect r = {j.obstaculos[i].x * TILE, j.obstaculos[i].y * TILE, TILE, TILE};
            SDL_RenderCopy(ren, texObstaculo, NULL, &r);
        }

        // Jugador
        SDL_Rect p = {j.jugador.x * TILE, j.jugador.y * TILE, TILE, TILE};
        SDL_RenderCopy(ren, texJugador, NULL, &p);

        // Vidas y puntaje
        char texto[64];
        sprintf(texto, "Vidas: %d   Puntaje: %d", j.vidas, j.puntaje);
        SDL_Color white = {255, 255, 255, 255};
        render_text(ren, font, texto, 10, 10, white);

        SDL_RenderPresent(ren);

        if (SDL_GetTicks() - lastTime < 16)
            SDL_Delay(16 - (SDL_GetTicks() - lastTime));
        lastTime = SDL_GetTicks();
    }

        // Vidas y puntaje
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    SDL_Color red = {255, 50, 50, 255};
    SDL_Color white = {255, 255, 255, 255};
    render_text(ren, font, "GAME OVER", WIDTH / 2 - 80, HEIGHT / 2 - 40, red);
    char final[64];
    sprintf(final, "Puntaje final: %d", j.puntaje);
    render_text(ren, font, final, WIDTH / 2 - 90, HEIGHT / 2 + 10, white);
    render_text(ren, font, "Presiona cualquier tecla para salir", WIDTH / 2 - 190, HEIGHT / 2 + 60, white);
    SDL_RenderPresent(ren);

    int esperar = 1;
    while (esperar) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN)
                esperar = 0;
        }
        SDL_Delay(10);
    }

    // Limpieza
    SDL_DestroyTexture(texJugador);
    SDL_DestroyTexture(texMoneda);
    SDL_DestroyTexture(texObstaculo);
    if (texFondo) SDL_DestroyTexture(texFondo);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    printf("\nJuego terminado. Puntaje final: %d\n", j.puntaje);
    return 0;
}
