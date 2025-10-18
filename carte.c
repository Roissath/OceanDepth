/* carte.c
 * Implémentation du système de cartographie océanique
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "carte.h"
#include "joueur.h"

/* Initialiser la carte océanique */
int initialiser_carte(CarteOceanique *carte, Plongeur *joueur) {
    if (!carte || !joueur) return -1;
    
    // Initialiser la grille
    for (int y = 0; y < TAILLE_CARTE; y++) {
        for (int x = 0; x < TAILLE_CARTE; x++) {
            Zone *zone = &carte->grille[y][x];
            zone->exploree = 0;
            zone->debroquee = 0;
            zone->nb_creatures = 0;
            
            // Définir les types et profondeurs selon la position
            if (y == 0) {
                zone->profondeur = 0; // Surface
                if (x == 0 || x == 3) {
                    zone->type = ZONE_SURFACE;
                } else {
                    zone->type = ZONE_VIDE;
                }
            } else if (y == 1) {
                zone->profondeur = 50;
                switch (x) {
                    case 0: zone->type = ZONE_RECIFS; break;
                    case 1: zone->type = ZONE_EPAVES; break;
                    case 2: zone->type = ZONE_FORET_ALGUES; break;
                    case 3: zone->type = ZONE_GROTTES; break;
                }
            } else if (y == 2) {
                zone->profondeur = 150;
                zone->type = ZONE_FOSSES;
            } else {
                zone->profondeur = 300;
                zone->type = ZONE_INCONNUE;
            }
            
            // Définir le nombre de créatures
            zone->nb_creatures = (y > 0) ? (1 + rand() % 4) : 0;
        }
    }
    
    // Position initiale du joueur - BASE DE SURFACE
    carte->position_joueur.x = 0;
    carte->position_joueur.y = 0; // Base de surface (0m)
    carte->position_joueur.zone_actuelle = ZONE_SURFACE;
    carte->zones_decouvertes = 1;
    
    // Débloquer et explorer la base de départ (zone de sécurité)
    Zone *zone_depart = &carte->grille[0][0];
    zone_depart->debroquee = 1;
    zone_depart->exploree = 1;
    
    // Débloquer le récif adjacent pour permettre l'exploration
    carte->grille[1][0].debroquee = 1;
    
    return 0;
}

/* Obtenir la zone actuelle */
Zone* get_zone_actuelle(CarteOceanique *carte) {
    if (!carte) return NULL;
    return &carte->grille[carte->position_joueur.y][carte->position_joueur.x];
}

/* Vérifier si on peut se déplacer vers une position */
int peut_se_deplacer_vers(CarteOceanique *carte, Plongeur *joueur, int x, int y) {
    if (!carte || !joueur) return 0;
    
    // Vérifier les limites
    if (x < 0 || x >= TAILLE_CARTE || y < 0 || y >= TAILLE_CARTE) return 0;
    
    Zone *zone_cible = &carte->grille[y][x];
    
    // Vérifier si la zone est débloquée
    if (!zone_cible->debroquee) return 0;
    
    // Vérifier l'équipement pour la profondeur
    if (zone_cible->profondeur > 50 && joueur->defense < 2) return 0;
    
    return 1;
}

/* Se déplacer vers une nouvelle position */
ResultatDeplacement se_deplacer(CarteOceanique *carte, int nouveau_x, int nouveau_y, Plongeur *plongeur) {
    if (!carte || !plongeur) return DEPLACEMENT_HORS_LIMITES;
    
    // Vérifier les limites
    if (nouveau_x < 0 || nouveau_x >= TAILLE_CARTE || nouveau_y < 0 || nouveau_y >= TAILLE_CARTE) {
        return DEPLACEMENT_HORS_LIMITES;
    }
    
    Zone *zone_cible = &carte->grille[nouveau_y][nouveau_x];
    
    // Vérifier si la zone est débloquée
    if (!zone_cible->debroquee) {
        return DEPLACEMENT_ZONE_BLOQUEE;
    }
    
    // Vérifier l'équipement pour la profondeur
    if (zone_cible->profondeur > 50 && plongeur->defense < 2) {
        return DEPLACEMENT_EQUIPEMENT_REQUIS;
    }
    
    // Effectuer le déplacement
    carte->position_joueur.x = nouveau_x;
    carte->position_joueur.y = nouveau_y;
    
    // Découvrir la zone si ce n'est pas déjà fait
    if (!zone_cible->exploree) {
        printf("🗺️ Nouvelle zone découverte : %s !\n", nom_type_zone(zone_cible->type));
        carte->zones_decouvertes++;
    }
    
    return DEPLACEMENT_OK;
}

