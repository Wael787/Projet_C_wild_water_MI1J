#!/bin/sh


# Vérification des Parameters
if [ $# -lt 2 ]; then
        echo " Usage :"
        echo " $0 <fichier_donnees> histo {max|src|real}"
        echo " $0 <fichier_donnees> leaks {id usine}"
        exit 1
fi

# argument d entrée
program=$0
dirname=dirname $program
datafile=$dirname/dat/$1
action=$2

start_time=$(date +%s)
trap 'end_time=$(date +%s); echo "Durée totale du script : $((end_time - start_time)) s"' EXIT

# Control existance du fichier
if [ ! -f $datafile ]; then
        echo "Erreur : fichier $datafile introuvable"
        exit 1
fi;

#Compilation Program C
compile()
{
        program=$1

        if [ ! -x $dirname/bin/$program ]; then
                echo "Attention: fichier $dirname/bin/$program absent => compilation en cours ..."
                make
                retour=$?

                if [ $retour -ne 0 ]; then
                        echo "Erreur : compilation $dirname/bin/$program échouée."
                        exit 1
                fi
        fi
}

# Histogramme
if [ $action = "histo" ]; then
        if [ $# -ne 3 ]; then
                echo "Erreur: Histo nécessite un paramètre {max|src|real}"
                exit 1
        fi

        mode=$3
        csvfile=$dirname/csv/histo_$mode.csv

        compile $action

        $dirname/bin/$action $csvfile $mode
        retour=$?

        if [ $retour -ne 0 ]; then
                echo "Erreur : echec du traitement histgramme."
                exit 1
        fi

        echo "Histogramme généré : $csvfile"
        exit 0

# Leaks
elif [ $action = "leaks" ]; then
        if [ $# -ne 3 ]; then
                echo "Erreur: leaks nécessite un identifiant usine"
                exit 1
        fi

        usine=$3
        csvfile=$dirname/csv/leaks_$usine.csv

        compile $action

        $dirname/bin/$action $csvfile $mode
        retour=$?

        if [ $retour -ne 0 ]; then
                echo "Erreur : echec du traitement leaks."
                exit 1
        fi

        echo "Leaks généré : $csvfile"
        exit 0

else
        echo "Erreur : action inconnue $action"
        exit 1
fi
