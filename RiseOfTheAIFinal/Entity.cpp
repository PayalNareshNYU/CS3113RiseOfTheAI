#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;

    modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity* other) {
    if (isActive == false || other->isActive == false) return false;

    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (xdist < 0 && ydist < 0) return true;

    return false;
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
            }
        }
    }
}

void Entity::CheckCollisionsXEnemy(Entity* enemies, int enemyCount, Entity* player)
{
    for (int i = 0; i < enemyCount; i++)
    {
        Entity* object = &enemies[i];
        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
            }
            isActive = false;
            player->hasWon = "No";
        }
    }
}

void Entity::CheckCollisionsYEnemy(Entity* enemies, int enemyCount, Entity* player)
{
    for (int i = 0; i < enemyCount; i++)
    {
        Entity* object = &enemies[i];
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
            }
            isActive = false;
            player->hasWon = "No";
        }
    }
}

void Entity::AIWalker() {
    switch (aiState) {
    case WALKING:
        speed = 1.0f;
        movement = glm::vec3(1, 0, 0);
        if (position.x <= -4.90f) {
            aiState = MOVE_RIGHT;
        }
        else if (position.x >= -3.25f) {
            aiState = MOVE_LEFT;
        }
        break;
    case MOVE_RIGHT:
        movement = glm::vec3(1, 0, 0);
        if (position.x >= -3.25f) {
            aiState = MOVE_LEFT;
        }
        break;
    case MOVE_LEFT:
        movement = glm::vec3(-1, 0, 0);
        if (position.x <= -4.70f) {
            aiState = MOVE_RIGHT;
        }
        break;
    }
}

void Entity::AIWaitAndGo(Entity* player) {
    switch (aiState) {
    case IDLE:
        if (glm::distance(position, player->position) < 3.0f) {
            aiState = WALKING;
        }
        break;
    case WALKING:
        if (player->position.x < position.x) {
            movement = glm::vec3(-1, 0, 0);
        }
        else {
            movement = glm::vec3(1, 0, 0);
        }
        break;
    }
}

void Entity::AIShooter(Entity* player, Entity* enemyBullets, int enemyBulletCount, Entity* platforms, int platformCount) {
    switch (aiState) {
    case SHOOTRIGHT:
        enemyBullets->isActive = true;
        enemyBullets->movement = glm::vec3(1, 0, 0);
        enemyBullets->speed = 0.5f;

        if (glm::distance(enemyBullets->position, player->position) < 0.5f) {
            enemyBullets->isActive = false;
            player->isActive = false;
            player->hasWon = "No";
            aiState = SHOOTLEFT;
        }

        for (int i = 0; i < platformCount; i++) {
            if (glm::distance(enemyBullets->position, platforms[i].position) < 0.80f) {
                enemyBullets->isActive = false;
                enemyBullets->position = glm::vec3(-3.40f, -2.25f, 0);
                aiState = SHOOTLEFT;
            }
        }
        break;
    case SHOOTLEFT:
        enemyBullets->isActive = true;
        enemyBullets->movement = glm::vec3(-1, 0, 0);
        enemyBullets->speed = 0.5f;

        if (glm::distance(enemyBullets->position, player->position) < 0.80f) {
            enemyBullets->isActive = false;
            player->isActive = false;
            player->hasWon = "No";
            aiState = SHOOTRIGHT;
        }

        for (int i = 0; i < platformCount; i++) {
            if (glm::distance(enemyBullets->position, platforms[i].position) < 1.0f || enemyBullets->position.x < -5.0f) {
                enemyBullets->isActive = false;
                enemyBullets->position = glm::vec3(-3.40f, -2.25f, 0);
                aiState = SHOOTRIGHT;
            }
        }
        break;
    case IDLE:
        if (enemyBullets->isActive == false) {
            aiState = SHOOTRIGHT;
        }
        break;
    }
}

void Entity::AI(Entity* player, Entity* enemyBullets, int enemyBulletCount, Entity* platforms, int platformCount) {
    switch (aiType) {
    case WALKER:
        AIWalker();
        break;

    case WAITANDGO:
        AIWaitAndGo(player);
        break;

    case STATIONARYSHOOTER:
        AIShooter(player, enemyBullets, enemyBulletCount, platforms, platformCount);
        break;
    }
}

void Entity::shootBullet(Entity* player, Entity* bullets, int bulletCount) {
    if (isActive == false) return;

    if (player->gunReady == true) {
        player->gunReady = false;
        bullets->isActive = true;
        bullets->position = position;
        if (animIndices == animLeft) {
            bullets->movement.x = -0.5f;
        }
        else {
            bullets->movement.x = 0.5f;
        }
        bullets->speed = 2.5f;
    }
}

void Entity::Update(float deltaTime, Entity* player, Entity* enemies, int enemyCount, Entity* bullets, Entity* enemyBullets, int bulletCount, int enemyBulletCount, Entity* platforms, int platformCount)
{
    if (isActive == false) return;

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (entityType == ENEMY) {
        AI(player, enemyBullets, enemyBulletCount, platforms, platformCount);
    }

    if (entityType == PLAYER) {
        CheckCollisionsXEnemy(enemies, enemyCount, player);
        CheckCollisionsYEnemy(enemies, enemyCount, player);
    }

    if (entityType == BULLET) {
        velocity.x = movement.x * speed;
        velocity += acceleration * deltaTime;

        position.x += velocity.x * deltaTime;

        if (CheckCollision(&enemies[2])) {
            enemyBullets[0].isActive = false;
        }

        for (int i = 0; i < enemyCount; i++) {
            if (CheckCollision(&enemies[i])) {
                player->gunReady = true;
                enemies[i].isActive = false;
                isActive = false;
                player->numKills += 1;
            }
        }
        for (int i = 0; i < platformCount; i++) {
            if (CheckCollision(&platforms[i])) {
                player->gunReady = true;
                isActive = false;
            }
        }

        if (position.x < -5.0f || position.x > 5.0f) {
            player->gunReady = true;
        }

    }
    if (entityType == ENEMYBULLET) {
        velocity.x = movement.x * speed;
        velocity += acceleration * deltaTime;
        position.x += velocity.x * deltaTime;
    }

    if (animIndices != NULL) {
        if (glm::length(movement) != 0) {
            animTime += deltaTime;

            if (animTime >= 0.25f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }
        }
        else {
            animIndex = 0;
        }
    }

    if (jump) {
        jump = false;

        velocity.y += jumpPower;
    }

    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;

    position.y += velocity.y * deltaTime; // Move on Y 
    CheckCollisionsY(platforms, platformCount);// Fix if needed

    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(platforms, platformCount);// Fix if needed

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;

    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;

    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v };

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram* program) {

    if (isActive == false) return;

    program->SetModelMatrix(modelMatrix);

    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}