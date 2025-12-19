#!/bin/bash

# Script principal du projet C-Wildwater
# Gère les commandes histo et leaks

# Vérification des paramètres
if [ $# -lt 2 ]; then
    echo "Erreur : Nombre d'arguments insuffisant"
    echo ""
    echo "Usage :"
    echo "  $0 <fichier_donnees> histo {max|src|real}"
    echo "  $0 <fichier_donnees> leaks <id_usine>"
    echo ""
    exit 1
fi

# Arguments d'entrée
program=$0
dirname=$(dirname "$program")
datafile="$1"
action=$2

# Démarrage du chronomètre
start_time=$(date +%s)
trap 'end_time=$(date +%s); echo "Durée totale du script : $((end_time - start_time)) s"' EXIT

# Vérification de l'existence du fichier de données
if [ ! -f "$datafile" ]; then
    echo "Erreur : fichier $datafile introuvable"
    exit 5
fi

# Création des répertoires nécessaires
mkdir -p "$dirname/csv"
mkdir -p "$dirname/dat"
mkdir -p "$dirname/bin"

# Filtrage des données pour ne prendre que les lignes source=>usine
filtered_file="$dirname/dat/$(basename "$datafile").filtered"
grep "^-" "$datafile" | grep -v ";-;" > "$filtered_file"

if [ ! -s "$filtered_file" ]; then
    echo "Erreur : Aucune donnée valide après filtrage"
    rm -f "$filtered_file"
    exit 7
fi

# Fonction de compilation
compile() {
    if [ ! -x "$dirname/bin/wildwater" ]; then
        echo "Attention : fichier $dirname/bin/wildwater absent => compilation en cours..."
        cd "$dirname" || exit 1
        make
        retour=$?
        if [ $retour -ne 0 ]; then
            echo "Erreur : compilation échouée"
            exit 1
        fi
        echo "Compilation réussie"
    fi
}

# Compilation si nécessaire
compile

# Traitement selon l'action demandée

if [ "$action" = "histo" ]; then
    # Commande HISTO
    if [ $# -ne 3 ]; then
        echo "Erreur : histo nécessite un paramètre {max|src|real}"
        exit 2
    fi
    
    mode=$3
    
    # Validation du mode
    if [ "$mode" != "max" ] && [ "$mode" != "src" ] && [ "$mode" != "real" ]; then
        echo "Erreur : mode '$mode' invalide (attendu: max, src ou real)"
        exit 2
    fi
    
    csvfile="$dirname/csv/histo_$mode.csv"
    
    # Exécution du programme C
    "$dirname/bin/histo" "$mode" "$filtered_file" "$csvfile"
    retour=$?
    
    if [ $retour -ne 0 ]; then
        echo "Erreur : échec du traitement histogramme (code: $retour)"
        rm -f "$filtered_file"
        exit $retour
    fi
    
    echo "Histogramme généré : $csvfile"
    
    # Génération du graphique avec gnuplot (si disponible)
    if command -v gnuplot &> /dev/null; then
        pngfile="$dirname/csv/histo_$mode.png"
        
        # Script gnuplot pour générer l'histogramme
        gnuplot << EOF
set terminal png size 1200,800
set output '$pngfile'
set datafile separator ";"
set style data histogram
set style fill solid border
set xlabel "Usines de traitement"
set ylabel "Volume (k.m³/an)"
set title "Histogramme des usines - Mode: $mode"
set xtics rotate by -45
set grid y
plot '$csvfile' using 2:xtic(1) title "Volume" linecolor rgb "#4472C4"
EOF
        
        if [ $? -eq 0 ]; then
            echo "Graphique généré : $pngfile"
        fi
    fi
    
    rm -f "$filtered_file"
    exit 0

elif [ "$action" = "leaks" ]; then
    # Commande LEAKS
    if [ $# -ne 3 ]; then
        echo "Erreur : leaks nécessite un identifiant d'usine"
        exit 2
    fi
    
    usine=$3
    csvfile="$dirname/csv/leaks_${usine//[ #]/_}.csv"
    
    # Exécution du programme C
    "$dirname/bin/leaks" "$usine" "$filtered_file" "$csvfile"
    retour=$?
    
    if [ $retour -ne 0 ]; then
        echo "Erreur : échec du traitement leaks (code: $retour)"
        rm -f "$filtered_file"
        exit $retour
    fi
    
    echo "Fuites calculées : $csvfile"
    
    rm -f "$filtered_file"
    exit 0

else
    # Action inconnue
    echo "Erreur : action inconnue '$action'"
    echo "Actions disponibles : histo, leaks"
    rm -f "$filtered_file"
    exit 2
fi
