elif [ "$action" = "leaks" ]; then
    if [ $# -lt 3 ]; then
        echo "Erreur : Identifiant usine manquant."
        exit 1
    fi

    usine_id="$3"
    hist_file="$dirname/csv/rendements_historique.dat"
    temp_csv="$dirname/csv/temp_leaks.csv"

    # Vérification et compilation automatique via Makefile
    if [ ! -x "$dirname/bin/leaks" ]; then
        make -C "$dirname"
    fi

    # Exécution du programme C
    "$dirname/bin/leaks" "$datafile" "$temp_csv" "$usine_id"
    retour=$?

    if [ $retour -gt 0 ]; then
        echo "Erreur programme C : $retour"
        exit 1
    fi

    # Mise à jour de l'historique (mode append)
    if [ ! -f "$hist_file" ]; then
        echo "identifier;Leak volume (M.m3.year-1)" > "$hist_file"
    fi
    if [ -f "$temp_csv" ]; then
        cat "$temp_csv" >> "$hist_file"
        cat "$temp_csv"
        rm "$temp_csv"
    fi
fi
