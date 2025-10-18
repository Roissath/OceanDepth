
//
// Created by maxim on 07/10/2025.
// Implementation des créatures
//

#include "creatures.h"
#include <stdlib.h>
#include <string.h>

// Initialise stats selon le type (interne)
static void init_stats_for_type(Creature *c, CreatureType type) {
	c->type = type;
	switch (type) {
		case CREATURE_TYPE_FISH:
			// Petit monstre faible, rapide à tuer
			c->max_hp = 5; c->hp = 5; c->attack = 1; c->defence = 0; break;
		case CREATURE_TYPE_CRAB:
			// Crabe, un peu plus de PV et de défense
			c->max_hp = 8; c->hp = 8; c->attack = 2; c->defence = 1; break;
		case CREATURE_TYPE_SHARK:
			// Requin, prédateur plus dangereux
			c->max_hp = 15; c->hp = 15; c->attack = 4; c->defence = 2; break;
		case CREATURE_TYPE_BOSS:
			// Boss de niveau, beaucoup de PV et d'attaque
			c->max_hp = 40; c->hp = 40; c->attack = 7; c->defence = 3; break;
		default:
			// Valeurs par défaut minimales
			c->max_hp = 1; c->hp = 1; c->attack = 0; c->defence = 0; break;
	}
}

// Initialise la liste
void creatures_init(CreatureList *list) {
	if (!list) return; list->head = NULL; list->next_id = 1;
}

// Crée et insère en tête
Creature *creature_new(CreatureList *list, CreatureType type, int x, int y) {
	if (!list) return NULL;
	Creature *c = malloc(sizeof(Creature));
	if (!c) return NULL;
	memset(c, 0, sizeof(*c));
	c->id = list->next_id++;
	c->x = x; c->y = y;
	init_stats_for_type(c, type);
	c->alive = 1;
	c->next = list->head;
	list->head = c;
	return c;
}

// Supprime une créature par id
void creature_free(CreatureList *list, CreatureId id) {
	if (!list) return;
	Creature *prev = NULL;
	Creature *cur = list->head;
	while (cur) {
		if (cur->id == id) {
			// Retirer du chainage
			if (prev) prev->next = cur->next; else list->head = cur->next;
			free(cur);
			return;
		}
		prev = cur;
		cur = cur->next;
	}
}

// Retourne la créature par id
Creature *creature_get_by_id(CreatureList *list, CreatureId id) {
	if (!list) return NULL;
	Creature *c = list->head;
	while (c) {
		if (c->id == id) return c;
		c = c->next;
	}
	return NULL;
}

// Retourne la créature vivante à (x,y) ou NULL
Creature *creature_at(CreatureList *list, int x, int y) {
	if (!list) return NULL;
	Creature *c = list->head;
	while (c) {
		if (c->x == x && c->y == y && c->alive) return c;
		c = c->next;
	}
	return NULL;
}

// Déplace (sans vérification)
void creature_move(Creature *c, int new_x, int new_y) {
	if (!c) return;
	c->x = new_x;
	c->y = new_y;
}

// Applique dégâts (retourne 1 si mort)
int creature_damage(Creature *c, int dmg) {
	if (!c || !c->alive) return 0; // rien à faire
	int effective = dmg - c->defence;
	if (effective < 0) effective = 0;
	c->hp -= effective;
	if (c->hp <= 0) {
		c->hp = 0;
		c->alive = 0;
		return 1; // mort
	}
	return 0; // toujours vivant
}

// Attaque: applique attack et retourne dégâts bruts
int creature_hit(Creature *attacker, Creature *defender) {
	if (!attacker || !defender) return 0;
	int dmg = attacker->attack;
	creature_damage(defender, dmg);
	return dmg;
}

// Vide la liste
void creatures_clear(CreatureList *list) {
	if (!list) return;
	Creature *c = list->head;
	while (c) {
		Creature *n = c->next;
		free(c);
		c = n;
	}
	list->head = NULL;
}

// Sauvegarde (texte, 1 créature par ligne)
int creatures_save(FILE *out, CreatureList *list) {
	if (!out || !list) return -1;
	Creature *c = list->head;
	while (c) {
		// Écrire toujours l'id pour garder compatibilité de chargement
		fprintf(out, "%u %d %d %d %d %d %d %d\n",
				(unsigned)c->id, (int)c->type, c->x, c->y, c->hp, c->max_hp, c->attack, c->defence);
		c = c->next;
	}
	return 0;
}

// Chargement (texte)
int creatures_load(FILE *in, CreatureList *list) {
	if (!in || !list) return -1;
	// On vide la liste avant chargement pour repartir d'une base propre
	creatures_clear(list);
	unsigned id=0; int type=0, x=0, y=0, hp=0, max_hp=0, attack=0, defence=0;
	while (fscanf(in, "%u %d %d %d %d %d %d %d", &id, &type, &x, &y, &hp, &max_hp, &attack, &defence) == 8) {
		Creature *c = malloc(sizeof(Creature));
		if (!c) {
			// échec d'allocation : libérer ce qui a été chargé et signaler l'erreur
			creatures_clear(list);
			return -1;
		}
		c->id = (CreatureId)id;
		c->type = (CreatureType)type;
		c->x = x; c->y = y; c->hp = hp; c->max_hp = max_hp; c->attack = attack; c->defence = defence; c->alive = (hp>0)?1:0;
		// Insérer en tête (ordre inversé par rapport au fichier)
		c->next = list->head;
		list->head = c;
		// Mettre à jour next_id pour éviter les collisions futures d'identifiants
		if (c->id >= list->next_id) list->next_id = c->id + 1;
	}
	return 0;
}

