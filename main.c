#include "raylib.h"
#include <stdlib.h>

#define LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#define NUM_PIPES 3

typedef enum {
    STATE_START,
    STATE_PLAYING,
    STATE_GAMEOVER
} GameState;

typedef struct {
    float x;
    float gapY;
    bool passed;
} Pipe;

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 832;
    int SpriteIndex = 0;
    float bgX = 0.0f;
    float gapSize = 250.0f;

    // --- Base Speed Settings ---
    const float basePipeSpeed = 4.0f;
    const float baseBgSpeed = 2.0f;
    const float baseGravity = 0.30f;

    // --- Dynamic Physics / Speed Variables ---
    float pipeSpeed = basePipeSpeed;
    float bgSpeed = baseBgSpeed;
    float gravity = baseGravity;

    Vector2 birdPos = { 150.0f, 400.0f };
    float birdVelocity = 0.0f;
    const float jumpStrength = -7.0f;

    // Score Variables
    int score = 0;
    int highScore = 0;

    GameState gameState = STATE_START;

    InitWindow(screenWidth, screenHeight, "Flappy Bird");

    // 1. Initialize Audio Device
    InitAudioDevice();

    // 2. Load Sound Effects and Background Music Stream
    Sound hitSound = LoadSound("audio_hit.wav");
    Sound dieSound = LoadSound("audio_die.wav");
    Music bgMusic = LoadMusicStream("audio_bgm.mp3");

    // Start playing background music on repeat
    PlayMusicStream(bgMusic);

    Texture2D background = LoadTexture("background-day.png");
    Texture2D pipeTex = LoadTexture("pipe-green.png");
    Texture2D messageTex = LoadTexture("message.png");
    Texture2D gameoverTex = LoadTexture("gameover.png");

    SetTargetFPS(60);

    Texture2D sprites[3];
    sprites[0] = LoadTexture("bluebird1.png");
    sprites[1] = LoadTexture("bluebird2.png");
    sprites[2] = LoadTexture("bluebird3.png");

    Pipe pipes[NUM_PIPES];
    float pipeSpacing = 450.0f;

    for (int i = 0; i < NUM_PIPES; i++) {
        pipes[i].x = screenWidth + (i * pipeSpacing);
        pipes[i].gapY = (float)GetRandomValue(150, 480);
        pipes[i].passed = false;
    }

    while (!WindowShouldClose())
    {
        // Keep streaming background music buffer
        UpdateMusicStream(bgMusic);

        bool inputPressed = IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        // --- 1. GAME LOGIC ---
        if (gameState == STATE_START)
        {
            if (inputPressed) {
                gameState = STATE_PLAYING;
                birdVelocity = jumpStrength;
            }
        }
        else if (gameState == STATE_PLAYING)
        {
            // Calculate speed multiplier based on score (increases every 10 points)
            int speedLevel = score / 10;
            pipeSpeed = basePipeSpeed + (speedLevel * 0.75f); // Pipes speed up
            bgSpeed = baseBgSpeed + (speedLevel * 0.35f);    // Background scrolls faster
            gravity = baseGravity + (speedLevel * 0.02f);    // Gravity increases slightly

            birdVelocity += gravity;
            birdPos.y += birdVelocity;

            if (inputPressed) {
                birdVelocity = jumpStrength;
            }

            // Ceiling or Ground Collision
            if (birdPos.y < 0 || birdPos.y > screenHeight - 50) {
                PlaySound(dieSound);
                gameState = STATE_GAMEOVER;
            }

            bgX -= bgSpeed;
            if (bgX <= -screenWidth) bgX = 0.0f;

            float birdWidth = sprites[0].width * 3.0f;
            float birdHeight = sprites[0].height * 3.0f;
            Rectangle birdRec = { birdPos.x, birdPos.y, birdWidth, birdHeight };

            for (int i = 0; i < NUM_PIPES; i++) {
                pipes[i].x -= pipeSpeed;

                float pipeWidth = pipeTex.width * 2.0f;
                Rectangle topPipeRec = { pipes[i].x, 0, pipeWidth, pipes[i].gapY };
                Rectangle bottomPipeRec = { pipes[i].x, pipes[i].gapY + gapSize, pipeWidth, screenHeight - (pipes[i].gapY + gapSize) };

                // Pipe Collision
                if (CheckCollisionRecs(birdRec, topPipeRec) || CheckCollisionRecs(birdRec, bottomPipeRec)) {
                    PlaySound(hitSound);
                    gameState = STATE_GAMEOVER;
                }

                // Score Point
                if (!pipes[i].passed && (pipes[i].x + pipeWidth) < birdPos.x) {
                    score++;
                    pipes[i].passed = true;
                    if (score > highScore) {
                        highScore = score;
                    }
                }

                // Recycle Pipes
                if (pipes[i].x < -100) {
                    float maxX = pipes[0].x;
                    for (int j = 1; j < NUM_PIPES; j++) {
                        if (pipes[j].x > maxX) maxX = pipes[j].x;
                    }

                    pipes[i].x = maxX + pipeSpacing;
                    pipes[i].gapY = (float)GetRandomValue(150, 480);
                    pipes[i].passed = false;
                }
            }
        }
        else if (gameState == STATE_GAMEOVER)
        {
            if (inputPressed) {
                birdPos.y = 400.0f;
                birdVelocity = 0.0f;
                score = 0;

                // Reset speeds to baseline
                pipeSpeed = basePipeSpeed;
                bgSpeed = baseBgSpeed;
                gravity = baseGravity;

                for (int i = 0; i < NUM_PIPES; i++) {
                    pipes[i].x = screenWidth + (i * pipeSpacing);
                    pipes[i].gapY = (float)GetRandomValue(150, 480);
                    pipes[i].passed = false;
                }
                gameState = STATE_PLAYING;
            }
        }

        // --- 2. RENDERING ---
        BeginDrawing();
            ClearBackground(BLACK);

            Rectangle sourceRec = { 0.0f, 0.0f, (float)background.width, (float)background.height };
            Rectangle destRec   = { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight };
            Vector2 origin      = { 0.0f, 0.0f };
            DrawTexturePro(background, sourceRec, destRec, origin, 0.0f, WHITE);

            for (int i = 0; i < NUM_PIPES; i++) {
                DrawTextureEx(pipeTex, (Vector2){ pipes[i].x + 80, pipes[i].gapY }, 180.0f, 2.0f, WHITE);
                DrawTextureEx(pipeTex, (Vector2){ pipes[i].x, pipes[i].gapY + gapSize }, 0.0f, 2.0f, WHITE);
            }

            DrawTextureEx(sprites[SpriteIndex], birdPos, 0.0f, 3.0f, WHITE);

            if (gameState == STATE_PLAYING || gameState == STATE_GAMEOVER) {
                DrawText(TextFormat("SCORE: %d", score), 40, 40, 40, WHITE);
                DrawText(TextFormat("HIGH SCORE: %d", highScore), 40, 90, 30, YELLOW);
            }

            if (gameState == STATE_START)
            {
                float scale = 2.5f;
                float msgX = (screenWidth - (messageTex.width * scale)) / 2.0f;
                float msgY = (screenHeight - (messageTex.height * scale)) / 2.0f;
                DrawTextureEx(messageTex, (Vector2){ msgX, msgY }, 0.0f, scale, WHITE);
            }

            if (gameState == STATE_GAMEOVER)
            {
                float scale = 3.0f;
                float goX = (screenWidth - (gameoverTex.width * scale)) / 2.0f;
                float goY = (screenHeight - (gameoverTex.height * scale)) / 2.0f - 50.0f;
                DrawTextureEx(gameoverTex, (Vector2){ goX, goY }, 0.0f, scale, WHITE);

                DrawText(TextFormat("FINAL SCORE: %d", score), (screenWidth / 2) - 120, goY + 150, 30, WHITE);
                DrawText("PRESS SPACE TO RESTART", (screenWidth / 2) - 180, goY + 200, 25, LIGHTGRAY);
            }

        EndDrawing();

        if (gameState == STATE_PLAYING) {
            SpriteIndex = (int)(GetTime() / 0.1) % LENGTH(sprites);
        }
    }

    // --- CLEANUP ---
    UnloadSound(hitSound);
    UnloadSound(dieSound);
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();

    UnloadTexture(background);
    UnloadTexture(pipeTex);
    UnloadTexture(messageTex);
    UnloadTexture(gameoverTex);
    for (int i = 0; i < LENGTH(sprites); i++) {
        UnloadTexture(sprites[i]);
    }
    CloseWindow();

    return 0;
}
