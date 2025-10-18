/* creatures.h
 * Types et API pour les créatures (structure simple, liste chaînée).
 */

#ifndef OCEAN_DEPTH_CREATURES_H
#define OCEAN_DEPTH_CREATURES_H

#include <stdio.h>
#include <stdint.h>

typedef uint32_t CreatureId;

typedef enum {
    CREATURE_TYPE_NONE = 0,
    CREATURE_TYPE_FISH,
    CREATURE_TYPE_SHARK,
    CREATURE_TYPE_CRAB,
    CREATURE_TYPE_BOSS
} CreatureType;

typedef struct Creature {
    CreatureId id;
    CreatureType type;
    int x,y;    // position
    int hp,max_hp;
    int attack,defence;
    int alive;  // 0/1
    struct Creature *next;
} Creature;

typedef struct CreatureList { Creature *head; CreatureId next_id; } CreatureList;



void creatures_init(CreatureList *list);
Creature *creature_new(CreatureList *list, CreatureType type, int x, int y);
void creature_free(CreatureList *list, CreatureId id);
Creature *creature_get_by_id(CreatureList *list, CreatureId id);
Creature *creature_at(CreatureList *list, int x, int y);
void creature_move(Creature *c, int new_x, int new_y);
int creature_damage(Creature *c, int dmg);
int creature_hit(Creature *attacker, Creature *defender);
void creatures_clear(CreatureList *list);
int creatures_save(FILE *out, CreatureList *list);
int creatures_load(FILE *in, CreatureList *list);

/* Note: plus d'aliases — utiliser les nouveaux noms publics. */

#endif // OCEAN_DEPTH_CREATURES_H
