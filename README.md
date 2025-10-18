# OceanDepth
.\oceandepth_enhanced.exe   # Version finale complète
.\oceandepth_explorer.exe   # Version avec exploration manuelle  
.\test_carte.exe           # Version de base


pour compiler:
gcc -o oceandepth_final.exe test_carte.c carte.c combat.c creatures.c joueur.c sauvegarde.c inventaire.c


pour lancer la demo:
./oceandepth_final.exe

OceanDepth/
├──  FICHIERS DE JEU
│   ├── test_carte.c           ← FICHIER PRINCIPAL
│   ├── oceandepth_final.exe   ← EXÉCUTABLE FINAL
│   └── partie_oceandepth.save ← SAUVEGARDE
│
├──  MODULES C
│   ├── carte.c / carte.h      ← Cartographie
│   ├── combat.c / combat.h    ← Combat
│   ├── creatures.c / creatures.h ← Créatures  
│   ├── joueur.c / joueur.h    ← Joueur
│   ├── sauvegarde.c           ← Sauvegarde
│   └── inventaire.c           ← Inventaire
│
