#ifndef MONSTER_H
#define MONSTER_H

#include "common.h"

Enemy *create_enemy(char *name, int hp, int atk);

void free_enemy(Enemy *target);

#endif
