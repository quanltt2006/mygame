#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
using namespace std;

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 1000;
const int BIRD_SIZE = 40;
const int GRAVITY = 1;
const int JUMP_STRENGTH = -15;
const int BIRD_SPEED = 5;
const int OBSTACLE_WIDTH = 50;
const int OBSTACLE_GAP = 500

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr; 
SDL_Texture* obstacleTexture = nullptr;

struct Obstacle {
    int x, y, width, height;
};

vector<Obstacle> obstacles;

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) { // Khởi tạo SDL_image
        std::cerr << "SDL_image could not initialize! SDL_Error: " << IMG_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) { // Khởi tạo SDL_ttf
        std::cerr << "SDL_ttf could not initialize! SDL_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Flappy Bird Movement", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool loadMedia() {
    SDL_Surface* surface = IMG_Load("duck.jpg");
    if (!surface) {
        std::cerr << "Failed to load player image! SDL_Error: " << IMG_GetError() << std::endl;
        return false;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); 

    if (!texture) {
        std::cerr << "Failed to create texture from surface! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    surface = IMG_Load("cnv.png");
    if (!surface) {
        std::cerr << "Failed to load obstacle image! SDL_Error: " << IMG_GetError() << std::endl;
        return false;
    }

    obstacleTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!obstacleTexture) {
        std::cerr << "Failed to create texture from surface! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void close() {
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(obstacleTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit(); 
    IMG_Quit();
    SDL_Quit();
}

void generateObstacles() {
    obstacles.clear();

    const int MIN_GAP = 200; 
    const int MAX_HEIGHT = SCREEN_HEIGHT / 3; 

    int topHeight = rand() % (MAX_HEIGHT - MIN_GAP) + MIN_GAP;
    Obstacle leftTop = {0, 0, OBSTACLE_WIDTH, topHeight};
    Obstacle rightTop = {SCREEN_WIDTH - OBSTACLE_WIDTH, 0, OBSTACLE_WIDTH, topHeight};

    int bottomHeight = rand() % (MAX_HEIGHT - MIN_GAP) + MIN_GAP;
    int bottomY = SCREEN_HEIGHT - bottomHeight;
    Obstacle leftBottom = {0, bottomY, OBSTACLE_WIDTH, bottomHeight};
    Obstacle rightBottom = {SCREEN_WIDTH - OBSTACLE_WIDTH, bottomY, OBSTACLE_WIDTH, bottomHeight};

    obstacles.push_back(leftTop);
    obstacles.push_back(rightTop);
    obstacles.push_back(leftBottom);
    obstacles.push_back(rightBottom);

    if (rand() % 2 == 0) { // 50% có chướng ngại vật giữa
        int midY = topHeight + MIN_GAP + rand() % (SCREEN_HEIGHT - topHeight - bottomHeight - 2 * MIN_GAP);
        int midHeight = 150 ;

        Obstacle rightMid = {SCREEN_WIDTH - OBSTACLE_WIDTH, midY, OBSTACLE_WIDTH, midHeight};

        obstacles.push_back(rightMid);
    }

else {int midY = topHeight + MIN_GAP + rand() % (SCREEN_HEIGHT - topHeight - bottomHeight - 2 * MIN_GAP);
        int midHeight = 150;
            Obstacle leftMid = {0, midY, OBSTACLE_WIDTH, midHeight};
        obstacles.push_back(leftMid);


}


}


bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

int main(int argc, char* argv[]) {
    if (!init() || !loadMedia()) {
        return -1;
    }

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Chunk* jumpSound = Mix_LoadWAV("click.wav");
    Mix_Chunk* jumpSound1 = Mix_LoadWAV("beep.wav");

    TTF_Font* ourfont = TTF_OpenFont("hihi.ttf", 32);
    if (!ourfont) {
        std::cerr << "Failed to load font! SDL_Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    int birdX = SCREEN_WIDTH / 2;
    int birdY = SCREEN_HEIGHT / 2;
    int birdVelY = 0;
    int birdVelX = BIRD_SPEED; // Horizontal movement speed

    bool quit = false;
    SDL_Event e;

    bool flipped = false;
    int x = 0;

    generateObstacles();

   Uint32 lastPassedTime = 0; // Lưu thời gian vượt qua chướng ngại vật
int score = 0; // Điểm số
bool gameOver = false; // Trạng thái game over
int gameOverRectY = SCREEN_HEIGHT;
while (!quit) {
    Uint32 currentTime = SDL_GetTicks(); // Lấy thời gian hiện tại

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_SPACE) {
                if (gameOver) {
                    // Reset game state
                    gameOver = false;
                    gameOverRectY = SCREEN_HEIGHT;
                    birdX = SCREEN_WIDTH / 2;
                    birdY = SCREEN_HEIGHT / 2;
                    birdVelY = 0;
                    score = 0;
                    generateObstacles();
                } else {
                    birdVelY = JUMP_STRENGTH;
                    Mix_PlayChannel(-1, jumpSound, 0);
                }
            }
        }
    }

    if (!gameOver) {
        // Cập nhật vị trí chim
        birdVelY += GRAVITY;
        birdY += birdVelY;
        birdX += birdVelX;

        // Kiểm tra nếu chim chạm vào cạnh màn hình
        if (birdX < 0 || birdX + BIRD_SIZE > SCREEN_WIDTH) {
            birdVelX = -birdVelX; 
            flipped = !flipped;
            score++; 
            Mix_PlayChannel(-1, jumpSound1, 0);
            lastPassedTime = SDL_GetTicks();}

        if (lastPassedTime > 0 && SDL_GetTicks() - lastPassedTime >= 500) {
            generateObstacles();
            lastPassedTime = 0;
        }

        if (birdY < 0) {
            birdY = 0;
            birdVelY = 0;
        } else if (birdY + BIRD_SIZE > SCREEN_HEIGHT) {
            birdY = SCREEN_HEIGHT - BIRD_SIZE;
            birdVelY = 0;
        }

        SDL_Rect birdRect = {birdX, birdY, BIRD_SIZE, BIRD_SIZE};
        for (auto& obstacle : obstacles) {
            SDL_Rect obstacleRect = {obstacle.x, obstacle.y, obstacle.width, obstacle.height};
            if (checkCollision(birdRect, obstacleRect)) {
                gameOver = true;
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_Rect birdRect = {birdX, birdY, BIRD_SIZE, BIRD_SIZE};
    SDL_RenderCopyEx(renderer, texture, NULL, &birdRect, 0, NULL, flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

    for (auto& obstacle : obstacles) {
        SDL_Rect obstacleRect = {obstacle.x, obstacle.y, obstacle.width, obstacle.height};
        SDL_RenderCopy(renderer, obstacleTexture, NULL, &obstacleRect);
    }

    string scoreText = "Score: " + to_string(score);
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(ourfont, scoreText.c_str(), {255, 255, 255});
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Rect scoreRect = {20, 20, scoreSurface->w, scoreSurface->h};
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    // Vẽ "Game Over" nếu game over
    if (gameOver) {
        // Di chuyển hình chữ nhật từ dưới lên
        if (gameOverRectY > SCREEN_HEIGHT / 2 - 50) {
            gameOverRectY -= 20; // Tốc độ di chuyển
        }

        SDL_Rect gameOverRect = {SCREEN_WIDTH / 2 - 100, gameOverRectY, 200, 100};
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Màu đỏ
        SDL_RenderFillRect(renderer, &gameOverRect);

        SDL_Surface* gameOverSurface = TTF_RenderText_Solid(ourfont, "Game Over", {0, 0, 0});
        SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
        SDL_Rect gameOverTextRect = {SCREEN_WIDTH / 2 - 100, gameOverRectY + 20, gameOverSurface->w, gameOverSurface->h};
        SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverTextRect);

        SDL_Surface* replaySurface = TTF_RenderText_Solid(ourfont, "Press SPACE to replay", {0, 0,0});
        SDL_Texture* replayTexture = SDL_CreateTextureFromSurface(renderer, replaySurface);
        SDL_Rect replayTextRect = {SCREEN_WIDTH / 2 - 200 , gameOverRectY + 60, replaySurface->w, replaySurface->h};
        SDL_RenderCopy(renderer, replayTexture, NULL, &replayTextRect);

        SDL_FreeSurface(gameOverSurface);
        SDL_DestroyTexture(gameOverTexture);
        SDL_FreeSurface(replaySurface);
        SDL_DestroyTexture(replayTexture);
    }

    SDL_RenderPresent(renderer);

    SDL_Delay(16);
}

    TTF_CloseFont(ourfont);
    Mix_FreeChunk(jumpSound);
    Mix_FreeChunk(jumpSound1);
    Mix_CloseAudio();
    close();
cout << score;
    return 0;
}
