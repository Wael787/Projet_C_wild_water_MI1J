#!/bin/sh
#date 21/12/2025

# Vérification des Paramètres
if [ $# -lt 2 ]; then
    echo "Usage :"
    echo "  $0 <fichier.dat> histo {max|src|reel|maxC|srcC|reelC}"
    echo "  $0 <fichier.dat> leaks {id usine}"
    exit 1
fi

# Arguments d'entrée
program=$0
dirname=$(dirname "$program")
filename=$1
datafile="$dirname/dat/$filename"
action=$2

start_time=$(date +%s%3N)
trap 'end_time=$(date +%s%3N); echo -e "\nDurée totale du script : $((end_time - start_time)) ms"' EXIT

# Controle existence du fichier de données
if [ ! -f "$datafile" ]; then
    echo "Erreur : fichier $datafile introuvable"
    exit 1
fi

# recréation des répértoires de génération des fichiers 
mkdir -p "$dirname/csv"
mkdir -p "$dirname/bin"
mkdir -p "$dirname/png"

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
    csvfile=$dirname/csv/${action}_$mode.csv

    # traitement sur le shell (max, src, reel)
    if [ "$mode" = "max" ] || [ "$mode" = "src" ] || [ "$mode" = "reel" ]; then
        echo "Traitement via Shell/AWK ($mode)..."
        
        if [ "$mode" = "max" ]; then
            echo "=== Top 10 plus grandes usines ==="                       >  $csvfile
            echo "identifier;max volume(k.m3.year-1)"                       >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3=="-" && $4!="-" && $5=="-" {print $2";"$4}' $datafile | sort -t';' -k2,2gr | head -10       >> $csvfile
            echo "=== Top 50 plus petites usines ==="                       >> $csvfile
            echo "identifier;max volume(k.m3.year-1)"                       >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3=="-" && $4!="-" && $5=="-" {print $2";"$4}' $datafile | sort -t';' -k2,2g | head -50        >> $csvfile
        elif [ "$mode" = "src" ]; then
            echo "=== Top 10 plus grandes usines ==="                       >  $csvfile
            echo "identifier;src volume(k.m3.year-1)"                       >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {sum[$3]+=$4} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2nr | head -10  >> $csvfile
            echo "=== Top 50 plus petites usines ==="                       >> $csvfile
            echo "identifier;src volume(k.m3.year-1)"                       >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {sum[$3]+=$4} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2n | head -50  >> "$csvfile"
        else
            echo "=== Top 10 plus grandes usines ==="                       > $csvfile
            echo "identifier;reel volume(k.m3.year-1)"                      >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {res=$4*(100-$5)/100; sum[$3]+=res} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2nr | head -10 >> $csvfile
            echo "=== Top 50 plus petites usines ==="                       >> $csvfile
            echo "identifier;reel volume(k.m3.year-1)"                      >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {res=$4*(100-$5)/100; sum[$3]+=res} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2n | head -50  >> $csvfile            
        fi
        
        cat $csvfile
        
        # Génération des graphiques
        if command -v gnuplot >/dev/null 2>&1; then
            image_top10="$dirname/png/${action}_${mode}_${filename}_top10.png"
            echo "Génération du graphique top 10..."
            gnuplot <<EOF
set terminal pngcairo size 1000,600 font "Arial,10"
set output "$image_top10"
set datafile separator ";"
set style fill solid
set boxwidth 0.5
set xtics rotate by -90
set title "Top 10 - Volumes ($mode)"
set ylabel "Volume (k.m3/an)"
plot "< grep -v '===' '$csvfile' | head -n 11 | tail -n 10" using 2:xtic(1) with boxes title "Volume"
EOF
            echo "Image générée : $image_top10"
            
            image_top50="$dirname/png/${action}_${mode}_${filename}_top50.png"
            echo "Génération du graphique top 50..."
            gnuplot <<EOF
set terminal pngcairo size 1000,600 font "Arial,10"
set output "$image_top50"
set datafile separator ";"
set style fill solid
set boxwidth 0.5
set xtics rotate by -90
set title "Top 50 - Volumes ($mode)"
set ylabel "Volume (k.m3/an)"
plot "< awk '/Top 50/ {flag=1; next} flag && NF>0' '$csvfile' | grep -v '===' | tail -n 50" using 2:xtic(1) with boxes title "Volume"
EOF
            echo "Image générée : $image_top50"
        fi
 
    # traitement sur le C (maxC, srcC, reelC) 
    elif [ "$mode" = "maxC" ] || [ "$mode" = "srcC" ] || [ "$mode" = "reelC" ]; then
        echo "Traitement via Programme C/AVL ($mode)..."
        
        # Appel de la fonction de compilation
        compile $action
        
        # Exécution du programme C
        $dirname/bin/$action $datafile $mode
        retour=$?
        if [ $retour -ne 0 ]; then
            echo "Erreur : Le programme $action a échoué."
            exit 1
        fi
        
        cat $csvfile
        
        # Génération des graphiques
        if command -v gnuplot >/dev/null 2>&1; then
            image_top10="$dirname/png/${action}_${mode}_${filename}_top10.png"
            echo "Génération du graphique top 10..."
            gnuplot <<EOF
set terminal pngcairo size 1000,600 font "Arial,10"
set output "$image_top10"
set datafile separator ";"
set style fill solid
set boxwidth 0.5
set xtics rotate by -90
set title "Top 10 - Volumes ($mode)"
set ylabel "Volume (k.m3/an)"
plot "< grep -v '===' '$csvfile' | head -n 11 | tail -n 10" using 2:xtic(1) with boxes title "Volume"
EOF
            echo "Image générée : $image_top10"
            
            image_top50="$dirname/png/${action}_${mode}_${filename}_top50.png"
            echo "Génération du graphique top 50..."
            gnuplot <<EOF
set terminal pngcairo size 1000,600 font "Arial,10"
set output "$image_top50"
set datafile separator ";"
set style fill solid
set boxwidth 0.5
set xtics rotate by -90
set title "Top 50 - Volumes ($mode)"
set ylabel "Volume (k.m3/an)"
plot "< awk '/Top 50/ {flag=1; next} flag && NF>0' '$csvfile' | grep -v '===' | tail -n 50" using 2:xtic(1) with boxes title "Volume"
EOF
            echo "Image générée : $image_top50"
        fi
    else
        echo "Mode inconnu: $mode"
        exit 1
    fi

# Action = leaks 
elif [ "$action" = "leaks" ]; then
    if [ $# -lt 3 ]; then
        echo "Erreur : L'action 'leaks' nécessite un identifiant d'usine."
        echo "Usage : $0 <fichier.dat> leaks \"Facility complex #ID\""
        exit 1
    fi

    usine_cible="$3"
    usine_clean=$(echo "$usine_cible" | tr ' #' '__')
    csvfile=$dirname/csv/${action}_${usine_clean}.csv

    echo "Analyse des fuites pour l'usine : $usine_cible"
    
    # Appel de la fonction de compilation
    compile $action

    # Exécution du programme C
    $dirname/bin/$action $datafile $csvfile "$usine_cible"
    retour=$?

    if [ $retour -ne 0 ]; then
        echo "Erreur : Le programme $action a échoué (Code retour: $retour)."
        exit 1
    fi

    # Affichage du résultat
    if [ -f "$csvfile" ]; then
        echo "Fichier de fuites généré : $csvfile"
        echo "--- Détails des fuites ---"
        cat $csvfile
    else
        echo "Erreur : Le fichier $csvfile n'a pas été créé."
    fi
else
    echo "Action inconnue: $action"
    echo "Actions valides: histo, leaks"
    exit 1
fi
