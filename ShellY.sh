#!/bin/bash
set -euo pipefail #Securise le script

debut_ms=$(date +%s%3N) #Si %3N non supporté on utilise : (( $(date +%s) * 1000 ))
afficher_duree() {
	fin_ms=$(date +%s%3N) #Si %3N non supporté on utilise : (( $(date +%s) * 1000 ))
	echo "Durée totale : $((fin_ms - debut_ms)) ms"
}
trap afficher_duree EXIT #Fonction qui à la fin du script, appelle la fonction afficher_duree ( quand il y a un exit)

quitter_erreur() {
	echo "Erreur : $*" >&2
	exit 1
}

#Fonction qui veriife si la commande existe ca continue le script sinon appelle fonction quitter et quitte le script
verifier_commande() {
	command -v "$1" >/dev/null 2>&1 || quitter_erreur "Commande manquante : $1"
}

if [[ $# -lt 3 ]]; then
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
		max) ;;
		src) ;;
		real) ;;
		*)quitter_erreur "Option histo invalide : $option (attendu : max/src/real) " ;;
	esac

	#Outils nécessaire pour histo
	verifier_commande awk
	verifier_commande make
	verifier_commande sort
	verifier_commande head
	verifier_commande tail
	verifier_commande gnuplot

	#On vérifie si l’exécutable C est présent et exécutable
	executable_c="./c-wildwater"
	if [[ ! -x "$executable_c" ]]; then
		echo "[INFO] Executable C introuvable --> compilation via make"
		make || quitter_erreur "Compilation échouée"
	fi

	#Normalement on en a plus besoin ca maintenant le fichier de données est envoyer dans la commande de l'utilisateur
	#Dataset (fichier d'entrée)
	#dataset_defaut="c-wildwater_v0.dat"
	#dataset="${DATASET:-$dataset_defaut}"
	#if [[ ! -f "$dataset" ]]; then
	#	quitter_erreur "Fichier de données introuvable : $dataset"
	#fi

	#Noms de sortie : On construit dynamiquement les noms des fichiers de sortie à partir de l’option choisie afin de différencier clairement les résultats (selon max/src/real) 
	nom_base_sortie="histo_${option}"
	fichier_sortie_dat="${nom_base_sortie}.dat"
	image_petit50="${nom_base_sortie}_petit50.png"
	image_grand10="${nom_base_sortie}_grand10.png"

	#Appel du C + filtre CSV : il calcule et écrit le .dat
	echo "[INFO] Génération des données via C --> $fichier_sortie_dat"	#indique à l’utilisateur ce qui se passe montre le nom du fichier qui va être produit
	dataset_filtre="$(mktemp)"	#crée un fichier temporaire unique

	#on garde que les usines seules Pour histo max ( -;usine;-;capcité;- )
	if [[ "$option" == "max" ]]; then 
		awk -F';' '$1=="-" && $2!="-"
				&& $3=="-" && $4!="-"
				&& $5=="-" {print}' "$dataset" > "$dataset_filtre"

	#On garde que source->usine Pour histo src ou real ( -;source;usine;volume;%fuites )
	else 
		awk -F';' '$1=="-" && $2!="-"
				&& $3!="-" && $4!="-"
				&& $5!="-" {print}' "$dataset" > "$dataset_filtre"
	fi

	#Si fichier vide ...
	if [[ ! -s "$dataset_filtre" ]]; then
		rm -f "$dataset_filtre"
		quitter_erreur "Filtrage incorrecte : aucune ligne conservée (awk)"
		
	fi

	#Appel du programme C 
	"$executable_c" histo "$option" "$dataset_filtre" "$fichier_sortie_dat"

	#Verification du code retour (0 -> succèes sinon erreur)
	code_retour=$?

	#Si erreur (code_retour != 0) ...
	if ((code_retour != 0)); then
		rm -f "$dataset_filtre"
		quitter_erreur "Le programme C a échoué (code retour = $code_retour)"
	fi

	#Verif du .dat  (resultat C)
	if [[ ! -s "$fichier_sortie_dat" ]]; then
		rm -f "$dataset_filtre"
		quitter_erreur "Fichier de sortie vide : $fichier_sortie_dat "
	fi

	... Gnuplot ...

#Bloc leaks

if [[ "$commande" == "leaks" ]]; then
	if [[ "$#" -ne 3 ]]; then
		quitter_erreur "Usage : $0 <fichier.csh> leaks 'Identifiant usine' "
	fi

	Identifiant_usine="$3"

	#Verification outils nécessaire
	verifier_commande awk
	verifier_commande make
