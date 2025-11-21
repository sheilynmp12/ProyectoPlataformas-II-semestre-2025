#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 400
#define TILE 40
#define NUM_OBS 5

typedef struct { int x, y; } Posicion;

typedef struct {
    Posicion jugador, moneda, obstaculos[NUM_OBS];
    int vidas, puntaje;
} Juego;



//
void inicializar(Juego *j) {
    srand(time(NULL));
    j->jugador.x = WIDTH / (2 * TILE);
    j->jugador.y = HEIGHT / (2 * TILE);
    j->moneda.x = rand() % (WIDTH / TILE);
    j->moneda.y = rand() % (HEIGHT / TILE);
    j->vidas = 3;
    j->puntaje = 0;

    for (int i = 0; i < NUM_OBS; i++) {
        do {
            j->obstaculos[i].x = rand() % (WIDTH / TILE);
            j->obstaculos[i].y = rand() % (HEIGHT / TILE);
        } while (j->obstaculos[i].x == j->jugador.x && j->obstaculos[i].y == j->jugador.y);
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




// cierre de cambios //

int colision(Posicion a, Posicion b) { return a.x == b.x && a.y == b.y; }

void verificar(Juego *j) {
    if (colision(j->jugador, j->moneda)) {
        j->puntaje++;
        j->moneda.x = rand() % (WIDTH / TILE);
        j->moneda.y = rand() % (HEIGHT / TILE);
    }
    for (int i = 0; i < NUM_OBS; i++) {
        if (colision(j->jugador, j->obstaculos[i])) {
            j->vidas--;
            j->jugador.x = WIDTH / (2 * TILE);
            j->jugador.y = HEIGHT / (2 * TILE);
            break;
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

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *win = SDL_CreateWindow("Atrapa la Moneda",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    // Font
    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);
    if (!font) {
        printf("Error cargando fuente: %s\n", TTF_GetError());
        return 1;
    }

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

        verificar(&j);

        // Renderizado
        SDL_SetRenderDrawColor(ren, 20, 20, 20, 255);
        SDL_RenderClear(ren);

        // Moneda
        SDL_Rect coin = {j.moneda.x * TILE, j.moneda.y * TILE, TILE, TILE};
        SDL_SetRenderDrawColor(ren, 255, 215, 0, 255);
        SDL_RenderFillRect(ren, &coin);

        // Obstáculos
        SDL_SetRenderDrawColor(ren, 180, 0, 0, 255);
        for (int i = 0; i < NUM_OBS; i++) {
            SDL_Rect r = {j.obstaculos[i].x * TILE, j.obstaculos[i].y * TILE, TILE, TILE};
            SDL_RenderFillRect(ren, &r);
        }

        // Jugador
        SDL_SetRenderDrawColor(ren, 0, 200, 0, 255);
        SDL_Rect p = {j.jugador.x * TILE, j.jugador.y * TILE, TILE, TILE};
        SDL_RenderFillRect(ren, &p);

        // Vidas y puntaje
        char texto[64];
        sprintf(texto, "Vidas: %d   Puntaje: %d", j.vidas, j.puntaje);
        SDL_Color white = {255, 255, 255, 255};
        render_text(ren, font, texto, 10, 10, white);

        SDL_RenderPresent(ren);

        // Control de FPS (~60)
        Uint32 now = SDL_GetTicks();
        if (now - lastTime < 16)
            SDL_Delay(16 - (now - lastTime));
        lastTime = now;
    }

    // Game Over
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

    // Cierre
    int esperar = 1;
    while (esperar) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN)
                esperar = 0;
        }
        SDL_Delay(10);
    }

    // Limpieza
    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();

    printf("\nJuego terminado. Puntaje final: %d\n", j.puntaje);
    return 0;
}
