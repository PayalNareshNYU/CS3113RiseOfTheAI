#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"
#include <vector>

#define PLATFORM_COUNT 21
#define ENEMY_COUNT 3
#define BULLET_COUNT 1
#define ENEMY_BULLET_COUNT 1

struct GameState {
    Entity* player;
    Entity* platforms;
    Entity* enemies;
    Entity* bullets;
    Entity* enemyBullets;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;
bool showInstructionsPage1 = true;
bool showInstructionsPage2 = false;
bool doneWithInstructions = false;
bool gameHasStarted = false;
GLuint fontTextureID;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Rise of the AI!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize Player
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(0.50f, 2.00f, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.81f, 0);
    state.player->speed = 1.5f;
    state.player->textureID = LoadTexture("george_0.png");

    state.player->animRight = new int[4]{ 3, 7, 11, 15 };
    state.player->animLeft = new int[4]{ 1, 5, 9, 13 };
    state.player->animUp = new int[4]{ 2, 6, 10, 14 };
    state.player->animDown = new int[4]{ 0, 4, 8, 12 };

    state.player->animIndices = state.player->animRight;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols = 4;
    state.player->animRows = 4;

    state.player->height = 0.8f;
    state.player->width = 0.4f;

    state.player->jumpPower = 6.0f;

    state.player->numKills = 0;

    state.platforms = new Entity[PLATFORM_COUNT];

    fontTextureID = LoadTexture("font1.png");

    GLuint enemyBulletID = LoadTexture("ballYellowFinal.png");

    GLuint platformTextureID = LoadTexture("platformPack_tile001.png");

    GLuint platformTexture2ID = LoadTexture("platformPack_tile013.png");

    GLuint platformTexture3ID = LoadTexture("platformPack_tile016.png");

    for (int i = 0; i < 11; i++) {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTextureID;
        state.platforms[i].position = glm::vec3(-5 + i, -3.25f, 0);
    }

    for (int i = 11; i < 15; i++) {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTexture2ID;
        state.platforms[i].position = glm::vec3(-15.5 + i, 1.90f, 0);
    }

    state.platforms[15].entityType = PLATFORM;
    state.platforms[15].textureID = platformTexture3ID;
    state.platforms[15].position = glm::vec3(-1.00f, -2.25f, 0);

    state.platforms[16].entityType = PLATFORM;
    state.platforms[16].textureID = platformTexture3ID;
    state.platforms[16].position = glm::vec3(-0.50f, -1.25f, 0);

    state.platforms[17].entityType = PLATFORM;
    state.platforms[17].textureID = platformTexture3ID;
    state.platforms[17].position = glm::vec3(0.00f, -0.25f, 0);

    state.platforms[18].entityType = PLATFORM;
    state.platforms[18].textureID = platformTexture3ID;
    state.platforms[18].position = glm::vec3(0.50f, 0.75f, 0);

    state.platforms[19].entityType = PLATFORM;
    state.platforms[19].textureID = platformTexture3ID;
    state.platforms[19].position = glm::vec3(4.85f, -1.65f, 0);

