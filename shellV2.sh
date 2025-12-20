#!/bin/bash

# Script de test pour comparer les 3 approches de calcul

echo "=== TEST DES 3 APPROCHES DE CALCUL DES FUITES ==="
echo ""

# Usine à tester (à adapter selon vos données)
USINE="Facility complex #RH400057F"
DATAFILE="dat/wildwater.dat"

echo "Usine testée : $USINE"
echo ""

# Extraction des informations de l'usine
echo "--- Informations sur l'usine ---"

# Capacité maximale (ligne USINE)
echo "1. Capacité maximale de l'usine :"
grep "^-;$USINE;-;" "$DATAFILE" | head -1
echo ""

# Sources alimentant cette usine
echo "2. Sources alimentant cette usine :"
grep "^-;.*;$USINE;" "$DATAFILE" | head -5
echo "(... premières lignes seulement)"
echo ""

# Stockages en sortie de cette usine  
echo "3. Stockages en sortie de cette usine :"
grep "^-;$USINE;Storage" "$DATAFILE" | head -5
echo "(... premières lignes seulement)"
echo ""

# Calcul manuel des 3 approches
echo "--- Calculs manuels ---"

# Approche 1 : Somme des volumes captés (SANS fuites captage)
echo "Approche 1 : Somme(volumes_sources) sans appliquer fuites captage"
SOMME1=$(awk -F';' -v usine="$USINE" '
    $1 == "-" && $3 == usine && $4 != "-" {
        sum += $4
    }
    END { print sum }
' "$DATAFILE")
echo "Volume = $SOMME1 km³"
echo ""

# Approche 2 : Somme des volumes captés avec fuites captage (VERSION ACTUELLE)
echo "Approche 2 : Somme(volumes_sources × (1 - fuites%)) [VERSION ACTUELLE]"
SOMME2=$(awk -F';' -v usine="$USINE" '
    $1 == "-" && $3 == usine && $4 != "-" {
        volume = $4
        fuites = $5
        sum += volume * (100 - fuites) / 100
    }
    END { print sum }
' "$DATAFILE")
echo "Volume = $SOMME2 km³"
echo ""

# Approche 3 : Capacité maximale de l'usine
echo "Approche 3 : Capacité maximale de l'usine"
CAPACITE=$(awk -F';' -v usine="$USINE" '
    $1 == "-" && $2 == usine && $3 == "-" && $4 != "-" && $5 == "-" {
        print $4
        exit
    }
' "$DATAFILE")
echo "Volume = $CAPACITE km³"
echo ""

# Comparaison
echo "--- Comparaison ---"
echo "Approche 1 (sans fuites captage)  : $SOMME1 km³"
echo "Approche 2 (avec fuites captage)  : $SOMME2 km³"  
echo "Approche 3 (capacité maximale)    : $CAPACITE km³"
echo ""

# Vérifier si certaines approches donnent le même résultat
if [ "$SOMME1" = "$SOMME2" ]; then
    echo "⚠️  Approches 1 et 2 identiques → Fuites captage = 0% !"
fi

if [ "$SOMME1" = "$CAPACITE" ]; then
    echo "⚠️  Approches 1 et 3 identiques → Capacité = Somme sources !"
fi

if [ "$SOMME2" = "$CAPACITE" ]; then
    echo "⚠️  Approches 2 et 3 identiques → Coïncidence ou données spéciales !"
fi

echo ""
echo "=== Pour savoir quelle approche correspond aux résultats du prof ==="
echo "Compilez et testez votre programme avec cette usine, puis comparez."
