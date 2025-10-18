/* test_carte.c
 * Test interactif du système de jeu OceanDepth - VERSION CORRIGÉE
 * Intègre: Cartographie, Combat, Créatures, Joueur, Sauvegarde
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "carte.h"
#include "joueur.h"
#include "creatures.h"
#include "combat.h"

void afficher_menu() {
    printf("\n🌊 === OCEANDEPTH - MENU PRINCIPAL === 🌊\n");
    printf("1. 🧭  Avancer (choisir direction)\n");
    printf("2. 🔍  Explorer la zone actuelle\n");
    printf("3. ⚔️  Attaquer une créature\n");
    printf("4. 🏥  Afficher statut du joueur\n");
    printf("5. 🦈  Afficher créatures présentes\n");
    printf("6. 💾  Sauvegarder la partie\n");
    printf("7. 📂  Charger une partie\n");
    printf("8. ❌  Quitter\n");
    printf("Votre choix: ");
}

void afficher_statut_joueur(Plongeur *joueur) {
    printf("\n👤 === PROFIL DU PLONGEUR ===\n");
    printf("📛 Nom: %s %s\n", joueur->prenom, joueur->nom);
    printf("📅 Partie créée: %s\n", joueur->date_creation);
    printf("🎯 Avancement: %d%%\n", joueur->avancement);
    printf("⏱️  Temps de jeu: %d minutes\n", joueur->temps_jeu);
    printf("\n📊 === STATISTIQUES ===\n");
    printf("❤️  Points de vie: %d/%d\n", joueur->points_de_vie, joueur->points_de_vie_max);
    printf("🫁 Oxygène: %d/%d\n", joueur->niveau_oxygene, joueur->niveau_oxygene_max);
    printf("😴 Fatigue: %d/5\n", joueur->niveau_fatigue);
    printf("🛡️  Défense: %d\n", joueur->defense);
    printf("💰 Perles: %d\n", joueur->perles);
    if (joueur->tours_paralyse > 0) {
        printf("🥶 Paralysé pour %d tours\n", joueur->tours_paralyse);
    }
}

void gerer_avancement(CarteOceanique *carte, Plongeur *joueur) {
    printf("\n🧭 === AVANCEMENT ===\n");
    printf("Position actuelle: (%d, %d)\n", carte->position_joueur.x, carte->position_joueur.y);
    
    printf("Choisissez votre direction:\n");
    printf("1. ⬆️  Haut (monter d'une zone)\n");
    printf("2. ⬇️  Bas (descendre d'une zone)\n");
    printf("3. ⬅️  Gauche\n");
    printf("4. ➡️  Droite\n");
    printf("Votre choix: ");
    fflush(stdout);
    
    int direction;
    char buffer[10];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("❌ Erreur de lecture!\n");
        return;
    }
    
    direction = atoi(buffer);
    if (direction < 1 || direction > 4) {
        printf("❌ Direction invalide! Choisissez entre 1 et 4.\n");
        return;
    }
    
    int nouveau_x = carte->position_joueur.x;
    int nouveau_y = carte->position_joueur.y;
    
    switch (direction) {
        case 1: // Haut
            nouveau_y = carte->position_joueur.y - 1;
            printf("🔄 Tentative de mouvement vers le haut...\n");
            break;
        case 2: // Bas  
            nouveau_y = carte->position_joueur.y + 1;
            printf("🔄 Tentative de mouvement vers le bas...\n");
            break;
        case 3: // Gauche
            nouveau_x = carte->position_joueur.x - 1;
            printf("🔄 Tentative de mouvement vers la gauche...\n");
            break;
        case 4: // Droite
            nouveau_x = carte->position_joueur.x + 1;
            printf("🔄 Tentative de mouvement vers la droite...\n");
            break;
        default:
            printf("❌ Direction invalide! Choisissez entre 1 et 4.\n");
            return;
    }
    
    printf("Nouvelle destination: (%d, %d)\n", nouveau_x, nouveau_y);
    
    if (!peut_se_deplacer_vers(carte, joueur, nouveau_x, nouveau_y)) {
        printf("❌ Mouvement impossible!\n");
        if (nouveau_x < 0 || nouveau_x >= TAILLE_CARTE || nouveau_y < 0 || nouveau_y >= TAILLE_CARTE) {
            printf("   → Vous atteignez les limites de la zone d'exploration\n");
        } else {
            Zone *zone_cible = &carte->grille[nouveau_y][nouveau_x];
            if (!zone_cible->debroquee) {
                printf("   → Zone non accessible (explorez les zones adjacentes d'abord)\n");
            } else if (zone_cible->profondeur >= 50 && joueur->defense < 1) {
                printf("   → Équipement insuffisant pour cette profondeur (défense < 1)\n");
            } else if (zone_cible->profondeur >= 150 && joueur->defense < 2) {
                printf("   → Équipement insuffisant pour cette profondeur (défense < 2)\n");
            } else if (zone_cible->profondeur >= 300 && joueur->defense < 3) {
                printf("   → Équipement insuffisant pour cette profondeur (défense < 3)\n");
            } else {
                printf("   → Oxygène insuffisant pour ce déplacement\n");
            }
        }
        return;
    }
    
    ResultatDeplacement resultat = se_deplacer(carte, nouveau_x, nouveau_y, joueur);
    switch (resultat) {
        case DEPLACEMENT_OK:
            printf("✅ Avancement réussi!\n");
            Zone *nouvelle_zone = get_zone_actuelle(carte);
            if (nouvelle_zone) {
                printf("🗺️ Vous êtes maintenant à: %s (%dm)\n", 
                       nouvelle_zone->nom, nouvelle_zone->profondeur);
                if (nouvelle_zone->profondeur > 0) {
                    printf("🌊 Profondeur actuelle: %dm\n", nouvelle_zone->profondeur);
                }
            }
            break;
        default:
            printf("❌ Échec de l'avancement\n");
            break;
    }
}

void gerer_combat_interactif(CarteOceanique *carte, Plongeur *joueur, CreatureList *creatures) {
    Zone *zone_actuelle = get_zone_actuelle(carte);
    if (!zone_actuelle || zone_actuelle->nb_creatures == 0) {
        printf("❌ Aucune créature dans cette zone!\n");
        return;
    }
    
    printf("\n⚔️ === COMBAT INTERACTIF ===\n");
    printf("Créatures présentes: %d\n", zone_actuelle->nb_creatures);
    
    // Générer quelques créatures pour le combat
    for (int i = 0; i < zone_actuelle->nb_creatures && i < 3; i++) {
        CreatureType type = (CreatureType)(1 + rand() % 4); // 1-4 pour les types disponibles
        Creature *c = creature_new(creatures, type, carte->position_joueur.x, carte->position_joueur.y);
        if (!c) {
            printf("❌ Erreur lors de la création de créature\n");
        }
    }
    
    // Combat simple 
    int creatures_vivantes = 0;
    Creature *c = creatures->head;
    while (c) {
        if (c->hp > 0) creatures_vivantes++;
        c = c->next;
    }
    
    if (creatures_vivantes > 0) {
        printf("💥 Combat automatique!\n");
        c = creatures->head;
        while (c && c->hp > 0) {
            int degats = 20 + joueur->defense;
            creature_damage(c, degats);
            printf("⚔️ Vous infligez %d dégâts!\n", degats);
            if (c->hp <= 0) {
                printf("☠️ Créature éliminée!\n");
                joueur->perles += 5;
                printf("💰 +5 perles!\n");
            }
            c = c->next;
        }
        zone_actuelle->nb_creatures = 0;
    }
    
    // Vider la liste 
    creatures_clear(creatures);
}

void sauvegarder_partie_complete(CarteOceanique *carte, Plongeur *joueur, CreatureList *creatures) {
    FILE *fichier = fopen("partie_oceandepth.save", "w");
    if (!fichier) {
        printf("❌ Impossible de créer le fichier de sauvegarde!\n");
        return;
    }
    
    fprintf(fichier, "OCEANDEPTH_SAVE_V1\n");
    
    // Sauvegarder le joueur
    if (sauvegarder_joueur(fichier, joueur) != 0) {
        printf("❌ Erreur lors de la sauvegarde du joueur!\n");
        fclose(fichier);
        return;
    }
    
    // Sauvegarder les créatures
    if (creatures_save(fichier, creatures) != 0) {
        printf("❌ Erreur lors de la sauvegarde des créatures!\n");
        fclose(fichier);
        return;
    }
    
    // Sauvegarder la carte
    if (sauvegarder_carte(fichier, carte) != 0) {
        printf("❌ Erreur lors de la sauvegarde de la carte!\n");
        fclose(fichier);
        return;
    }
    
    fclose(fichier);
    printf("✅ Partie sauvegardée dans 'partie_oceandepth.save'!\n");
}

int charger_partie_complete(CarteOceanique *carte, Plongeur *joueur, CreatureList *creatures) {
    FILE *fichier = fopen("partie_oceandepth.save", "r");
    if (!fichier) {
        printf("❌ Aucune sauvegarde trouvée!\n");
        return -1;
    }
    
    char ligne[256];
    if (!fgets(ligne, sizeof(ligne), fichier) || strncmp(ligne, "OCEANDEPTH_SAVE_V1", 18) != 0) {
        printf("❌ Format de sauvegarde incompatible!\n");
        fclose(fichier);
        return -1;
    }
    
    // Charger le joueur
    if (charger_joueur(fichier, joueur) != 0) {
        printf("❌ Erreur lors du chargement du joueur!\n");
        fclose(fichier);
        return -1;
    }
    
    // Charger les créatures
    if (creatures_load(fichier, creatures) != 0) {
        printf("❌ Erreur lors du chargement des créatures!\n");
        fclose(fichier);
        return -1;
    }
    
    // Charger la carte
    if (charger_carte(fichier, carte) != 0) {
        printf("❌ Erreur lors du chargement de la carte!\n");
        fclose(fichier);
        return -1;
    }
    
    fclose(fichier);
    printf("✅ Partie chargée avec succès!\n");
    
    // Réparer les zones débloquées après chargement
    for (int y = 0; y < TAILLE_CARTE; y++) {
        for (int x = 0; x < TAILLE_CARTE; x++) {
            if (carte->grille[y][x].exploree) {
                debloquer_zones_adjacentes(carte, x, y);
            }
        }
    }
    printf("🔧 Réparation des zones débloquées terminée.\n");
    
    return 0;
}

int main() {
    srand(time(NULL));
    
    printf("🌊 === OCEANDEPTH - EXPLORATION SOUS-MARINE === 🌊\n");
    printf("Bienvenue dans les profondeurs océaniques!\n\n");
    
    // Variables pour le jeu
    CarteOceanique carte;
    Plongeur joueur;
    CreatureList creatures;
    
    // Vérifier si une sauvegarde existe
    FILE *test_save = fopen("partie_oceandepth.save", "r");
    if (test_save) {
        fclose(test_save);
        
        printf("💾 Une sauvegarde a été détectée!\n");
        printf("Voulez-vous:\n");
        printf("1. 📂 Charger la partie sauvegardée\n");
        printf("2. 🆕 Commencer une nouvelle partie\n");
        printf("Votre choix: ");
        
        int choix_save;
        char buffer[10];
        fflush(stdout);
        
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            choix_save = atoi(buffer);
        } else {
            choix_save = 2; // Par défaut nouvelle partie
        }
        
        if (choix_save == 1) {
            // Charger partie existante
            initialiser_joueur(&joueur, "Joueur", "Charge", 100, 100, 80, 80, 0, 0);
            creatures_init(&creatures);
            initialiser_carte(&carte, &joueur);
            
            if (charger_partie_complete(&carte, &joueur, &creatures) == 0) {
                printf("✅ Partie chargée avec succès!\n");
                printf("🎮 Bienvenue %s %s! (Avancement: %d%%)\n\n", 
                       joueur.prenom, joueur.nom, joueur.avancement);
            } else {
                printf("❌ Erreur lors du chargement! Nouvelle partie...\n");
                choix_save = 2;
            }
        }
        
        if (choix_save == 2) {
            // Nouvelle partie - demander nom et prénom
            char nom_joueur[50];
            char prenom_joueur[50];
            
            printf("🆕 Nouvelle partie!\n");
            printf("👤 Entrez votre prénom: ");
            fflush(stdout);
            if (fgets(prenom_joueur, sizeof(prenom_joueur), stdin) != NULL) {
                prenom_joueur[strcspn(prenom_joueur, "\n")] = 0;
            } else {
                strcpy(prenom_joueur, "Plongeur");
            }
            
            printf("👤 Entrez votre nom: ");
            fflush(stdout);
            if (fgets(nom_joueur, sizeof(nom_joueur), stdin) != NULL) {
                nom_joueur[strcspn(nom_joueur, "\n")] = 0;
            } else {
                strcpy(nom_joueur, "Anonyme");
            }
            
            printf("\n🤿 Bienvenue %s %s! Préparez-vous à explorer les océans!\n", prenom_joueur, nom_joueur);
            
            // Initialiser nouvelle partie
            initialiser_joueur(&joueur, nom_joueur, prenom_joueur, 100, 100, 80, 80, 0, 0);
            creatures_init(&creatures);
            
            if (initialiser_carte(&carte, &joueur) != 0) {
                printf("❌ Erreur lors de l'initialisation du jeu!\n");
                return 1;
            }
            
            printf("✅ Jeu initialisé! Vous commencez à la base de plongée.\n");
        }
    } else {
        // Pas de sauvegarde - nouvelle partie
        char nom_joueur[50];
        char prenom_joueur[50];
        
        printf("🆕 Aucune sauvegarde détectée. Nouvelle partie!\n");
        printf("👤 Entrez votre prénom: ");
        fflush(stdout);
        if (fgets(prenom_joueur, sizeof(prenom_joueur), stdin) != NULL) {
            prenom_joueur[strcspn(prenom_joueur, "\n")] = 0;
        } else {
            strcpy(prenom_joueur, "Plongeur");
        }
        
        printf("👤 Entrez votre nom: ");
        fflush(stdout);
        if (fgets(nom_joueur, sizeof(nom_joueur), stdin) != NULL) {
            nom_joueur[strcspn(nom_joueur, "\n")] = 0;
        } else {
            strcpy(nom_joueur, "Anonyme");
        }
        
        printf("\n🤿 Bienvenue %s %s! Préparez-vous à explorer les océans!\n", prenom_joueur, nom_joueur);
        
        // Initialiser nouvelle partie
        initialiser_joueur(&joueur, nom_joueur, prenom_joueur, 100, 100, 80, 80, 0, 0);
        creatures_init(&creatures);
        
        if (initialiser_carte(&carte, &joueur) != 0) {
            printf("❌ Erreur lors de l'initialisation du jeu!\n");
            return 1;
        }
        
        printf("✅ Jeu initialisé! Vous commencez à la base de plongée.\n");
    }
    
    // BOUCLE DE JEU PRINCIPALE
    printf("\n🎮 === DEBUT DE L'AVENTURE === 🎮\n");
    int continuer = 1;
    while (continuer) {
        // Afficher la carte automatiquement à chaque tour
        afficher_carte_complete(&carte);
        
        // Explorer automatiquement la zone actuelle si pas encore explorée
        Zone *zone_actuelle = get_zone_actuelle(&carte);
        if (zone_actuelle && !zone_actuelle->exploree) {
            printf("\n🔍 Exploration automatique de %s...\n", zone_actuelle->nom);
            explorer_zone_actuelle(&carte, &joueur);
        }
        
        afficher_menu();
        
        int choix;
        char buffer[10];
        printf("Votre choix: ");
        fflush(stdout);
        
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("❌ Erreur de lecture!\n");
            continue;
        }
        
        choix = atoi(buffer);
        if (choix < 1 || choix > 8) {
            printf("❌ Choix invalide! Veuillez choisir entre 1 et 8.\n");
            continue;
        }
        
        switch (choix) {
            case 1:
                gerer_avancement(&carte, &joueur);
                break;
                
            case 2:
                explorer_zone_actuelle(&carte, &joueur);
                break;
                
            case 3:
                gerer_combat_interactif(&carte, &joueur, &creatures);
                break;
                
            case 4:
                afficher_statut_joueur(&joueur);
                break;
                
            case 5: {
                Zone *zone = get_zone_actuelle(&carte);
                if (zone && zone->nb_creatures > 0) {
                    printf("🦈 %d créature(s) dans cette zone\n", zone->nb_creatures);
                } else {
                    printf("🌊 Aucune créature visible dans cette zone\n");
                }
                break;
            }
            
            case 6:
                sauvegarder_partie_complete(&carte, &joueur, &creatures);
                break;
                
            case 7:
                printf("📂 Chargement d'une partie...\n");
                if (charger_partie_complete(&carte, &joueur, &creatures) == 0) {
                    printf("✅ Partie chargée! Bonjour %s %s! (Avancement: %d%%)\n", 
                           joueur.prenom, joueur.nom, joueur.avancement);
                } else {
                    printf("❌ Échec du chargement.\n");
                }
                break;
                
            case 8:
                printf("🌊 Au revoir! Merci d'avoir exploré les profondeurs! 🌊\n");
                continuer = 0;
                break;
                
            default:
                printf("❌ Choix invalide!\n");
                break;
        }
        
        // Vérification de l'état du joueur
        if (joueur.points_de_vie <= 0) {
            printf("\n💀 GAME OVER - Vous avez succombé aux profondeurs...\n");
            printf("🔄 Retour à la base de plongée...\n");
            joueur.points_de_vie = 50; // Récupération partielle
            carte.position_joueur.x = 0;
            carte.position_joueur.y = 0;
            carte.position_joueur.zone_actuelle = ZONE_SURFACE;
        }
        
        if (joueur.niveau_oxygene <= 0) {
            printf("\n🫁 ALERTE OXYGÈNE - Remontée d'urgence!\n");
            printf("🔄 Retour à la surface...\n");
            joueur.niveau_oxygene = joueur.niveau_oxygene_max / 2;
            carte.position_joueur.x = 0;
            carte.position_joueur.y = 0;
            carte.position_joueur.zone_actuelle = ZONE_SURFACE;
        }
    }
    
    // Nettoyage
    creatures_clear(&creatures);
    
    return 0;
}