#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define ROCKET_WIDTH 90
#define ROCKET_HEIGHT 90
#define BOMB_WIDTH 50
#define BOMB_HEIGHT 50
#define ALIEN_WIDTH 100 
#define ALIEN_HEIGHT 100
#define BULLET_WIDTH 50
#define BULLET_HEIGHT 30
#define MAX_BOMBS 15
#define MAX_ALIENS 5
#define MAX_BULLETS 30

typedef struct {
    int x, y;
    bool active;
    bool exploded; 
} Bomb;
typedef struct {
    int x, y;
    bool active;
    bool exploded; 
} alien;



typedef struct {
    int x, y;
    bool active;
} Bullet;

typedef struct {
    int x, y;
    Uint32 startTime;
    bool active;
    int alpha; // Alpha value for fading effect
} BangEffect;
typedef struct {
    int x, y;
    Uint32 startTime;
    bool active;
    int alpha; // Alpha value for fading effect
} ScoreIndicator;
typedef struct {
    int x, y;
    Uint32 startTime;
    bool active;
    int alpha; // Alpha value for fading effect
} alienScoreIndicator;

typedef struct {
    int x, y;
    Uint32 startTime;
    bool active;
    int alpha; // Alpha value for fading effect
} LevelIndicator;
ScoreIndicator scoreIndicator = {0, 0, 0, false, 0}; // Initialize score indicator
alienScoreIndicator alienscoreIndicator = {0, 0, 0, false, 0}; // Initialize score indicator
LevelIndicator levelIndicator = {0, 0, 0, false, 0}; // Initialize level indicator


int score = 0; // Initialize score
int level = 1; // Initialize level


SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *rocketTexture = NULL;
SDL_Texture *bombTexture = NULL;
SDL_Texture *alienTexture = NULL;
SDL_Texture *bulletTexture = NULL;
SDL_Texture *bangTexture = NULL; // Texture for bang effect
SDL_Texture *backgroundTexture = NULL; // Background texture
Mix_Chunk *laserSound = NULL; // Laser sound effect
TTF_Font *font = NULL; // Font for score and level

int bgY1 = 0; // First background Y position for scrolling
int bgY2 = -SCREEN_HEIGHT; // Second background Y position for scrolling
int bombCount = 0;
int alienCount = 0;
float fallSpeed = 2.5; // Initial bomb falling speed



void init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Initialize SDL with audio
     TTF_Init(); // Initialize SDL_ttf
    window = SDL_CreateWindow("Rocket Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White background
    
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }
    
    font = TTF_OpenFont("C:/Windows/Fonts/impact.ttf", 24); // Load your font file here
    if (!font) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
    }
}




void loadTextures() {
    SDL_Surface *rocketSurface = SDL_LoadBMP("rocket.bmp");
    SDL_SetColorKey(rocketSurface, SDL_TRUE, SDL_MapRGB(rocketSurface->format, 255, 255, 255));
    rocketTexture = SDL_CreateTextureFromSurface(renderer, rocketSurface);
    SDL_FreeSurface(rocketSurface);

    SDL_Surface *bombSurface = SDL_LoadBMP("bomb1.bmp");
    SDL_SetColorKey(bombSurface, SDL_TRUE, SDL_MapRGB(bombSurface->format, 255, 255, 255));
    bombTexture = SDL_CreateTextureFromSurface(renderer, bombSurface);
    SDL_FreeSurface(bombSurface);

    SDL_Surface *alienSurface = SDL_LoadBMP("alien1.bmp");
    SDL_SetColorKey(alienSurface, SDL_TRUE, SDL_MapRGB(alienSurface->format, 255, 255, 255));
    alienTexture = SDL_CreateTextureFromSurface(renderer, alienSurface);
    SDL_FreeSurface(alienSurface);

    SDL_Surface *bulletSurface = SDL_LoadBMP("bullet.bmp");
    SDL_SetColorKey(bulletSurface, SDL_TRUE, SDL_MapRGB(bulletSurface->format, 255, 255, 255));
    bulletTexture = SDL_CreateTextureFromSurface(renderer, bulletSurface);
    SDL_FreeSurface(bulletSurface);

    // Load the bang effect
    SDL_Surface *bangSurface = SDL_LoadBMP("bang1.bmp");
    SDL_SetColorKey(bangSurface, SDL_TRUE, SDL_MapRGB(bangSurface->format, 0, 0, 0));
    bangTexture = SDL_CreateTextureFromSurface(renderer, bangSurface);
    SDL_FreeSurface(bangSurface);

    // Load the background
    SDL_Surface *backgroundSurface = SDL_LoadBMP("background.bmp");
    backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);

    // Load the laser sound effect
    laserSound = Mix_LoadWAV("laser.mp3"); // Ensure you have a laser.wav file in the working directory
    if (!laserSound) {
        printf("Failed to load laser sound! SDL_mixer Error: %s\n", Mix_GetError());
    }
}

