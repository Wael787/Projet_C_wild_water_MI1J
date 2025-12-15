#!/bin/bash
set -euo pipefail #Securise le script

debut_secondes=$(date +%s)
afficher_duree() {
	fin_secondes=$(date +%s)
	echo "Durée totale : $((fin_secondes - debut_secondes)) s"
}
trap afficher_duree EXIT

quitter_erreur() {
	echo "Erreur : $*" >&2
	exit 1
}

verifier_commande() {
	command -v "$1" >/dev/null 2>&1 || quitter_erreur "Commande manquante : $1"
}

if [[ $# -lt 2 ]]; then
	quitter_erreur "Usage: $0 <fichier.csv> histo {max|src|real} OU $0 <fichier.csv> leaks 'Nom de l'usine' "
fi

dataset="$1"	#Nom fichier de donées
commande="$2"	# "histo" ou "leak"

if [[ ! -f "$dataset" ]]; then
	quitter_erreur "Fichier de données introuvable : $dataset"
fi

case "$commande" in
	histo|leaks) ;;
	*)quitter_erreur "Commande inconnue : $commande (attendu : histo ou leaks) " ;;
esac


#Cas histo
if [[ "$commande" == "histo" ]]; then	#Cas histo (on veut donc 2 arguments histo + max/src/real)
	if [[ $# -ne 3 ]]; then
		quitter_erreur "Usage : $0 <fichier.csv> histo {max/src/real} "
	fi
	
	option="$3"		# "max"|"src"|"real"

	case "$option" in
		max|src|real);;
		*)quitter_erreur "Option histo invalide : $option (attendu : max/src/real) ";;
	esac

	#Outils nécessaire pour histo
	verifier_commande make
	verifier_commande sort
	verifier_commande head
	verifier_commande tail
	verifier_commande gnuplot

	#Compilation du programme C si necessaire(via make)
	executable_c="./c-wildwater"
	if [[ ! -x "$executable_c" ]]; then
		echo "[INFO] Executable C introuvable --> compilation via make"
		make || quitter_erreur "Compilation échouée"
	fi

	
	#Noms de sortie (distincts selon max/src/real)
	nom_base_sortie="histo_${option}"
	fichier_sortie_dat="${nom_base_sortie}.dat"
	image_petit50="${nom_base_sortie}_petit50.png"
	image_grand10="${nom_base_sortie}_grand10.png"

	#Appel du C : il calcule et écrit le .dat
	echo "[INFO] Génération des données via C --> $fichier_sortie_dat"
	#Filtre du CSV 
	#(filtre des données pour prendre que les lignes source=>usine
	#cat $datafile | grep ^- | grep -v ";-;" > $datafile.new)
	#REFAIRE et revoir condition du FILTRE AVEC grep ou awk 
	dataset_filtre="$(mktemp)"
	grep "Plant #" "$dataset" > "$dataset_filtre"
	if [[ ! -s "$dataset_filtre" ]]; then
		rm -f "$dataset_filtre"
		quitter_erreur "Filtrage incorrecte : aucune ligne conservée (grep Plant #)"
		
	fi
	"$executable_c" histo "$option" "$dataset_filtre" "$fichier_sortie_dat"

	code_retour=$?

	if ((code_retour != 0)); then
		quitter_erreur "Le programme C a échoué (code retour = $code_retour)"
	fi

	if [[ ! -s "$fichier_sortie_dat" ]]; then
		quitter_erreur "Fichier de sortie vide : $fichier_sortie_dat "
	fi

	... Gnuplot ...

#Bloc leaks

if [[ "$commande" == "leaks" ]]; then
