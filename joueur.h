/* joueur.h
 * Déclarations publiques pour le plongeur (Plongeur).
 */

#ifndef OCEAN_DEPTH_JOUEUR_H
#define OCEAN_DEPTH_JOUEUR_H

#include <stdint.h>

typedef struct Plongeur {
    char nom[50];           // Nom du joueur
    char prenom[50];        // Prénom du joueur
    char date_creation[20]; // Date de création de la partie 
    int points_de_vie;
    int points_de_vie_max;
    int niveau_oxygene;
    int niveau_oxygene_max;
    int niveau_fatigue; // 0..5
    int perles;
    int tours_paralyse; // nombre de tours paralysé (réduit attaques)
    int defense; // bonus défensif (combinaison)
    int avancement;     // Pourcentage d'avancement du jeu (0-100)
    int temps_jeu;      // Temps de jeu en minutes
} Plongeur;

/* prototypes existants (implémentés dans joueur.c) */
Plongeur *initialiser_joueur(Plongeur *plongeur, const char *nom, const char *prenom, int points_de_vie, int points_de_vie_max, int niveau_oxygene, int niveau_oxygene_max, int niveau_fatigue, int perles);
int gestion_fatigue(Plongeur *plongeur, int action);
int consommation_oxygene(Plongeur *plongeur, int action, int profondeur);
int calcul_degats_variation(int attaque_min, int attaque_max, int defense_creature, int bonus_arme);

/* nouvelles fonctions de sauvegarde/chargement */
int sauvegarder_joueur(FILE *out, Plongeur *plongeur);
int charger_joueur(FILE *in, Plongeur *plongeur);
void mettre_a_jour_avancement(Plongeur *plongeur, int zones_explorees, int total_zones);

#endif // OCEAN_DEPTH_JOUEUR_H
