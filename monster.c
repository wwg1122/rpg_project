#include "monster.h"
#include <stdio.h>

Enemy *create_enemy(char *name, int hp, int atk)
{
	Enemy *new_enemy = (Enemy*)malloc(sizeof(Enemy));

	if (new_enemy == NULL)
	{
		printf("메모리 할당 실패!\n");
		return NULL;
	}

	new_enemy -> name = name;
	new_enemy -> hp = hp;
	new_enemy->max_hp = hp;
        new_enemy->atk = atk;
        new_enemy->next = NULL;

	printf("[시스템] 몬스터 '%s' 소환 완료\n", name);
	return new_enemy;
}

void free_enemy(Enemy *target)
	{
		if (target != NULL)
		{
			printf("[시스템] 몬스터 '%s' 메모리 해제\n", target ->name);
			free(target);
		}
	}

