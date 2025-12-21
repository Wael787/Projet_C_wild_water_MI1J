#!/bin/sh
# version finale avec BONUS
# date 21/12/2025

# Vérification des Paramètres
if [ $# -lt 2 ]; then
    echo "Usage :"
    echo "  $0 <fichier.dat> histo {max|src|reel|maxC|srcC|reelC|allC}"
    echo "  $0 <fichier.dat> leaks {id usine}"
    exit 1
fi

# Arguments d'entrée
program=$0
dirname=$(dirname "$program")
datafile="$dirname/dat/$1"
action=$2

start_time=$(date +%s%3N)
trap 'end_time=$(date +%s%3N); echo -e "\nDurée totale du script : $((end_time - start_time)) ms"' EXIT

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

    # traitement sur le shell (max, src, reel)
    if [ "$mode" = "max" ] || [ "$mode" = "src" ] || [ "$mode" = "reel" ]; then
        echo "Traitement via Shell/AWK ($mode)..."
        
        if [ "$mode" = "max" ]; then
            echo "=== Top 10 plus grandes usines ==="                       > $csvfile
            echo "identifier;max volume(k.m3.year-1)"                       >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3=="-" && $4!="-" && $5=="-" {print $2";"$4}' $datafile | sort -t';' -k2,2gr | head -10       >> $csvfile
            echo "=== Top 50 plus petites usines ==="                       >> $csvfile
            echo "identifier;max volume(k.m3.year-1)"                       >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3=="-" && $4!="-" && $5=="-" {print $2";"$4}' $datafile | sort -t';' -k2,2g | head -50       >> $csvfile
        elif [ "$mode" = "src" ]; then
            echo "=== Top 10 plus grandes usines ==="                       > $csvfile
            echo "identifier;src volume(k.m3.year-1)"                       >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {sum[$3]+=$4} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2nr |head -10  >> "$csvfile"
            echo "=== Top 50 plus petites usines ==="                       >> $csvfile
            echo "identifier;src volume(k.m3.year-1)"                       >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {sum[$3]+=$4} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2n |head -50  >> "$csvfile"
        else
            echo "=== Top 10 plus grandes usines ==="                         > $csvfile
            echo "identifier;reel volume(k.m3.year-1)"                       >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {res=$4*(100-$5)/100; sum[$3]+=res} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2nr |head -10 >> "$csvfile"
            echo "=== Top 50 plus petites usines ==="                         >> $csvfile
            echo "identifier;reel volume(k.m3.year-1)"                        >> $csvfile
            awk -F';' '$1=="-" && $2!="-" && $3!="-" && $4!="-" {res=$4*(100-$5)/100; sum[$3]+=res} END {for (k in sum) print k";"sum[k]}' "$datafile" | sort -t';' -k2,2n |head -50 >> "$csvfile"            
        fi
        
        cat $csvfile

    # traitement sur le C (maxC, srcC, reelC, allC) 
    elif [ "$mode" = "maxC" ] || [ "$mode" = "srcC" ] || [ "$mode" = "reelC" ] || [ "$mode" = "allC" ]; then
        echo "Traitement via Programme C/AVL ($mode)..."
        compile "histo"
        csvfile=$dirname/csv/histo_$mode.csv
        "$dirname/bin/$action" "$datafile" "$mode"
        retour=$?
        if [ $retour -ne 0 ]; then
            echo "Erreur : Le programme histo a échoué."
            exit 1
        fi
        
        # GNUPLOT pour modes simples
        if [ "$mode" != "allC" ] && command -v gnuplot >/dev/null 2>&1; then
            image_top10="$dirname/csv/histo_${mode}_top10.png"
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
            plot "< grep -v '==' '$csvfile' | head -n 11 | tail -n 10" using 2:xtic(1) with boxes title "Volume"
EOF
            echo "Image générée : $image_top10"
            
            image_top50=$dirname/csv/histo_${mode}_top50.png
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
            plot "< awk '/Top 50/ {flag=1} flag' '$csvfile' | grep -v '===' | tail -n +2" using 2:xtic(1) with boxes title "Volume"
EOF
            echo "Image générée : $image_top50"
        
        # GNUPLOT pour mode ALL (BONUS)
        elif [ "$mode" = "allC" ] && command -v gnuplot >/dev/null 2>&1; then
            image_top10="$dirname/csv/histo_${mode}_top10.png"
            echo "Génération du graphique cumulé top 10..."
            
            gnuplot <<EOF
            set terminal pngcairo size 1200,700 font "Arial,10"
            set output "$image_top10"
            set datafile separator ";"
            set style data histograms
            set style histogram rowstacked
            set style fill solid border -1
            set boxwidth 0.75
            set xtics rotate by -45 right
            set title "Top 10 - Volumes cumulés (max, source, réel)"
            set ylabel "Volume (k.m3/an)"
            set key outside right top
            
            # Extraction des données: max-source (vert), source-real (rouge), real (bleu)
            plot "< grep -v '===' '$csvfile' | head -n 11 | tail -n 10 | awk -F';' '{diff1=\$2-\$3; diff2=\$3-\$4; print \$1\";\"\$4\";\"diff2\";\"diff1}'" \
                 using 2:xtic(1) title "Volume réel (bleu)" linecolor rgb "blue", \
                 '' using 3 title "Pertes captage (rouge)" linecolor rgb "red", \
                 '' using 4 title "Capacité non utilisée (vert)" linecolor rgb "green"
EOF
            echo "Image générée : $image_top10"
            
            image_top50="$dirname/csv/histo_${mode}_top50.png"
            echo "Génération du graphique cumulé top 50..."
            
            gnuplot <<EOF
            set terminal pngcairo size 1200,700 font "Arial,10"
            set output "$image_top50"
            set datafile separator ";"
            set style data histograms
            set style histogram rowstacked
            set style fill solid border -1
            set boxwidth 0.75
            set xtics rotate by -45 right
            set title "Top 50 - Volumes cumulés (max, source, réel)"
            set ylabel "Volume (k.m3/an)"
            set key outside right top
            
            plot "< awk '/Top 50/ {flag=1} flag' '$csvfile' | grep -v '===' | tail -n +2 | awk -F';' '{diff1=\$2-\$3; diff2=\$3-\$4; print \$1\";\"\$4\";\"diff2\";\"diff1}'" \
                 using 2:xtic(1) title "Volume réel (bleu)" linecolor rgb "blue", \
                 '' using 3 title "Pertes captage (rouge)" linecolor rgb "red", \
                 '' using 4 title "Capacité non utilisée (vert)" linecolor rgb "green"
EOF
            echo "Image générée : $image_top50"
        fi

        cat "$csvfile"
    else
        echo "Mode inconnu: $mode"; exit 1
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
    csvfile="$dirname/csv/leaks_$usine_clean.csv"

    echo "Analyse des fuites pour l'usine : $usine_cible"
    
    compile "leaks"

    "$dirname/bin/leaks" "$datafile" "$csvfile" "$usine_cible"
    retour=$?

    if [ $retour -ne 0 ]; then
        echo "Erreur : Le programme leaks a échoué (Code retour: $retour)."
        exit 1
    fi

    if [ -f "$csvfile" ]; then
        echo "Fichier de fuites généré : $csvfile"
        echo "--- Détails des fuites ---"
        cat "$csvfile"
    else
        echo "Erreur : Le fichier $csvfile n'a pas été créé."
    fi
else
    echo "Action inconnue: $action"
    echo "Actions valides: histo, leaks"
    exit 1
fi
