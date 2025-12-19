elif [ "$action" = "leaks" ]; then
    if [ $# -lt 3 ]; then
        echo "Erreur : Identifiant usine manquant."
        exit 1
    fi

    usine_id="$3"
    # S'assurer que le dossier csv existe
    mkdir -p "$dirname/csv"
    
    hist_file="$dirname/csv/rendements_historique.dat"
    temp_csv="$dirname/csv/temp_leaks.csv"

    # Vérification et compilation automatique via Makefile
    if [ ! -x "$dirname/bin/leaks" ]; then
        make -C "$dirname"
        if [ $? -ne 0 ]; then echo "Erreur : Échec de la compilation"; exit 1; fi
    fi

    # Exécution du programme C
    # $datafile = CSV source | $temp_csv = Sortie du C | $usine_id = Paramètre
    "$dirname/bin/leaks" "$datafile" "$temp_csv" "$usine_id"
    retour=$?

    if [ $retour -gt 0 ]; then
        echo "Erreur programme C : Code $retour (Usine introuvable ou erreur mémoire)"
        exit 1
    fi

    # Mise à jour de l'historique
    if [ ! -f "$hist_file" ]; then
        echo "identifier;Leak volume (M.m3.year-1)" > "$hist_file"
    fi

    if [ -f "$temp_csv" ]; then
        # On ajoute le résultat à l'historique
        cat "$temp_csv" >> "$hist_file"
        # On affiche le résultat à l'écran pour l'utilisateur
        echo "Résultat pour $usine_id :"
        cat "$temp_csv"
        # On nettoie
        rm "$temp_csv"
    fi
fi