void cleanUp() {
    Mix_FreeChunk(laserSound); // Free the sound effect
    SDL_DestroyTexture(rocketTexture);
    SDL_DestroyTexture(bombTexture);
    SDL_DestroyTexture(alienTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyTexture(bangTexture); // Free the bang effect texture
    SDL_DestroyTexture(backgroundTexture); // Free the background texture
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio(); // Close audio
    SDL_Quit();
}

void handleInput(bool *running, int *rocketX, int *rocketY, Bullet bullets[]) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                *running = false;
            }
            if (event.key.keysym.sym == SDLK_SPACE) {
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].active) {
                        bullets[i].x = *rocketX + (ROCKET_WIDTH / 2) - (BULLET_WIDTH / 2);
                        bullets[i].y = *rocketY;
                        bullets[i].active = true;
                        Mix_PlayChannel(-1, laserSound, 0); // Play laser sound
                        break;
                    }
                }
            }
        }
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_LEFT] && *rocketX > 0) {
        *rocketX -= 8;
    }
    if (state[SDL_SCANCODE_RIGHT] && *rocketX < SCREEN_WIDTH - ROCKET_WIDTH) {
        *rocketX += 8;
    }
    if (state[SDL_SCANCODE_UP] && *rocketY > 0) {
        *rocketY -= 6;
    }
    if (state[SDL_SCANCODE_DOWN] && *rocketY < SCREEN_HEIGHT - ROCKET_HEIGHT) {
        *rocketY += 6;
    }
}

void updateBackground() {
    bgY1 += 1; // Adjust this value to control the scrolling speed
    bgY2 += 1; // Keep both backgrounds moving at the same speed

    // Reset the background positions if they go off-screen
    if (bgY1 >= SCREEN_HEIGHT) {
        bgY1 = bgY2 - SCREEN_HEIGHT*2; // Reposition bgY1 to follow bgY2
    }
    if (bgY2 >= SCREEN_HEIGHT) {
        bgY2 = bgY1 - SCREEN_HEIGHT*2; // Reposition bgY2 to follow bgY1
    }
}

void spawnBomb(Bomb bombs[]) {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (!bombs[i].active) {
            bombs[i].x = rand() % (SCREEN_WIDTH - BOMB_WIDTH);
            bombs[i].y = 0; // Start at the top of the screen
            bombs[i].active = true;
            bombs[i].exploded = false; // Mark as not exploded
            break;
        }
    }
}
void spawnAlien(alien aliens[]) {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (!aliens[i].active) {
            aliens[i].x = rand() % (SCREEN_WIDTH - BOMB_WIDTH);
            aliens[i].y = 0; // Start at the top of the screen
            aliens[i].active = true;
            aliens[i].exploded = false; // Mark as not exploded
            break;
        }
    }
}

