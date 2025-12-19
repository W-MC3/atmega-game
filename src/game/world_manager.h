//
// Created by Christian ten Brinke on 18/12/2025.
//

#ifndef ATMEGA_WORLD_MANAGER_H
#define ATMEGA_WORLD_MANAGER_H

#include "gfx/gfx.h"

typedef enum {
    SCENE_HOME,
    SCENE_GAME,
    SCENE_GAMEOVER
} scene_t;

void world_manager_init(void);
void load_scene(scene_t scene);

#endif //ATMEGA_WORLD_MANAGER_H