/* Explorer la zone actuelle */
void explorer_zone_actuelle(CarteOceanique *carte, Plongeur *joueur) {
    if (!carte || !joueur) return;
    
    Zone *zone = get_zone_actuelle(carte);
    if (!zone) return;
    
    if (zone->exploree) {
        printf("🗺️ Cette zone a déjà été explorée.\n");
        printf("🧭 Zones adjacentes déjà débloquées.\n");
        return;
    }
    
    // Explorer la zone
    zone->exploree = 1;
    printf("🔍 Vous explorez la zone %s...\n", nom_type_zone(zone->type));
    
    // Mettre à jour l'avancement du joueur
    mettre_a_jour_avancement(joueur, carte->zones_decouvertes, TAILLE_CARTE * TAILLE_CARTE);
    
    // Débloquer les zones adjacentes
    debloquer_zones_adjacentes(carte, carte->position_joueur.x, carte->position_joueur.y);
}

/* Débloquer les zones adjacentes */
void debloquer_zones_adjacentes(CarteOceanique *carte, int x, int y) {
    if (!carte) return;
    
    int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}}; // Haut, Bas, Gauche, Droite
    int nouvelles_zones = 0;
    
    for (int i = 0; i < 4; i++) {
        int nx = x + directions[i][0];
        int ny = y + directions[i][1];
        
        if (nx >= 0 && nx < TAILLE_CARTE && ny >= 0 && ny < TAILLE_CARTE) {
            Zone *zone_adj = &carte->grille[ny][nx];
            if (!zone_adj->debroquee) {
                zone_adj->debroquee = 1;
                nouvelles_zones++;
            }
        }
    }
    
    if (nouvelles_zones > 0) {
        printf("🗝️ Nouvelle zone accessible !\n");
    }
}

/* Afficher la carte complète */
void afficher_carte_complete(CarteOceanique *carte) {
    if (!carte) return;
    
    printf("\n CARTOGRAPHIE OCÉANIQUE - SECTEUR PACIFIQUE\n");
    printf(" ┌─────────┬─────────┬─────────┬─────────┬──────────┐\n");
    
    for (int y = 0; y < TAILLE_CARTE; y++) {
        // Ligne du contenu
        printf(" │");
        for (int x = 0; x < TAILLE_CARTE; x++) {
            Zone *zone = &carte->grille[y][x];
            
            // Symbole de la zone
            if (carte->position_joueur.x == x && carte->position_joueur.y == y) {
                printf(" ♦%s ♦ │", symbole_zone(zone->type));
            } else {
                printf("  %s   │", symbole_zone(zone->type));
            }
        }
        
        // Indicateur de niveau
        if (y == 0) printf(" SURFACE │ %dm", carte->grille[y][0].profondeur);
        else printf(" ZONE %d │ %dm", y, carte->grille[y][0].profondeur);
        
        printf("\n │");
        
        // Ligne des noms et états
        for (int x = 0; x < TAILLE_CARTE; x++) {
            Zone *zone = &carte->grille[y][x];
            const char *nom = nom_type_zone(zone->type);
            
            // Nom abrégé
            char nom_court[10];
            if (strcmp(nom, "Récif Corallien") == 0) strcpy(nom_court, "Récif");
            else if (strcmp(nom, "Épave du Titanic") == 0) strcpy(nom_court, "Épave");
            else if (strcmp(nom, "Océan Ouvert") == 0) strcpy(nom_court, "Vide");
            else if (strcmp(nom, "Base Sous-Marine") == 0) strcpy(nom_court, "Base");
            else if (strcmp(nom, "Forêt d'Algues") == 0) strcpy(nom_court, "Forêt");
            else if (strcmp(nom, "Gouffre Mystérieux") == 0) strcpy(nom_court, "Gouffre");
            else if (strcmp(nom, "Abysses") == 0) strcpy(nom_court, "Abysses");
            else strcpy(nom_court, "Inconnu");
            
            printf(" %-7s │", nom_court);
        }
        printf("         │\n");
        
        // Ligne des états
        printf(" │");
        for (int x = 0; x < TAILLE_CARTE; x++) {
            Zone *zone = &carte->grille[y][x];
            
            if (!zone->debroquee) {
                printf(" [LOCK]  │");
            } else if (!zone->exploree) {
                printf("         │");
            } else if (zone->nb_creatures > 0) {
                printf(" [%d ENM] │", zone->nb_creatures);
            } else if (zone->type == ZONE_EPAVES) {
                printf(" [TRÉSOR]│");
            } else if (zone->type == ZONE_SURFACE) {
                printf(" [SAUF]  │");
            } else {
                printf("         │");
            }
        }
        printf("         │\n");
        
        if (y < TAILLE_CARTE - 1) {
            printf(" ├─────────┼─────────┼─────────┼─────────┼──────────┤\n");
        }
    }
    
    printf(" └─────────┴─────────┴─────────┴─────────┴──────────┘\n");
    
    Zone *zone_actuelle = get_zone_actuelle(carte);
    if (zone_actuelle) {
        printf("\nPosition actuelle: [%s] %s (%dm)\n", 
               symbole_zone(zone_actuelle->type),
               nom_type_zone(zone_actuelle->type),
               zone_actuelle->profondeur);
        
        // Conditions environnementales simulées
        printf("Conditions: ");
        if (zone_actuelle->profondeur == 0) printf("Courant faible, Visibilité excellente, Température 25°C\n");
        else if (zone_actuelle->profondeur <= 50) printf("Courant faible, Visibilité bonne, Température 23°C\n");
        else if (zone_actuelle->profondeur <= 150) printf("Courant moyen, Visibilité moyenne, Température 18°C\n");
        else printf("Courant fort, Visibilité faible, Température 12°C\n");
    }
}