void updateBombs(Bomb bombs[]) {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bombs[i].active) {
            bombs[i].y += fallSpeed; // Bomb falling speed
            if (bombs[i].y > SCREEN_HEIGHT) {
                bombs[i].active = false; // Deactivate if it goes off-screen
            }
        }
    }
}
void updateAliens(alien aliens[]) {
    for (int i = 0; i < MAX_ALIENS; i++) {
        if (aliens[i].active) {
            aliens[i].y += fallSpeed; // Bomb falling speed
            if (aliens[i].y > SCREEN_HEIGHT) {
                aliens[i].active = false; // Deactivate if it goes off-screen
            }
        }
    }
}

void updateBullets(Bullet bullets[]) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= 10; // Bullet moving speed
            if (bullets[i].y < 0) {
                bullets[i].active = false; // Deactivate if it goes off-screen
            }
        }
    }
}

bool checkCollision(int rocketX, int rocketY, Bomb bomb) {
    return bomb.active &&
           rocketX < bomb.x + BOMB_WIDTH &&
           rocketX + ROCKET_WIDTH > bomb.x &&
           rocketY < bomb.y + BOMB_HEIGHT &&
           rocketY + ROCKET_HEIGHT > bomb.y;
}
bool checkAlienCollision(int rocketX, int rocketY, alien alien) {
    return alien.active &&
           rocketX < alien.x + ALIEN_WIDTH &&
           rocketX + ROCKET_WIDTH > alien.x &&
           rocketY < alien.y + ALIEN_HEIGHT &&
           rocketY + ROCKET_HEIGHT > alien.y;
}

bool checkBulletCollision(Bullet bullet, Bomb bomb) {
    return bullet.active &&
           bullet.x < bomb.x + BOMB_WIDTH &&
           bullet.x + BULLET_WIDTH > bomb.x &&
           bullet.y < bomb.y + BOMB_HEIGHT &&
           bullet.y + BULLET_HEIGHT > bomb.y;
}
bool checkBulletAlienCollision(Bullet bullet, alien alien) {
    return bullet.active &&
           bullet.x < alien.x + ALIEN_WIDTH &&
           bullet.x + BULLET_WIDTH > alien.x &&
           bullet.y < alien.y + ALIEN_HEIGHT &&
           bullet.y + BULLET_HEIGHT > alien.y;
}

void renderScoreAndLevel() {
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);
    SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreText, (SDL_Color){135, 206, 250}); // Sky blue color
    SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Rect scoreRect = {10, 10, scoreSurface->w, scoreSurface->h};
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    char levelText[50];
    sprintf(levelText, "Level: %d", level);
    SDL_Surface *levelSurface = TTF_RenderText_Solid(font, levelText, (SDL_Color){255, 255, 255}); // White color
    SDL_Texture *levelTexture = SDL_CreateTextureFromSurface(renderer, levelSurface);
    SDL_Rect levelRect = {SCREEN_WIDTH - levelSurface->w - 10, 10, levelSurface->w, levelSurface->h};
    SDL_RenderCopy(renderer, levelTexture, NULL, &levelRect);
    SDL_FreeSurface(levelSurface);
    SDL_DestroyTexture(levelTexture);
}


