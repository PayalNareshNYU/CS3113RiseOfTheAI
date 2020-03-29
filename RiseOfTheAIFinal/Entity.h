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

enum EntityType { PLAYER, PLATFORM, ENEMY, BULLET, ENEMYBULLET };

enum AIType { WALKER, WAITANDGO, STATIONARYSHOOTER };
enum AIState { IDLE, WALKING, MOVE_RIGHT, MOVE_LEFT, SHOOTRIGHT, SHOOTLEFT };

class Entity {
public:
    EntityType entityType;
    AIType aiType;
    AIState aiState;

    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 acceleration;
    glm::vec3 velocity;

    float width = 1;
    float height = 1;

    bool jump = false;
    float jumpPower = 0;

    float speed;

    GLuint textureID;
    int bulletNumber = 0;
    glm::mat4 modelMatrix;

    int* animRight = NULL;
    int* animLeft = NULL;
    int* animUp = NULL;
    int* animDown = NULL;

    int* animIndices = NULL;
    int animFrames = 0;
    int animIndex = 0;
    float animTime = 0;
    int animCols = 0;
    int animRows = 0;

    bool isActive = true;
    std::string hasWon;
    int numKills = 0;
    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;

    bool shootNow = true;

    bool gunReady = true;

    bool noMoreRight = false;
    bool noMoreLeft = false;

    Entity();

    bool CheckCollision(Entity* other);
    void CheckCollisionsY(Entity* objects, int objectCount);
    void CheckCollisionsX(Entity* objects, int objectCount);
    void Update(float deltaTime, Entity* player, Entity* enemies, int enemyCount, Entity* bullets, Entity* enemyBullets, int bulletCount, int enemyBulletCount, Entity* platforms, int platformCount);
    void Render(ShaderProgram* program);
    void DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index);
    void AI(Entity* player, Entity* enemyBullets, int enemyBulletCount, Entity* platforms, int platformCount);
    void AIWalker();
    void AIWaitAndGo(Entity* player);
    void CheckCollisionsXEnemy(Entity* enemies, int enemyCount, Entity* player);
    void CheckCollisionsYEnemy(Entity* enemies, int enemyCount, Entity* player);
    void shootBullet(Entity* player, Entity* bullets, int bulletCount);
    void AIShooter(Entity* player, Entity* enemyBullets, int enemyBulletCount, Entity* platforms, int platformCount);
};