    state.platforms[20].entityType = PLATFORM;
    state.platforms[20].textureID = platformTexture3ID;
    state.platforms[20].position = glm::vec3(2.80f, -0.35f, 0);

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].Update(0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL, 0);
    }

    state.enemies = new Entity[ENEMY_COUNT];
    GLuint enemyTextureID = LoadTexture("ctg.png");

    state.enemies[0].entityType = ENEMY;
    state.enemies[0].textureID = enemyTextureID;
    state.enemies[0].position = glm::vec3(3.75f, -2.25f, 0);
    state.enemies[0].speed = 0.50f;
    state.enemies[0].aiType = WAITANDGO;
    state.enemies[0].aiState = IDLE;
    state.enemies[0].height = 0.8f;
    state.enemies[0].width = 0.8f;

    state.enemies[1].entityType = ENEMY;
    state.enemies[1].textureID = enemyTextureID;
    state.enemies[1].position = glm::vec3(-4.50f, 2.90f, 0);
    state.enemies[1].speed = 1;
    state.enemies[1].aiType = WALKER;
    state.enemies[1].aiState = WALKING;
    state.enemies[1].height = 0.8f;
    state.enemies[1].width = 0.8f;

    state.enemies[2].entityType = ENEMY;
    state.enemies[2].textureID = enemyTextureID;
    state.enemies[2].position = glm::vec3(-3.40f, -2.25f, 0);
    state.enemies[2].speed = 1.0f;
    state.enemies[2].aiType = STATIONARYSHOOTER;
    state.enemies[2].aiState = SHOOTRIGHT;
    state.enemies[2].height = 0.8f;
    state.enemies[2].width = 0.8f;

    state.bullets = new Entity();
    state.enemyBullets = new Entity();

    state.bullets->entityType = BULLET;
    state.bullets->height = 0.3f;
    state.bullets->width = 0.3f;
    state.bullets->textureID = enemyBulletID;
    state.bullets->position = state.player->position;
    state.bullets->isActive = false;

    state.enemyBullets->entityType = ENEMYBULLET;
    state.enemyBullets->height = 0.3f;
    state.enemyBullets->width = 0.3f;
    state.enemyBullets->textureID = enemyBulletID;
    state.enemyBullets->position = glm::vec3(-3.40f, -2.25f, 0);
    state.enemyBullets->speed = 0.50f;
    state.enemyBullets->isActive = true;
    state.enemyBullets->shootNow = true;
}