void render(int rocketX, int rocketY,alien aliens[], Bomb bombs[], Bullet bullets[], BangEffect bangEffect) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 0);
    SDL_RenderClear(renderer);

    // Render the backgrounds
    SDL_Rect bgRect1 = {0, bgY1, SCREEN_WIDTH, SCREEN_HEIGHT*2};
    SDL_RenderCopy(renderer, backgroundTexture, NULL, &bgRect1);
    SDL_Rect bgRect2 = {0, bgY2, SCREEN_WIDTH, SCREEN_HEIGHT*2};
    SDL_RenderCopy(renderer, backgroundTexture, NULL, &bgRect2);

    // Draw rocket
    SDL_Rect rocketRect = {rocketX, rocketY, ROCKET_WIDTH, ROCKET_HEIGHT};
    SDL_RenderCopy(renderer, rocketTexture, NULL, &rocketRect);

    // Draw bombs
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bombs[i].active) {
            SDL_Rect bombRect = {bombs[i].x, bombs[i].y, BOMB_WIDTH, BOMB_HEIGHT};
            SDL_RenderCopy(renderer, bombTexture, NULL, &bombRect);
        }
    }

    for (int i = 0; i < MAX_ALIENS; i++) {
        if (aliens[i].active) {
            SDL_Rect alienRect = {aliens[i].x, aliens[i].y, ALIEN_WIDTH, ALIEN_HEIGHT};
            SDL_RenderCopy(renderer, alienTexture, NULL, &alienRect);
        }
    }

    // Draw bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            SDL_Rect bulletRect = {bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT};
            SDL_RenderCopy(renderer, bulletTexture, NULL, &bulletRect);
        }
    }

    // Handle bang effect rendering
    if (bangEffect.active) {
        SDL_SetTextureAlphaMod(bangTexture, bangEffect.alpha); // Set alpha for bang effect
        SDL_Rect bangRect = {bangEffect.x, bangEffect.y, BOMB_WIDTH, BOMB_HEIGHT}; // Position bang effect
        SDL_RenderCopy(renderer, bangTexture, NULL, &bangRect);
    }
    renderScoreAndLevel();  // Add this line to render score and level

    // Render score indicator
        if (scoreIndicator.active) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, scoreIndicator.alpha); // Green color
            SDL_Surface *scoreIndicatorSurface = TTF_RenderText_Solid(font, "+10", (SDL_Color){0, 255, 0}); // Green color
            SDL_Texture *scoreIndicatorTexture = SDL_CreateTextureFromSurface(renderer, scoreIndicatorSurface);
            SDL_Rect scoreIndicatorRect = {scoreIndicator.x, scoreIndicator.y, scoreIndicatorSurface->w, scoreIndicatorSurface->h};
            SDL_RenderCopy(renderer, scoreIndicatorTexture, NULL, &scoreIndicatorRect);
            SDL_FreeSurface(scoreIndicatorSurface);
            SDL_DestroyTexture(scoreIndicatorTexture);
        }
        if (alienscoreIndicator.active) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, alienscoreIndicator.alpha); // Green color
            SDL_Surface *scoreIndicatorSurface = TTF_RenderText_Solid(font, "+20", (SDL_Color){0, 255, 0}); // Green color
            SDL_Texture *scoreIndicatorTexture = SDL_CreateTextureFromSurface(renderer, scoreIndicatorSurface);
            SDL_Rect scoreIndicatorRect = {alienscoreIndicator.x, alienscoreIndicator.y, scoreIndicatorSurface->w, scoreIndicatorSurface->h};
            SDL_RenderCopy(renderer, scoreIndicatorTexture, NULL, &scoreIndicatorRect);
            SDL_FreeSurface(scoreIndicatorSurface);
            SDL_DestroyTexture(scoreIndicatorTexture);
        }

        // Render level indicator
        if (levelIndicator.active) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, levelIndicator.alpha); // Green color
            char levelText[10]; // You need to declare this here
            sprintf(levelText, "+1"); // Set the text for level increase
            SDL_Surface *levelIndicatorSurface = TTF_RenderText_Solid(font, levelText, (SDL_Color){0, 255, 0}); // Green color
            SDL_Texture *levelIndicatorTexture = SDL_CreateTextureFromSurface(renderer, levelIndicatorSurface);
            SDL_Rect levelIndicatorRect = {levelIndicator.x, levelIndicator.y, levelIndicatorSurface->w, levelIndicatorSurface->h};
            SDL_RenderCopy(renderer, levelIndicatorTexture, NULL, &levelIndicatorRect);
            SDL_FreeSurface(levelIndicatorSurface);
            SDL_DestroyTexture(levelIndicatorTexture);
        }


    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    init();
    loadTextures();
    char levelText[10]; // Enough to hold the level text like "+1", "+2", etc.

    bool running = true;
    int rocketX = SCREEN_WIDTH / 2 - ROCKET_WIDTH / 2;
    int rocketY = SCREEN_HEIGHT - ROCKET_HEIGHT; // Initial position of the rocket
    Bomb bombs[MAX_BOMBS] = {0};  
    alien aliens[MAX_ALIENS] = {0}; 
    Bullet bullets[MAX_BULLETS] = {0};
    BangEffect bangEffect = {0, 0, 0, false, 0}; // Initialize bang effect with alpha

    Uint32 lastBombSpawn = SDL_GetTicks();
    Uint32 lastAlienSpawn = SDL_GetTicks();
    
    

    while (running) {
        handleInput(&running, &rocketX, &rocketY, bullets);
        updateBackground(); // Update the background scrolling
        updateBombs(bombs);
        updateAliens(aliens);
        updateBullets(bullets);

        if (SDL_GetTicks() - lastBombSpawn > 500) { // Spawn bomb every half second
            spawnBomb(bombs);
            lastBombSpawn = SDL_GetTicks();
        }
        if (SDL_GetTicks() - lastAlienSpawn > 5000) { // Spawn bomb every half second
            spawnAlien(aliens);
            lastAlienSpawn = SDL_GetTicks();
        }

        // Handle fade-in and fade-out for score indicator
        if (scoreIndicator.active) {
            Uint32 elapsed = SDL_GetTicks() - scoreIndicator.startTime;
            if (elapsed < 500) {
                scoreIndicator.alpha = (int)(255 * (elapsed / 500.0)); // Fade in
                scoreIndicator.y = scoreIndicator.y - 1;
            } else if (elapsed < 600) {
                scoreIndicator.alpha = 255 - (int)(255 * ((elapsed - 500) / 500.0)); // Fade out
            } else {
                scoreIndicator.active = false; // Deactivate after 600 ms
            }
        }

        if (alienscoreIndicator.active) {
            Uint32 elapsed = SDL_GetTicks() - alienscoreIndicator.startTime;
            if (elapsed < 500) {
                alienscoreIndicator.alpha = (int)(255 * (elapsed / 500.0)); // Fade in
                alienscoreIndicator.y = alienscoreIndicator.y - 1;
            } else if (elapsed < 600) {
                alienscoreIndicator.alpha = 255 - (int)(255 * ((elapsed - 500) / 500.0)); // Fade out
            } else {
                alienscoreIndicator.active = false; // Deactivate after 600 ms
            }
        }

        // Handle fade-in and fade-out for level indicator
        if (levelIndicator.active) {
            Uint32 elapsed = SDL_GetTicks() - levelIndicator.startTime;
            if (elapsed < 500) {
                levelIndicator.alpha = (int)(255 * (elapsed / 500.0)); // Fade in
                levelIndicator.y = levelIndicator.y - 1;
            } else if (elapsed < 700) {
                levelIndicator.alpha = 255 - (int)(255 * ((elapsed - 500) / 500.0)); // Fade out
            } else {
                levelIndicator.active = false; // Deactivate after 700 ms
            }
        }



        

        // Check for collisions with rocket
        for (int i = 0; i < MAX_BOMBS; i++) {
            if (checkCollision(rocketX, rocketY, bombs[i])) {
                running = false;
                printf("Game Over! A bomb hit your rocket!\n");
                break;
            }
        }
        // Check for collisions with rocket
        for (int i = 0; i < MAX_ALIENS; i++) {
            if (checkAlienCollision(rocketX, rocketY, aliens[i])) {
                running = false;
                printf("Game Over! A bomb hit your rocket!\n");
                break;
            }
        }

        // Check for bullet-bomb collisions
        for (int i = 0; i < MAX_BULLETS; i++) {
            for (int j = 0; j < MAX_BOMBS; j++) {
                // Only activate bang effect if the bomb has not exploded yet
                if (checkBulletCollision(bullets[i], bombs[j]) && !bombs[j].exploded) {
                    bullets[i].active = false; // Deactivate bullet
                    bombs[j].active = false; // Deactivate bomb
                    bombs[j].exploded = true; // Mark bomb as exploded
                    score= score+10;

                    // Set bang effect position, activate it, and set initial alpha
                    bangEffect.x = bombs[j].x; 
                    bangEffect.y = bombs[j].y; 
                    bangEffect.startTime = SDL_GetTicks();
                    bangEffect.active = true;
                    bangEffect.alpha = 0; // Start with fully transparent
                    // Set score indicator position and activate it
                    scoreIndicator.x = 10; // Position below the score
                    scoreIndicator.y = 50; // Adjust this for where you want it to appear
                    scoreIndicator.startTime = SDL_GetTicks();
                    scoreIndicator.active = true;
                    scoreIndicator.alpha = 0; // Start fully transparent

                    // Level up logic
                    if (score % 100 == 0) {
                        level++;
                        fallSpeed = fallSpeed + 0.5;

                        // Set level indicator position and activate it
                        levelIndicator.x = SCREEN_WIDTH - 50 - (strlen(levelText) * 10); // Position to the left of the level
                        levelIndicator.y = 50; // Adjust this for where you want it to appear
                        levelIndicator.startTime = SDL_GetTicks();
                        levelIndicator.active = true;
                        levelIndicator.alpha = 0; // Start fully transparent
                    }
                    break; // Break inner loop after the first collision
                }
            }
        }

        for (int i = 0; i < MAX_ALIENS; i++) {
            for (int j = 0; j < MAX_ALIENS; j++) {
                // Only activate bang effect if the bomb has not exploded yet
                if (checkBulletAlienCollision(bullets[i], aliens[j]) && !aliens[j].exploded) {
                    bullets[i].active = false; // Deactivate bullet
                    aliens[j].active = false; // Deactivate bomb
                    aliens[j].exploded = true; // Mark bomb as exploded
                    score= score+20;

                    // Set bang effect position, activate it, and set initial alpha
                    bangEffect.x = bombs[j].x; 
                    bangEffect.y = bombs[j].y; 
                    bangEffect.startTime = SDL_GetTicks();
                    bangEffect.active = true;
                    bangEffect.alpha = 0; // Start with fully transparent
                    // Set score indicator position and activate it
                    alienscoreIndicator.x = 10; // Position below the score
                    alienscoreIndicator.y = 50; // Adjust this for where you want it to appear
                    alienscoreIndicator.startTime = SDL_GetTicks();
                    alienscoreIndicator.active = true;
                    alienscoreIndicator.alpha = 0; // Start fully transparent

                    // Level up logic
                    if (score % 100 == 0) {
                        level++;
                        fallSpeed = fallSpeed + 0.5;

                        // Set level indicator position and activate it
                        levelIndicator.x = SCREEN_WIDTH - 50 - (strlen(levelText) * 10); // Position to the left of the level
                        levelIndicator.y = 50; // Adjust this for where you want it to appear
                        levelIndicator.startTime = SDL_GetTicks();
                        levelIndicator.active = true;
                        levelIndicator.alpha = 0; // Start fully transparent
                    }
                    break; // Break inner loop after the first collision
                }
            }
        }

        // Handle fade-in and fade-out for bang effect
        if (bangEffect.active) {
            // Calculate elapsed time since bang effect was activated
            Uint32 elapsed = SDL_GetTicks() - bangEffect.startTime;

            if (elapsed < 500) {
                // Fade in over 500 ms
                bangEffect.alpha = (int)(255 * (elapsed / 500.0)); // 0 to 255
            } else if (elapsed < 1000) {
                // Fade out over the next 500 ms
                bangEffect.alpha = 255 - (int)(255 * ((elapsed - 500) / 500.0)); // 255 to 0
            } else {
                bangEffect.active = false; // Deactivate bang effect after 1000 ms
            }
        }

        render(rocketX, rocketY,aliens, bombs, bullets, bangEffect);
        SDL_Delay(16); // Roughly 60 FPS
    }

    cleanUp();
    return 0;
}