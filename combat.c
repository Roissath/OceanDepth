/* combat.c
 * Implémentation simplifiée de l'étape 3 : attaques des créatures marines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "creatures.h"
#include "joueur.h"

// Calculer dégâts d'une créature (variabilité)
static int creature_compute_damage(Creature *c) {
    if (!c) return 0;
    int base = c->attack; // chez nous attack est fixe
    // introduire une petite variabilité +/-20%
    int variance = (base * 20) / 100;
    int min = base - variance; if (min < 1) min = 1;
    int max = base + variance;
    return rand() % (max - min + 1) + min;
}

// Applique effets spéciaux selon le type
static void apply_special_effect(Creature *c, Plongeur *p, int *player_attack_reduction, float *damage_multiplier) {
    if (!c || !p) return;
    switch (c->type) {
        case CREATURE_TYPE_BOSS: // kraken -> double attaque (géré par hits=2)
            break;
        case CREATURE_TYPE_SHARK: // requin -> frénésie si PV < 50%
            if (c->hp * 2 < c->max_hp) *damage_multiplier = 1.3f;
            break;
        case CREATURE_TYPE_CRAB: // crabe -> réduction dégâts (géré dans creature_damage)
            break;
        default:
            break;
    }
}

// Phase d'attaque des créatures : elles attaquent le plongeur selon vitesse.
// Pour simplifier, nous itérons la liste, bâtissons un tableau trié par vitesse
// puis appliquons les attaques avec effets spéciaux.
int phase_attaque_creatures(Plongeur *plongeur, CreatureList *liste) {
    if (!liste || !plongeur) return 0;
    // Compter créatures vivantes
    int count = 0;
    for (Creature *c = liste->head; c; c = c->next) if (c->alive) count++;
    if (count == 0) return 0;

    Creature **arr = malloc(sizeof(Creature*) * count);
    if (!arr) return 0;
    int i = 0;
    for (Creature *c = liste->head; c; c = c->next) if (c->alive) arr[i++] = c;

    
    for (int a = 0; a < count-1; ++a) for (int b = a+1; b < count; ++b) {
        int speed_a = 0, speed_b = 0;
        switch (arr[a]->type) {
            case CREATURE_TYPE_FISH: speed_a = 25; break;
            case CREATURE_TYPE_SHARK: speed_a = 30; break; // requin: rapide
            case CREATURE_TYPE_CRAB: speed_a = 10; break;  // crabe: lent mais résistant
            case CREATURE_TYPE_BOSS: speed_a = 20; break;  // kraken: vitesse moyenne
            default: speed_a = 5; break;
        }
        switch (arr[b]->type) {
            case CREATURE_TYPE_FISH: speed_b = 25; break;
            case CREATURE_TYPE_SHARK: speed_b = 30; break;
            case CREATURE_TYPE_CRAB: speed_b = 10; break;
            case CREATURE_TYPE_BOSS: speed_b = 20; break;
            default: speed_b = 5; break;
        }
        if (speed_b > speed_a || (speed_b == speed_a && arr[b]->id < arr[a]->id)) {
            Creature *tmp = arr[a]; arr[a] = arr[b]; arr[b] = tmp;
        }
    }

    int degats_totaux = 0;
    
    // Chaque créature attaque
    for (int k = 0; k < count; ++k) {
        Creature *c = arr[k];
        if (!c || !c->alive) continue;

        // calculate damage
        float dmg_mult = 1.0f;
        int player_attack_reduction = 0;
        apply_special_effect(c, plongeur, &player_attack_reduction, &dmg_mult);

        // Kraken spécial: deux attaques consécutives ("Étreinte tentaculaire")
        int hits = (c->type == CREATURE_TYPE_BOSS) ? 2 : 1;
        for (int h = 0; h < hits; ++h) {
            int dmg = creature_compute_damage(c);
            int effective = (int)(dmg * dmg_mult) - plongeur->defense;
            
            if (effective < 1) effective = 1;
            plongeur->points_de_vie -= effective;
            degats_totaux += effective;
            
            // Stress: chaque attaque fait perdre 1-2 oxygène supplémentaire
            plongeur->niveau_oxygene -= (rand()%2)+1;

            // Messages descriptifs selon le type de créature
            const char* nom_creature = "Créature";
            const char* effet_special = "";
            switch (c->type) {
                case CREATURE_TYPE_BOSS:
                    nom_creature = "Kraken";
                    effet_special = (hits == 2 && h == 0) ? " (Étreinte tentaculaire - 1ère attaque)" : 
                                   (hits == 2 && h == 1) ? " (Étreinte tentaculaire - 2ème attaque)" : "";
                    break;
                case CREATURE_TYPE_SHARK:
                    nom_creature = "Requin";
                    effet_special = (dmg_mult > 1.0f) ? " (Frénésie sanguinaire - dégâts augmentés)" : "";
                    break;
                case CREATURE_TYPE_CRAB:
                    nom_creature = "Crabe Géant";
                    break;
                default:
                    nom_creature = "Poisson";
                    break;
            }

            printf("� Une créature vous inflige %d dégâts!\n", effective);

            if (plongeur->points_de_vie <= 0) {
                printf("💀 Le plongeur est mort !\n");
                free(arr);
                return degats_totaux;
            }
        }
    }

    free(arr);
    return degats_totaux;
}

// Exécute un tour de combat complet selon l'ordre décrit
// Simplifications : actions du joueur sont simulées (attaquer une créature la plus proche)
// pour éviter d'ajouter une I/O interactive ici.
void tour_de_combat(CreatureList *liste, Plongeur *plongeur, int profondeur) {
    if (!liste || !plongeur) return;

    // Vérifier s'il y a des créatures
    int nb_creatures = 0;
    for (Creature *c = liste->head; c; c = c->next) {
        if (c->alive && c->hp > 0) nb_creatures++;
    }
    
    if (nb_creatures == 0) {
        printf("🎉 Victoire! Toutes les créatures ont été vaincues!\n");
        printf("💰 +%d perles récupérées!\n", nb_creatures * 10);
        plongeur->perles += nb_creatures * 10;
        return;
    }
    
    printf("\n=== TOUR DE COMBAT ===\n");
    printf("👤 Votre état: %d/%d PV, %d/%d O2, Fatigue: %d\n", 
           plongeur->points_de_vie, plongeur->points_de_vie_max,
           plongeur->niveau_oxygene, plongeur->niveau_oxygene_max,
           plongeur->niveau_fatigue);
    
    // Afficher les créatures présentes
    int index = 1;
    for (Creature *c = liste->head; c; c = c->next) {
        if (c->alive && c->hp > 0) {
            const char *nom = "Créature";
            switch (c->type) {
                case CREATURE_TYPE_FISH: nom = "Poisson"; break;
                case CREATURE_TYPE_SHARK: nom = "Requin"; break;
                case CREATURE_TYPE_CRAB: nom = "Crabe"; break;
                case CREATURE_TYPE_BOSS: nom = "Boss"; break;
                default: nom = "Créature"; break;
            }
            printf("🦈 %s: %d/%d PV\n", nom, c->hp, c->max_hp);
            index++;
        }
    }
    
    printf("\nVos actions:\n");
    printf("1. ⚔️  Attaquer une créature\n");
    printf("2. 🛡️  Se défendre (+2 défense ce tour)\n");
    printf("3. 🏃 Fuir le combat\n");
    printf("Votre choix: ");
    
    char buffer[100];
    fgets(buffer, sizeof(buffer), stdin);
    int choix = atoi(buffer);
    
    switch(choix) {
        case 1: {
            // Attaque
            printf("Choisir la cible (1-%d): ", nb_creatures);
            fgets(buffer, sizeof(buffer), stdin);
            int cible = atoi(buffer) - 1;
            
            if (cible >= 0 && cible < nb_creatures) {
                // Trouver la créature par index
                Creature *creature_cible = NULL;
                int current_index = 0;
                for (Creature *c = liste->head; c; c = c->next) {
                    if (c->alive && c->hp > 0) {
                        if (current_index == cible) {
                            creature_cible = c;
                            break;
                        }
                        current_index++;
                    }
                }
                
                if (creature_cible) {
                    int degats = 15 + rand() % 10; // 15-24 dégâts
                    printf("💥 Vous infligez %d dégâts!\n", degats);
                    
                    int mort = creature_damage(creature_cible, degats);
                    if (mort) {
                        printf("☠️ Créature éliminée!\n");
                    }
                }
            }
            break;
        }
        case 2:
            // Défense
            printf("🛡️ Vous vous défendez! (+2 défense temporaire)\n");
            plongeur->defense += 2;
            break;
        case 3:
            // Fuite
            printf("🏃 Vous fuyez le combat!\n");
            return;
        default:
            printf("❌ Action invalide!\n");
            break;
    }
    
    // Phase d'attaque des créatures
    printf("\n=== TOUR DES CRÉATURES ===\n");
    phase_attaque_creatures(plongeur, liste);
    
    // Réduire défense temporaire
    if (plongeur->defense > 2) {
        plongeur->defense -= 2;
    }
    
    // Consommer oxygène
    plongeur->niveau_oxygene -= 2;
    if (plongeur->niveau_oxygene < 0) {
        plongeur->niveau_oxygene = 0;
        plongeur->points_de_vie -= 5; // Dégâts d'asphyxie
        printf("💀 Vous manquez d'oxygène! -5 PV\n");
    }
}