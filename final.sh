#!/bin/bash

# Vérification des Paramètres
if [ $# -lt 2 ]; then
    echo "Usage :"
    echo "  $0 <fichier.dat> histo {max|src|real|maxC|srcC|reelC}"
    echo "  $0 <fichier.dat> leaks {id usine}"
    exit 1
fi

# Arguments d'entrée
program=$0
dirname=$(dirname "$program")
datafile="$dirname/dat/$1"
action=$2

start_time=$(date +%s)
trap 'end_time=$(date +%s); echo -e "\nDurée totale du script : $((end_time - start_time)) s"' EXIT

# Controle existence du fichier de données
if [ ! -f "$datafile" ]; then
    echo "Erreur : fichier $datafile introuvable"
    exit 1
fi

mkdir -p "$dirname/csv"
mkdir -p "$dirname/bin"

compile() {
    local prog_name=$1
    if [ ! -x "$dirname/bin/$prog_name" ]; then
        echo "Attention: binaire $dirname/bin/$prog_name absent => compilation via Makefile..."
        make -C "$dirname" "$prog_name"
        retour=$?
        if [ $retour -ne 0 ]; then
            echo "Erreur : la compilation a échoué."
            exit 1
        fi
    fi
}

# L'action : histo
if [ "$action" = "histo" ]; then
    if [ $# -ne 3 ]; then
        echo "Usage: $0 <fichier.dat> histo {mode}"
        exit 1
    fi

    mode=$3
    csvfile="$dirname/csv/histo_$mode.csv"

    # taitement sur le shell (max, src, real)
    if [ "$mode" = "max" ] || [ "$mode" = "src" ] || [ "$mode" = "real" ]; then
        echo "Traitement via Shell/AWK ($mode)..."
        # Logique de calcul via AWK (simplifiée pour l'exemple)
        if [ "$mode" = "max" ]; then
            awk -F';' '$1=="-" && $2!="-" && $3=="-" && $4!="-" {print $2";"$4}' "$datafile" | sort -t';' -k2,2nr > "$csvfile.tmp"
        elif [ "$mode" = "src" ]; then
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {sum[$3]+=$4} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2nr > "$csvfile.tmp"
        else
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {res=$4*(100-$5)/100; sum[$3]+=res} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2nr > "$csvfile.tmp"
        fi

    # traitement sur le C (maxC, srcC, reelC) 
    elif [ "$mode" = "maxC" ] || [ "$mode" = "srcC" ] || [ "$mode" = "reelC" ]; then
        echo "Traitement via Programme C/AVL ($mode)..."
        compile "histo"
        "$dirname/bin/histo" "$datafile" "$csvfile" "$mode"
        
        # On trie le résultat du C par volume décroissant pour faciliter l'extraction
        tail -n +2 "$csvfile" | sort -t';' -k2,2nr > "$csvfile.tmp"
    else
        echo "Mode inconnu"; exit 1
    fi

    # top 10 max et top 50 min
    echo "Génération des classements dans $csvfile..."
    
      echo "    TOP 10 PLUS GRANDS VOLUMES" > "$csvfile"
    echo "identifier;volume" >> "$csvfile"
    head -n 10 "$csvfile.tmp" >> "$csvfile"
    
    echo "" >> "$csvfile"
    echo "    TOP 50 PLUS PETITS VOLUMES" >> "$csvfile"
    echo "identifier;volume" >> "$csvfile"
    tail -n 50 "$csvfile.tmp" | sort -t';' -k2,2n >> "$csvfile"

    rm "$csvfile.tmp"

    # GNUPLOT
    if command -v gnuplot >/dev/null 2>&1; then
        image_sortie="$dirname/csv/histo_$mode.png"
        echo "Génération du graphique..."
        # les 10 plus grandes 
        gnuplot <<EOF
            set terminal pngcairo size 1000,600 font "Arial,10"
            set output "$image_sortie"
            set datafile separator ";"
            set style fill solid
            set boxwidth 0.5
            set xtics rotate by -45
            set title "Top 10 - Volumes ($mode)"
            set ylabel "Volume (k.m3/an)"
            # On saute les lignes de texte du fichier pour ne tracer que les données
            plot "< grep -v '==' '$csvfile' | head -n 11 | tail -n 10" using 2:xtic(1) with boxes title "Volume"
EOF
        echo "Image générée : $image_sortie"
    fi

    cat "$csvfile"

# Action = leaks 
elif [ "$action" = "leaks" ]; then
    # Vérification qu'on a bien l'identifiant de l'usine en 3ème argument
    if [ $# -lt 3 ]; then
        echo "Erreur : L'action 'leaks' nécessite un identifiant d'usine."
        echo "Usage : $0 <fichier.dat> leaks \"Facility complex #ID\""
        exit 1
    fi

    usine_cible="$3"
    # Nettoyage du nom pour le fichier (on remplace espaces et # par des underscores)
    usine_clean=$(echo "$usine_cible" | tr ' #' '__')
    csvfile="$dirname/csv/leaks_$usine_clean.csv"

    echo "Analyse des fuites pour l'usine : $usine_cible"
    
    # Appel de la fonction de compilation pour leaks.c
    compile "leaks"

    # Exécution du programme C
    "$dirname/bin/leaks" "$datafile" "$csvfile" "$usine_cible"
    retour=$?

    if [ $retour -ne 0 ]; then
        echo "Erreur : Le programme leaks a échoué (Code retour: $retour)."
        exit 1
    fi

    # Affichage du résultat
    if [ -f "$csvfile" ]; then
        echo "Fichier de fuites généré : $csvfile"
        echo "--- Détails des fuites par source ---"
        cat "$csvfile"
    else
        echo "Erreur : Le fichier $csvfile n'a pas été créé. Vérifiez si l'identifiant d'usine existe dans le fichier .dat"
    fi
