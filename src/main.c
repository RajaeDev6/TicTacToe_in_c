#include <SDL2/SDL.h>
#include <unistd.h>
#include <stdio.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define gridSize 3

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

char *player1 = "x";
char *player2 = "o";

int player1_score = 0;
int player2_score = 0;
int moves = 0;



char font_path[1024];


int board[gridSize][gridSize]; // 3x3 board state: 0 = empty, 1 = X, -1 = O
int currentPlayer = 1; // 1 for X, -1 for O

typedef struct {
    SDL_Rect rect;
    SDL_Color color;
} Container;

int check_win() {
    // Check rows
    for (int i = 0; i < gridSize; i++) {
        if (board[i][0] != 0 && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            return board[i][0];
        }
    }
    
    // Check columns
    for (int i = 0; i < gridSize; i++) {
        if (board[0][i] != 0 && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            return board[0][i];
        }
    }
    
    // Check diagonals
    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        return board[0][0];
    }
    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        return board[0][2];
    }
    
    // Return 0 if no win condition is met
    return 0;
}

// Update the score for a player
void update_score(int *score) {
    (*score)++;
}

void init_container(Container *container, int x, int y, int w, int h, SDL_Color color) {
    container->rect.x = x;
    container->rect.y = y;
    container->rect.w = w;
    container->rect.h = h;
    container->color = color;
}

Container container;
int containerWidth = WINDOW_WIDTH * 0.7;
int containerHeight = WINDOW_HEIGHT * 0.7;

///////////////////////
int SDL_RenderDrawCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
    int offsetx, offsety, d;
    int status;

    /* CHECK_RENDERER_MAGIC(renderer, -1); */

    offsetx = 0;
    offsety = radius;
    d = radius -1;
    status = 0;

    while (offsety >= offsetx) {
        status += SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
        status += SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
        status += SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
        status += SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
        status += SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
        status += SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
        status += SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
        status += SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }

    return status;
}

///

void reset_board() {
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            board[i][j] = 0; // Set all cells to empty
        }
    }
}



void Init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("tiktak", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
}

void destroy_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void render_container(SDL_Renderer *renderer, const Container *container) {
    SDL_SetRenderDrawColor(renderer, container->color.r, container->color.g, container->color.b, container->color.a);
    SDL_RenderFillRect(renderer, &container->rect);
}

void draw_board(const Container *container) {
    SDL_SetRenderDrawColor(renderer, 0,0,0, 255); 

    for (int i = 1; i < 3; i++) {
        int x = container->rect.w * i / 3 + container->rect.x;
        SDL_RenderDrawLine(renderer, x, container->rect.y, x, container->rect.y + container->rect.h);
    }

    for (int i = 1; i < 3; i++) {
        int y = container->rect.h * i / 3 + container->rect.y;
        SDL_RenderDrawLine(renderer, container->rect.x, y, container->rect.x + container->rect.w, y);
    }
}



void draw_X(SDL_Renderer* renderer, int x, int y, int size) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
    
    // Calculate the diagonal lines for the X
    int halfSize = size / 2;
    int startX = x + halfSize;
    int startY = y + halfSize;
    
    // Draw the first line of the X (from top-left to bottom-right)
    SDL_RenderDrawLine(renderer, startX, startY, x + size, y + size);
    // Draw the second line of the X (from bottom-left to top-right)
    SDL_RenderDrawLine(renderer, startX, y + size, x + size, startY);
}


void draw_circle(SDL_Renderer* renderer, int x, int y, int radius) {
    SDL_RenderDrawCircle(renderer, x, y, radius);
}

void render_player_turn(SDL_Renderer *renderer, const char *player) {
    // Open the font with a suitable font size
    TTF_Font* Noto = TTF_OpenFont(font_path, 48); // Choose a suitable font size (e.g., 16)
    if (!Noto) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    // Define the color for the text
    SDL_Color White = {255, 255, 255, 255}; // Full white

    // Render the text using anti-aliasing (blended rendering) for better quality
    SDL_Surface* surfaceMessage = TTF_RenderText_Blended(Noto, player, White);
    if (!surfaceMessage) {
        printf("Failed to render text: %s\n", TTF_GetError());
        TTF_CloseFont(Noto);
        return;
    }

    // Create a texture from the rendered surface
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    if (!Message) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surfaceMessage);
        TTF_CloseFont(Noto);
        return;
    }

    // Define the rectangle for rendering the text
    SDL_Rect Message_rect;
    Message_rect.x = WINDOW_WIDTH / 2 - surfaceMessage->w / 2; // Center the text horizontally
    Message_rect.y = 0; // Position at the top of the screen
    Message_rect.w = surfaceMessage->w; // Width of the text surface
    Message_rect.h = surfaceMessage->h; // Height of the text surface

    // Render the text on the screen
    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

    // Clean up resources
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
    TTF_CloseFont(Noto);
}

