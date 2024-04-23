#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GRID_SIZE 3
#define FONT_SIZE 32

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

const char *PLAYER1_SYMBOL = "X";
const char *PLAYER2_SYMBOL = "O";

int player1_score = 0;
int player2_score = 0;
int moves = 0;
int board[GRID_SIZE][GRID_SIZE] = {0}; // Initialize the board as empty

int current_player = 1; // 1 for X, -1 for O
char font_path[1024];

typedef struct {
    SDL_Rect rect;
    SDL_Color color;
} Container;

Container game_container;

void initialize_game();
void cleanup();
void handle_events(int *running);
void update_screen();
void handle_click();
void reset_board();
int check_win();
void update_score(int *score);
void render_container(SDL_Renderer *renderer, const Container *container);
void draw_board(const Container *container);
void draw_x(SDL_Renderer *renderer, int x, int y, int size);
void draw_circle(SDL_Renderer *renderer, int x, int y, int radius);
void render_player_turn(SDL_Renderer *renderer, const char *player_symbol);
void show_score(SDL_Renderer *renderer, const char *player_symbol, int player_score, int x, int y);

int main() {
    initialize_game();

    // Initialize the board state to empty
    reset_board();

    SDL_Event event;
    int running = 1;
		char cwd[1024];

    // Get the current working directory and set the font path
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Failed to get current working directory");
        cleanup();
        return EXIT_FAILURE;
    }
    snprintf(font_path, sizeof(font_path), "%s/font/NotoSansMono-Regular.ttf", cwd);

    while (running) {
        handle_events(&running);
        update_screen();
    }

    cleanup();
    return EXIT_SUCCESS;
}

void initialize_game() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("Tic-Tac-Toe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        cleanup();
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        cleanup();
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Initialize the container
    game_container.rect.x = (WINDOW_WIDTH - WINDOW_HEIGHT * 0.7) / 2;
    game_container.rect.y = (WINDOW_HEIGHT - WINDOW_HEIGHT * 0.7) / 2;
    game_container.rect.w = WINDOW_HEIGHT * 0.7;
    game_container.rect.h = WINDOW_HEIGHT * 0.7;
    game_container.color = (SDL_Color){255, 255, 255, 255}; // White color for the background
}

void cleanup() {
    TTF_Quit();
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void handle_events(int *running) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            *running = 0;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            handle_click();
            // Check win condition after each move
            int winner = check_win();
            if (winner == 1) {
                // Player 1 (X) wins
                update_score(&player1_score);
                reset_board(); // Reset the board after a win
            } else if (winner == -1) {
                // Player 2 (O) wins
                update_score(&player2_score);
                reset_board(); // Reset the board after a win
            } else if (moves == GRID_SIZE * GRID_SIZE) {
                // Draw condition
                reset_board();
                moves = 0;
            }
        }
    }
}

void update_screen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    render_container(renderer, &game_container);
    show_score(renderer, PLAYER1_SYMBOL, player1_score, 0, WINDOW_HEIGHT / 2);
    show_score(renderer, PLAYER2_SYMBOL, player2_score, WINDOW_WIDTH - 100, WINDOW_HEIGHT / 2);
    draw_board(&game_container);

    // Draw Xs and Os based on the board state
    int cell_width = game_container.rect.w / GRID_SIZE;
    int cell_height = game_container.rect.h / GRID_SIZE;

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            int x = game_container.rect.x + j * cell_width;
            int y = game_container.rect.y + i * cell_height;
            int center_x = x + cell_width / 2;
            int center_y = y + cell_height / 2;

            if (board[i][j] == 1) {
                // Draw X
                draw_x(renderer, x, y, cell_width / 2);
            } else if (board[i][j] == -1) {
                // Draw O
                draw_circle(renderer, center_x, center_y, (cell_width * 0.5) / 2);
            }
        }
    }

    const char *current_player_symbol = (current_player == 1) ? PLAYER1_SYMBOL : PLAYER2_SYMBOL;
    render_player_turn(renderer, current_player_symbol);

    SDL_RenderPresent(renderer);
}

void handle_click() {
    // Get mouse click coordinates
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    // Calculate which cell was clicked
    int cell_width = game_container.rect.w / GRID_SIZE;
    int cell_height = game_container.rect.h / GRID_SIZE;

    int col = (mouse_x - game_container.rect.x) / cell_width;
    int row = (mouse_y - game_container.rect.y) / cell_height;

    // Ensure the click was within the container bounds and the cell is empty
    if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE && board[row][col] == 0) {
        // Place the current player's mark in the clicked cell
        board[row][col] = current_player;
        // Switch to the other player
        current_player = -current_player;
        // Increment moves counter
        moves++;
    }
}