/* Obtenir le nom d'un type de zone */
const char* nom_type_zone(TypeZone type) {
    switch (type) {
        case ZONE_SURFACE: return "Base Sous-Marine";
        case ZONE_RECIFS: return "Récif Corallien";
        case ZONE_EPAVES: return "Épave du Titanic";
        case ZONE_FORET_ALGUES: return "Forêt d'Algues";
        case ZONE_GROTTES: return "Gouffre Mystérieux";
        case ZONE_FOSSES: return "Abysses";
        case ZONE_INCONNUE: return "Fosse Profonde";
        case ZONE_VIDE: return "Océan Ouvert";
        default: return "Zone Inconnue";
    }
}

/* Obtenir le symbole d'un type de zone */
const char* symbole_zone(TypeZone type) {
    switch (type) {
        case ZONE_SURFACE: return "🏝️";
        case ZONE_RECIFS: return "🐠";
        case ZONE_EPAVES: return "💰";
        case ZONE_FORET_ALGUES: return "🌿";
        case ZONE_GROTTES: return "🕳️";
        case ZONE_FOSSES: return "🐙";
        case ZONE_INCONNUE: return "❓";
        case ZONE_VIDE: return "❌";
        default: return "?";
    }
}

/* Sauvegarder la carte */
int sauvegarder_carte(FILE *out, CarteOceanique *carte) {
    if (!out || !carte) return -1;
    
    fprintf(out, "CARTE:\n");
    fprintf(out, "POSITION:%d:%d\n", carte->position_joueur.x, carte->position_joueur.y);
    fprintf(out, "DECOUVERTES:%d\n", carte->zones_decouvertes);
    
    for (int y = 0; y < TAILLE_CARTE; y++) {
        for (int x = 0; x < TAILLE_CARTE; x++) {
            Zone *zone = &carte->grille[y][x];
            fprintf(out, "ZONE:%d:%d:%d:%d:%d:%d:%d\n",
                   x, y, (int)zone->type, zone->profondeur,
                   zone->exploree, zone->debroquee, zone->nb_creatures);
        }
    }
    
    return 0;
}

/* Charger la carte */
int charger_carte(FILE *in, CarteOceanique *carte) {
    if (!in || !carte) return -1;
    
    char ligne[256];
    
    // Lire l'en-tête CARTE:
    if (!fgets(ligne, sizeof(ligne), in) || strncmp(ligne, "CARTE:", 6) != 0) {
        return -1;
    }
    
    // Lire la position
    if (fgets(ligne, sizeof(ligne), in)) {
        if (sscanf(ligne, "POSITION:%d:%d", &carte->position_joueur.x, &carte->position_joueur.y) != 2) {
            return -1;
        }
    }
    
    // Lire les découvertes
    if (fgets(ligne, sizeof(ligne), in)) {
        if (sscanf(ligne, "DECOUVERTES:%d", &carte->zones_decouvertes) != 1) {
            return -1;
        }
    }
    
    // Lire les zones
    for (int i = 0; i < TAILLE_CARTE * TAILLE_CARTE; i++) {
        if (fgets(ligne, sizeof(ligne), in)) {
            int x, y, type, profondeur, exploree, debroquee, nb_creatures;
            if (sscanf(ligne, "ZONE:%d:%d:%d:%d:%d:%d:%d",
                      &x, &y, &type, &profondeur, &exploree, &debroquee, &nb_creatures) == 7) {
                Zone *zone = &carte->grille[y][x];
                zone->type = (TypeZone)type;
                zone->profondeur = profondeur;
                zone->exploree = exploree;
                zone->debroquee = debroquee;
                zone->nb_creatures = nb_creatures;
            }
        }
    }
    
    // Réparer les zones adjacentes pour toutes les zones explorées
    for (int y = 0; y < TAILLE_CARTE; y++) {
        for (int x = 0; x < TAILLE_CARTE; x++) {
            if (carte->grille[y][x].exploree) {
                debloquer_zones_adjacentes(carte, x, y);
            }
        }
    }
    
    return 0;
}