void ProcessInput() {

    state.player->movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                // Move the player left
                break;

            case SDLK_RIGHT:
                // Move the player right
                break;

            case SDLK_SPACE:
                if (gameHasStarted == true) {
                    if (state.player->collidedBottom) {
                        state.player->jump = true;
                    }
                }
                break;

            case SDLK_RETURN:
                if (showInstructionsPage1 == true && showInstructionsPage2 == false && doneWithInstructions == false) {
                    showInstructionsPage1 = false;
                    showInstructionsPage2 = true;
                }
                else if (showInstructionsPage1 == false && showInstructionsPage2 == true && doneWithInstructions == false) {
                    showInstructionsPage2 = false;
                    doneWithInstructions = true;
                    gameHasStarted = true;
                }
                break;
            }
            break; // SDL_KEYDOWN2
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        if (gameHasStarted == true) {
            if (state.player->noMoreLeft == false) {
                state.player->movement.x = -1.0f;
                state.player->animIndices = state.player->animLeft;
            }
        }
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        if (gameHasStarted == true) {
            if (state.player->noMoreRight == false) {
                state.player->movement.x = 1.0f;
                state.player->animIndices = state.player->animRight;
            }
        }
    }
    if (keys[SDL_SCANCODE_S]) {
        if (gameHasStarted == true) {
            state.player->shootBullet(state.player, state.bullets, BULLET_COUNT);
        }
    }

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }

}

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for (int i = 0; i < text.size(); i++) {

        int index = (int)text[i];
        float offset = (size + spacing) * i;

        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;

        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            });

        texCoords.insert(texCoords.end(), {
        u, v,
        u, v + height,
        u + width, v,
        u + width, v + height,
        u + width, v,
        u, v + height,
            });
    } // end of for loop
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);

    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player->Update(FIXED_TIMESTEP, state.player, state.enemies, ENEMY_COUNT, state.bullets, state.enemyBullets, BULLET_COUNT, ENEMY_BULLET_COUNT, state.platforms, PLATFORM_COUNT);
        state.bullets->Update(FIXED_TIMESTEP, state.player, state.enemies, ENEMY_COUNT, state.bullets, state.enemyBullets, BULLET_COUNT, ENEMY_BULLET_COUNT, state.platforms, PLATFORM_COUNT);

        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].Update(FIXED_TIMESTEP, state.player, state.enemies, ENEMY_COUNT, state.bullets, state.enemyBullets, BULLET_COUNT, ENEMY_BULLET_COUNT, state.platforms, PLATFORM_COUNT);
        }

        for (int i = 0; i < ENEMY_BULLET_COUNT; i++) {
            state.enemyBullets[i].Update(FIXED_TIMESTEP, state.player, state.enemies, ENEMY_COUNT, state.bullets, state.enemyBullets, BULLET_COUNT, ENEMY_BULLET_COUNT, state.platforms, PLATFORM_COUNT);
        }

        deltaTime -= FIXED_TIMESTEP;
    }
    state.player->noMoreRight = false;
    state.player->noMoreLeft = false;

    if (state.player->numKills == ENEMY_COUNT) {
        state.player->hasWon = "Yes";
    }

    if (state.player->position.x > 4.8f) {
        state.player->noMoreRight = true;
    }

    if (state.player->position.x < -4.8f) {
        state.player->noMoreLeft = true;
    }
    accumulator = deltaTime;
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (showInstructionsPage1 == true) {
        DrawText(&program, fontTextureID, "Instructions: ", 0.8f, -0.25f, glm::vec3(-3.3f, 2.9f, 0));
        DrawText(&program, fontTextureID, "Press the RIGHT ARROW key (->) to move right", 0.40f, -0.20f, glm::vec3(-4.3f, 1.8f, 0));
        DrawText(&program, fontTextureID, "Press the LEFT ARROW key (<-) to move left", 0.40f, -0.20f, glm::vec3(-4.3f, 0.9f, 0));
        DrawText(&program, fontTextureID, "Press the SPACEBAR to jump", 0.45f, -0.15f, glm::vec3(-4.3f, 0.1f, 0));
        DrawText(&program, fontTextureID, "Press the 'S' key to shoot", 0.45f, -0.15f, glm::vec3(-4.3f, -0.7f, 0));
        DrawText(&program, fontTextureID, "You can only shoot 1 bullet at a time!", 0.40f, -0.15f, glm::vec3(-4.6f, -1.5f, 0));
        DrawText(&program, fontTextureID, "Press 'Enter' to go", 0.40f, -0.15f, glm::vec3(0.0f, -2.7f, 0));
        DrawText(&program, fontTextureID, "to the next page...", 0.40f, -0.15f, glm::vec3(0.0f, -3.2f, 0));
    }
    if (showInstructionsPage2 == true) {
        DrawText(&program, fontTextureID, "Instructions: ", 0.8f, -0.25f, glm::vec3(-3.3f, 2.9f, 0));
        DrawText(&program, fontTextureID, "Your goal:", 0.47f, -0.15f, glm::vec3(-4.3f, 1.8f, 0));
        DrawText(&program, fontTextureID, "Shoot all of the enemies", 0.47f, -0.15f, glm::vec3(-4.3f, 0.5f, 0));
        DrawText(&program, fontTextureID, "...before they get to you", 0.47f, -0.15f, glm::vec3(-4.3f, -0.8f, 0));
        DrawText(&program, fontTextureID, "Press 'Enter' to", 0.4f, -0.15f, glm::vec3(0.4f, -2.7f, 0));
        DrawText(&program, fontTextureID, "begin the game...", 0.4f, -0.15f, glm::vec3(0.4f, -3.2f, 0));
    }

    if (doneWithInstructions == true) {
        for (int i = 0; i < PLATFORM_COUNT; i++) {
            state.platforms[i].Render(&program);
        }

        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].Render(&program);
        }

        state.bullets->Render(&program);

        for (int i = 0; i < ENEMY_BULLET_COUNT; i++) {
            state.enemyBullets[i].Render(&program);
        }

        if (state.player->hasWon == "Yes") {
            DrawText(&program, fontTextureID, "YOU WIN!", 0.8f, -0.25f, glm::vec3(-1.9f, 3.0f, 0));
        }
        if (state.player->hasWon == "No") {
            DrawText(&program, fontTextureID, "GAME OVER!", 0.8f, -0.25f, glm::vec3(-2.2f, 3.0f, 0));
        }

        state.player->Render(&program);
    }
    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}