/* carte.h
 * Système de cartographie des océans - Étape 7
 * Gestion des zones, navigation, exploration
 */

#ifndef OCEAN_DEPTH_CARTE_H
#define OCEAN_DEPTH_CARTE_H

#include <stdio.h>
#include <stdint.h>

/* Forward declarations */
typedef struct Plongeur Plongeur;
typedef struct CreatureList CreatureList;

/* Types de zones océaniques */
typedef enum {
    ZONE_SURFACE = 0,        // Sauvegarde, boutique, repos complet
    ZONE_RECIFS,            // Créatures faciles, équipement de base
    ZONE_EPAVES,            // Trésors, créatures moyennes, équipements rares
    ZONE_FORET_ALGUES,      // Labyrinthique, créatures qui se cachent
    ZONE_GROTTES,           // Sécurisées, cristaux d'oxygène, sauvegarde
    ZONE_FOSSES,            // Créatures légendaires, équipements exceptionnels
    ZONE_VIDE,              // Zone non explorée ou inaccessible
    ZONE_INCONNUE           // Zone à découvrir
} TypeZone;

/* Conditions environnementales */
typedef struct {
    int courant;            // Force du courant (0=faible, 5=violent)
    int visibilite;         // Visibilité (0=nulle, 10=parfaite)
    int temperature;        // Température en °C
    int danger_level;       // Niveau de danger (0=sûr, 10=mortel)
} ConditionsZone;

/* Une zone de la carte */
typedef struct {
    TypeZone type;
    int profondeur;         // Profondeur en mètres (0 = surface)
    int nb_creatures;       // Nombre de créatures dans cette zone
    int a_tresor;           // 1 si la zone contient un trésor
    int exploree;           // 1 si la zone a été visitée
    int debroquee;          // 1 si la zone est accessible
    ConditionsZone conditions;
    char nom[50];           // Nom descriptif de la zone
} Zone;

/* Position sur la carte */
typedef struct {
    int x, y;               // Coordonnées sur la grille
    TypeZone zone_actuelle; // Type de la zone où se trouve le joueur
} Position;

/* Carte océanique (grille 4x4 comme dans le sujet) */
#define TAILLE_CARTE 4
typedef struct {
    Zone grille[TAILLE_CARTE][TAILLE_CARTE];
    Position position_joueur;
    int zones_decouvertes;  // Nombre total de zones découvertes
} CarteOceanique;

/* Résultat d'un déplacement */
typedef enum {
    DEPLACEMENT_OK = 0,
    DEPLACEMENT_ZONE_BLOQUEE,
    DEPLACEMENT_HORS_LIMITES,
    DEPLACEMENT_EQUIPEMENT_REQUIS,
    DEPLACEMENT_OXYGENE_INSUFFISANT
} ResultatDeplacement;

/* Prototypes des fonctions */
int initialiser_carte(CarteOceanique *carte, Plongeur *joueur);
Zone* get_zone_actuelle(CarteOceanique *carte);
ResultatDeplacement se_deplacer(CarteOceanique *carte, int nouveau_x, int nouveau_y, Plongeur *plongeur);
void explorer_zone_actuelle(CarteOceanique *carte, Plongeur *joueur);
void debloquer_zones_adjacentes(CarteOceanique *carte, int x, int y);
int peut_se_deplacer_vers(CarteOceanique *carte, Plongeur *joueur, int x, int y);
void afficher_carte_complete(CarteOceanique *carte);
void afficher_carte_detaillee(CarteOceanique *carte);
int sauvegarder_carte(FILE *out, CarteOceanique *carte);
int charger_carte(FILE *in, CarteOceanique *carte);

/* Fonctions utilitaires */
const char* nom_type_zone(TypeZone type);
const char* symbole_zone(TypeZone type);
int calculer_distance(int x1, int y1, int x2, int y2);
int consommation_oxygene_deplacement(int distance, int profondeur);

#endif // OCEAN_DEPTH_CARTE_H