void show_score(SDL_Renderer *renderer, const char *player, int playerScore, int x, int y) {
    char scoreText[50];

    // Use snprintf instead of sprintf to format the score string
    snprintf(scoreText, sizeof(scoreText), "%s: %d", player, playerScore);

    TTF_Font* Noto = TTF_OpenFont(font_path, 32);
    if (!Noto) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color White = {255, 255, 255};
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Noto, scoreText, White);
    if (!surfaceMessage) {
        printf("Failed to render text: %s\n", TTF_GetError());
        TTF_CloseFont(Noto);
        return;
    }

    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    if (!Message) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surfaceMessage);
        TTF_CloseFont(Noto);
        return;
    }

    // Define the rectangle for rendering the text
    SDL_Rect Message_rect;
    Message_rect.x = x; // x-coordinate
    Message_rect.y = y; // y-coordinate
    Message_rect.w = surfaceMessage->w; // width of the text surface
    Message_rect.h = surfaceMessage->h; // height of the text surface

    // Render the text on the screen
    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

    // Clean up resources
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
    TTF_CloseFont(Noto);
}

void update_screen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    render_container(renderer, &container);
		show_score(renderer, player1, player1_score, 0, WINDOW_HEIGHT / 2);
		show_score(renderer, player2, player2_score, WINDOW_WIDTH - 100, WINDOW_HEIGHT / 2);
    draw_board(&container);

		// Draw Xs and Os based on the board state
    int cellWidth = container.rect.w / gridSize;
    int cellHeight = container.rect.h / gridSize;

    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            int x = container.rect.x + j * cellWidth;
            int y = container.rect.y + i * cellHeight;
            int centerX = x + cellWidth / 2;
            int centerY = y + cellHeight / 2;

            if (board[i][j] == 1) {
                // Draw X
                draw_X(renderer, x, y, cellWidth / 2);
            } else if (board[i][j] == -1) {
                // Draw O
                draw_circle(renderer, centerX, centerY, (cellWidth * 0.5) / 2);
            }
        }
    }

		const char *current_player_symbol = (currentPlayer == 1) ? player1 : player2;

    render_player_turn(renderer, current_player_symbol);

    SDL_RenderPresent(renderer);
}



void handle_click() {
    // Get mouse click coordinates
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // Calculate which cell was clicked
    int cellWidth = container.rect.w / gridSize;
    int cellHeight = container.rect.h / gridSize;

    int col = (mouseX - container.rect.x) / cellWidth;
    int row = (mouseY - container.rect.y) / cellHeight;

    // Ensure the click was within the container bounds and cell is empty
    if (row >= 0 && row < gridSize && col >= 0 && col < gridSize && board[row][col] == 0) {
        // Place the current player's mark in the clicked cell
        board[row][col] = currentPlayer;
        // Switch to the other player
        currentPlayer = -currentPlayer;

				moves++;
    }
}


int main() {
	
    Init();

    if (TTF_Init() != 0) {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        exit(1);
    }

    // Initialize the board state to empty
    reset_board();

    SDL_Color container_background = {255, 255, 255, 255};
    init_container(&container, (WINDOW_WIDTH - containerWidth) / 2, (WINDOW_HEIGHT - containerHeight) / 2, containerWidth, containerHeight, container_background);

    SDL_Event e;
    int RUN = 0;

		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		sprintf(font_path, "%s/font/NotoSansMono-Regular.ttf", cwd);



    while (!RUN) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                RUN = 1;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
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
                } else if (winner == 0 && moves == 9) {
									reset_board();
									moves = 0;
								}
            }
        }
        update_screen();
    }

    TTF_Quit();
    destroy_window();
    return 0;
}