void reset_board() {
    // Reset the board state to empty and reset moves counter
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            board[i][j] = 0;
        }
    }
    moves = 0;
}

int check_win() {
    // Check rows and columns for a win
    for (int i = 0; i < GRID_SIZE; i++) {
        if (board[i][0] != 0 && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            return board[i][0];
        }
        if (board[0][i] != 0 && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            return board[0][i];
        }
    }

    // Check diagonals for a win
    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        return board[0][0];
    }
    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        return board[0][2];
    }

    // No win condition met
    return 0;
}

void update_score(int *score) {
    // Increment the given player's score
    (*score)++;
}

void render_container(SDL_Renderer *renderer, const Container *container) {
    SDL_SetRenderDrawColor(renderer, container->color.r, container->color.g, container->color.b, container->color.a);
    SDL_RenderFillRect(renderer, &container->rect);
}

void draw_board(const Container *container) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color for grid lines

    // Draw vertical and horizontal grid lines
    for (int i = 1; i < GRID_SIZE; i++) {
        int x = container->rect.w * i / GRID_SIZE + container->rect.x;
        SDL_RenderDrawLine(renderer, x, container->rect.y, x, container->rect.y + container->rect.h);
        int y = container->rect.h * i / GRID_SIZE + container->rect.y;
        SDL_RenderDrawLine(renderer, container->rect.x, y, container->rect.x + container->rect.w, y);
    }
}

void draw_x(SDL_Renderer *renderer, int x, int y, int size) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color for X

    // Calculate the diagonal lines for the X
    int start_x1 = x;
    int start_y1 = y;
    int end_x1 = x + size;
    int end_y1 = y + size;

    int start_x2 = x;
    int start_y2 = y + size;
    int end_x2 = x + size;
    int end_y2 = y;

    // Draw the diagonals for the X
    SDL_RenderDrawLine(renderer, start_x1, start_y1, end_x1, end_y1);
    SDL_RenderDrawLine(renderer, start_x2, start_y2, end_x2, end_y2);
}

void draw_circle(SDL_Renderer *renderer, int x, int y, int radius) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color for O

    // Draw a circle using the midpoint circle algorithm
    int offsetx = 0;
    int offsety = radius;
    int decision = 3 - 2 * radius;

    while (offsety >= offsetx) {
        SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
        SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
        SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
        SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
        SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
        SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
        SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
        SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

        if (decision < 0) {
            decision += 4 * offsetx + 6;
        } else {
            decision += 4 * (offsetx - offsety) + 10;
            offsety--;
        }
        offsetx++;
    }
}

void render_player_turn(SDL_Renderer *renderer, const char *player_symbol) {
    TTF_Font* font = TTF_OpenFont(font_path, FONT_SIZE);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color white = {255, 255, 255, 255}; // White color for text

    SDL_Surface* surface_message = TTF_RenderText_Blended(font, player_symbol, white);
    if (!surface_message) {
        fprintf(stderr, "Failed to render text: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* texture_message = SDL_CreateTextureFromSurface(renderer, surface_message);
    if (!texture_message) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface_message);
        TTF_CloseFont(font);
        return;
    }

    SDL_Rect message_rect = {
        WINDOW_WIDTH / 2 - surface_message->w / 2,
        0,
        surface_message->w,
        surface_message->h
    };

    SDL_RenderCopy(renderer, texture_message, NULL, &message_rect);

    SDL_FreeSurface(surface_message);
    SDL_DestroyTexture(texture_message);
    TTF_CloseFont(font);
}

void show_score(SDL_Renderer *renderer, const char *player_symbol, int player_score, int x, int y) {
    char score_text[50];
    snprintf(score_text, sizeof(score_text), "%s: %d", player_symbol, player_score);

    TTF_Font* font = TTF_OpenFont(font_path, FONT_SIZE);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color white = {255, 255, 255, 255}; // White color for text

    SDL_Surface* surface_message = TTF_RenderText_Blended(font, score_text, white);
    if (!surface_message) {
        fprintf(stderr, "Failed to render text: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* texture_message = SDL_CreateTextureFromSurface(renderer, surface_message);
    if (!texture_message) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface_message);
        TTF_CloseFont(font);
        return;
    }

    SDL_Rect message_rect = {
        x,
        y,
        surface_message->w,
        surface_message->h
    };

    SDL_RenderCopy(renderer, texture_message, NULL, &message_rect);

    SDL_FreeSurface(surface_message);
    SDL_DestroyTexture(texture_message);
    TTF_CloseFont(font);
}
