/* combat.h
 * Déclarations pour le système de combat
 */

#ifndef OCEAN_DEPTH_COMBAT_H
#define OCEAN_DEPTH_COMBAT_H

#include "creatures.h"
#include "joueur.h"

/* Fonctions de combat */
void tour_de_combat(CreatureList *liste, Plongeur *plongeur, int profondeur);
int phase_attaque_creatures(Plongeur *plongeur, CreatureList *creatures);

#endif /* OCEAN_DEPTH_COMBAT_H */