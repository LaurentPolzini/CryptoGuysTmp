# ProjetAvancé-Crypto
GROUPE 1 : 
BELKHDER ILIAS (partie 1)
MOHAMED BEN AOUN (partie 1)
POLZIN LAURENT (partie 3)
GBAKATCHETCHE YANN (partie 2)


Ce programme final regroupe les parties de chiffrement déchiffrement et crackage
Partie 1 : chiffrement / déchiffrement avec une clef via xor, cbc, mask
        Génération de clef aléatoire

Partie 3 : Trouver la clef qui a servi à chiffrer le message

Pour plus de précision veuillez consulter les REAMME appropriés

Pour compiler : make

Les commandes : 
Partie 1 :
./crypto -c chiffrement -m xor -i clair.txt -o crypte.txt -k clef (clair.txt s'encrypte dans crypte.txt avec la clef clef)
./crypto -c chiffrement -m mask-crypt ou mask-uncrypt -i clair.txt -o crypte.txt -k clef (si crypt et clef pas fournie : génération automatique de clef)
    (si uncrypt -k est OBLIGATOIRE)

Partie 3 :
./crypto -c crack -m mask -i msgsTests/msgMaskCrypte1.txt -r msgsTests/msgMaskCrypte2.txt -e msgsTests/msgMaskClair1.txt -o msgsTests/msgMaskClair2.txt
./crypto -c crack -m freq -i Partie_3/tests/1_Million/crypted_rutab-1_000_000.txt -d Partie_3/Dicos/dicoFrSA.txt -l 5
    Optionnels : -s Partie_3/keysScores.txt et -l Partie_3/logKeys.txt

Les messages de tests pour le crackage sont dans Partie_3/tests/
il y a des tests pour des nombres significatifs.
Pour 500 millions de clefs ca prend 10 minutes.

