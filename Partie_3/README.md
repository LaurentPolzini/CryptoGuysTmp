# ProjetAvancé-Crypto
Auteur : POLZIN LAURENT

Cette 3ème partie porte sur le crackage d'un texte, autrement dit, tenter de trouver la clef ayant permit le chiffrage d'un texte.
On utilisera 3 méthodes : Une première méthode (c1) énumérant chaque clef possible.
Une seconde méthode (c2) utilisera toutes les clefs générées afin de traduire le texte
et de lui associer des fréquences de lettres. Ces fréquences sont comparées aux fréquences de la langue cible.
Une troisième et derniere méthode (c3) utilisera les meilleures clefs filtrées par c2
et assignera a chaque message traduit un poid étant le nombre de mots du texte présent dans le dictionnaire
de la langue cible. La meilleure clef est supposément la clef utilisée (il est peu probable qu'un texte
autre que l'original possède plus de mots présents.)

pour compiler : make
make clean
make proper
make DEBUG=yes
make asan (mais si vous avez valgrind utilisez le, clang marche pas pour les ressources pas free'd)


Pour chaque commandes, on supposera qu'on est dans le dossier Partie_3

Commandes optionnelles : 
-s fichier_scores (a chaque clef sera assigné un score)
    sauf si on lance c3, les scores enregistrés sont ceux de c2

-l logFile (on trouvera dans ce dernier les meilleurs scores des tableaux ainsi que le temps
de génération des clefs)



Pour lancer c1 seulement (simple génération de clef)
./break_code -m c1 -i tests/1_Million/crypted_rutab-1_000_000.txt -k 5

Pour lancer all : (sur la taille de la clef vue comme un maximum)
./break_code -m all -i tests/1_Million/crypted_rutab-1_000_000.txt -k 5 -d Dicos/dicoFrSA.txt

c2 : 
./break_code -m c2 -i tests/1_Million/crypted_rutab-1_000_000.txt -k 5
    ATTENTION : -t pour indiquer les statistiques a utiliser
    de base ce sont les stats francaises, pour les anglaises il faut
    ecrire -t english

c3 : 
./break_code -m c3 -i tests/1_Million/crypted_rutab-1_000_000.txt -k 5 -d Dicos/dicoFrSA.txt


Pour lancer les tests : ./break_code -e
(les tests sont dans tests_crackage.c)

Pour le crackage masque :
./crack mask chif1.txt chif2.txt test2.txt clair.txt 

Il faut faire attention à ce que les fichiers soient bien de la même taille



Remarques et points intéressants : 

On observe rapidement la non perspicacité de c2 : 
    les fréquences dépendent trop de la longueur du texte
    et du style d'écriture. Ainsi, la clef réelle peut très vite tomber très bas
    (texte en clair a 150 de distance -> laisse beaucoup de marge
        pour des clefs traduisant un texte a 70 de distance par exemple)

    Le c3 quant à lui est assez puissant mais dans de rares cas il peut ne pas être
    efficace : un mot est une suite de caractère alphabétique
    un "non mot" est tout ce qui n'est pas alphabétique
    autrement dit /a.a_a%a est décomposé en : 
    char **mots = [a, a, a, a] -> 4 mots
    or la lettre a est considéré comme un mot (dans le dictionnaire)
    imaginons le texte réel : abaissai -> 1 mots
    ainsi, sur de rares cas comme celui ci, c3 ne serait pas efficace.
    Toujours est-il que la clef réelle devrait se trouver assez haut dans le classement.



Points d'amélioration (notament sur l'optimisation)

On traite (c3) 1 milliard de clef en 20 minutes, ce qui est vachement long,
il faut donc optimiser :

    - La différence entre la clef i et la clef i + 1 est généralement de 1 caractère
        ainsi, il ne faudrait pas retraduire le message a chaque fois 
        mais seulement les lettres changées
    - De même pour c2 : il ne faudrait pas recalculer les fréquences des toutes les lettres
        mais uniquement celles modifiées.
    - Pour c3 : je ne vois pas : une lettre peut modifier un mot (tout recalculer a chaque fois)
    - Pour la génération des clefs : peut être long d'avoir des boucles dans des boucles
        mais je ne sais pas trop comment changer ce système.



On trouve aussi d'autres méthodes (les méthodes utilisées au début)
dans le dossier autres_methodes. Elles ne sont probablement pas toutes a jour
je n'ai pas eu le temps de revenir dessus. Au début du fichier.c chacune présente son idée
et son amélioration suivante

c1_stockingKeys.c fonctionne (les clefs sont stockées,
pour faire simple je met dans un tableau la premiere ligne de caractères candidats
puis a chaque caractère j'ajoute la seconde ligne, etc...)
ce qui nous permet d'observer le nombre de clef possible augmenter exponentiellement,
en effet s'il y a trop de clef la RAM ne peut supporter de telles charges
A partir de là j'ai donc du trouver une nouvelle solution, ce qui m'a mené
à la génération de clef "à la volée" et de les traiter avec un functor
(J'ai aussi voulu faire la meme chose mais avec un fichier plutot qu'un tableau
mais bon la lecture et ecriture dans un fichier aurait était bien trop long)

