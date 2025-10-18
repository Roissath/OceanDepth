/* structure du joueur
 * points de vie
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "joueur.h"

static int rand_range(int min, int max) {
    if (min > max) {
        int tmp = min; min = max; max = tmp;
    }
    return (rand() % (max - min + 1)) + min;
}

/* Initialise un Plongeur; retourne le pointeur passé (ou NULL si NULL) */
Plongeur *initialiser_joueur(Plongeur *plongeur, const char *nom, const char *prenom, int points_de_vie, int points_de_vie_max, int niveau_oxygene, int niveau_oxygene_max, int niveau_fatigue, int perles) {
    if (!plongeur) return NULL;
    
    // Copier les informations personnelles
    if (nom) {
        strncpy(plongeur->nom, nom, sizeof(plongeur->nom) - 1);
        plongeur->nom[sizeof(plongeur->nom) - 1] = '\0';
    } else {
        strcpy(plongeur->nom, "Anonyme");
    }
    
    if (prenom) {
        strncpy(plongeur->prenom, prenom, sizeof(plongeur->prenom) - 1);
        plongeur->prenom[sizeof(plongeur->prenom) - 1] = '\0';
    } else {
        strcpy(plongeur->prenom, "Plongeur");
    }
    
    // Date actuelle (format simple)
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    snprintf(plongeur->date_creation, sizeof(plongeur->date_creation), 
             "%02d/%02d/%04d", tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
    
    // Statistiques de jeu
    plongeur->points_de_vie = points_de_vie;
    plongeur->points_de_vie_max = points_de_vie_max;
    plongeur->niveau_oxygene = niveau_oxygene;
    plongeur->niveau_oxygene_max = niveau_oxygene_max;
    plongeur->niveau_fatigue = niveau_fatigue;
    plongeur->perles = perles;
    plongeur->tours_paralyse = 0;
    plongeur->defense = 1; // Défense de base
    plongeur->avancement = 0; // Début du jeu
    plongeur->temps_jeu = 0;
    
    return plongeur;
}

int gestion_fatigue(Plongeur *plongeur, int action) {
    int max_attack = 0;
 if (plongeur->niveau_fatigue == 0 || plongeur->niveau_fatigue == 1) {
   max_attack = 3;
   return max_attack;
 } else if (plongeur->niveau_fatigue == 2 || plongeur->niveau_fatigue == 3) {
   max_attack = 2;
   return max_attack;
 } else if (plongeur->niveau_fatigue == 4 || plongeur->niveau_fatigue == 5) {
   max_attack = 1;
   return max_attack;
 }
 return -1;
}

int consommation_oxygene(Plongeur *plongeur, int action, int profondeur) {

    if (action == 1) { /* normale */
        if (profondeur < 5) {
            plongeur->niveau_oxygene -= rand_range(2, 4); /* 2..4 */
        } else if (profondeur < 10 && profondeur >= 5) {
            plongeur->niveau_oxygene -= rand_range(5, 8); /* 5..8 */
        }
    }

    if (plongeur->niveau_oxygene <= 10) {
        printf(" ALERTE CRITIQUE\n");
    }

    if (plongeur->niveau_oxygene <= 0) {
        plongeur->niveau_oxygene = 0;
        plongeur->points_de_vie -= 5;
    }

    return plongeur->niveau_oxygene;
}

int calcul_degats_variation(int attaque_min, int attaque_max, int defense_creature, int bonus_arme) {
    if (attaque_min > attaque_max) {
        int tmp = attaque_min; attaque_min = attaque_max; attaque_max = tmp;
    }

    int degats_base = rand() % (attaque_max - attaque_min + 1) + attaque_min;
    degats_base += bonus_arme;
    int degats = degats_base - defense_creature;
    if (degats < 1) degats = 1;
    return degats;
}

/* Sauvegarder les données du joueur dans un fichier */
int sauvegarder_joueur(FILE *out, Plongeur *plongeur) {
    if (!out || !plongeur) return -1;
    
    // Sauvegarder les informations personnelles
    fprintf(out, "JOUEUR_INFO:%s:%s:%s:%d:%d\n",
           plongeur->nom, plongeur->prenom, plongeur->date_creation,
           plongeur->avancement, plongeur->temps_jeu);
    
    // Sauvegarder les statistiques de jeu
    fprintf(out, "PLONGEUR:%d:%d:%d:%d:%d:%d:%d:%d\n",
           plongeur->points_de_vie, plongeur->points_de_vie_max,
           plongeur->niveau_oxygene, plongeur->niveau_oxygene_max,
           plongeur->niveau_fatigue, plongeur->perles,
           plongeur->defense, plongeur->tours_paralyse);
    
    return 0;
}

/* Charger les données du joueur depuis un fichier */
int charger_joueur(FILE *in, Plongeur *plongeur) {
    if (!in || !plongeur) return -1;
    
    char ligne[256];
    
    // Lire les informations personnelles
    if (fgets(ligne, sizeof(ligne), in)) {
        char nom[50], prenom[50], date[20];
        int avancement, temps_jeu;
        if (sscanf(ligne, "JOUEUR_INFO:%49[^:]:%49[^:]:%19[^:]:%d:%d",
                  nom, prenom, date, &avancement, &temps_jeu) == 5) {
            strcpy(plongeur->nom, nom);
            strcpy(plongeur->prenom, prenom);
            strcpy(plongeur->date_creation, date);
            plongeur->avancement = avancement;
            plongeur->temps_jeu = temps_jeu;
        }
    }
    
    // Lire les statistiques de jeu
    if (fgets(ligne, sizeof(ligne), in)) {
        int pv, pv_max, ox, ox_max, fatigue, perles, defense, paralyse;
        if (sscanf(ligne, "PLONGEUR:%d:%d:%d:%d:%d:%d:%d:%d",
                  &pv, &pv_max, &ox, &ox_max, &fatigue, &perles, &defense, &paralyse) == 8) {
            plongeur->points_de_vie = pv;
            plongeur->points_de_vie_max = pv_max;
            plongeur->niveau_oxygene = ox;
            plongeur->niveau_oxygene_max = ox_max;
            plongeur->niveau_fatigue = fatigue;
            plongeur->perles = perles;
            plongeur->defense = defense;
            plongeur->tours_paralyse = paralyse;
            return 0;
        }
    }
    
    return -1;
}

/* Mettre à jour l'avancement du joueur basé sur l'exploration */
void mettre_a_jour_avancement(Plongeur *plongeur, int zones_explorees, int total_zones) {
    if (!plongeur || total_zones <= 0) return;
    
    plongeur->avancement = (zones_explorees * 100) / total_zones;
    if (plongeur->avancement > 100) plongeur->avancement = 100;
    
    // Incrémenter le temps de jeu (simulé)
    plongeur->temps_jeu += 1;
}