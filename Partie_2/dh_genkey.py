# -*- coding: utf-8 -*- 

import argparse
import secrets
from threading import Thread
from queue import Queue
from collections import namedtuple

Params = namedtuple('Params', ['g', 'p'])
NumbersAliceBob = namedtuple('NumbersAliceBob', ['AB', 'ab'])

queue_alice = Queue()
queue_bob = Queue()
queue_eve = Queue()

def getParams(fileName : str) -> Params:
    with open(fileName, 'r') as file:
        lines = file.readlines()
   
    params_dict = {}
    for line in lines:
        # Supprimer les espaces autour et ignorer les lignes vides
        line = line.strip()
        if not line or line.startswith('#'):  # Ignore les lignes vides ou les commentaires
            continue

        # Extraire la partie avant le '#' pour ignorer les commentaires
        key_value = line.split('#')[0].strip()

        # Séparer la clé et la valeur
        key, value = key_value.split(' = ')
        params_dict[key.strip()] = int(value.strip())
    
    # Extraire g et p à partir des clés 'generateur' et 'premier'
    # (L'affichage dans le fichier fileName)
    g = params_dict['generateur']
    p = params_dict['premier']

    params = Params(g = g, p = p)

    return params

# returns the big A or big B, known to everyone
def calcul_nombres(alic : bool, params : Params) -> NumbersAliceBob:
    # only alice knows a
    secret_a = secrets.randbelow(params.p - 1) + 1
    # everyone knows A
    known_A = pow(params.g, secret_a, params.p)

    # i need the 'a' to print the information
    nbs = NumbersAliceBob(AB = known_A, ab = secret_a)

    return nbs

# returns A and a, which is (g^a)[p], 'a' being generated randomly between 1 and p - 1
def alice(param : Params) -> NumbersAliceBob:
    return calcul_nombres(True, param)

# returns B, calling alice()
def bob(param : Params) -> NumbersAliceBob:
    return calcul_nombres(False, param)

# AB is A or B, ab is the secret 'a' or 'b' choosed by alice or bob
# returns the commun key, which is (AB ^ ab) [p]
def clef_commune(AB : int, ab : int, param : Params) -> int:
    return pow(AB, ab, param.p)



# thread being alice
def echange_depuis_alice(param : Params, fileKey : str):
    alicE = alice(param) # big A
    # to transfer to bob
    print(f"(Alice) Moi seule connais a = {alicE.ab}.\n")

    print(f"(Alice) Je calcul A : ({param.g} ^ {alicE.ab})[{param.p}] = {alicE.AB}\n")
    print(f"(Alice) Je transmet A = {alicE.AB} à Bob\n")

    queue_bob.put(alicE)
    queue_eve.put(alicE)

    print("(Alice) J'attend le nombre de Bob...\n")
    boB = queue_alice.get()

    print(f"(Alice) Je viens de recevoir B = {boB.AB} de la part de Bob\n")

    commun_key = clef_commune(boB.AB, alicE.ab, param)
    print(f"(Alice) Je peux alors calculer notre clef commune : {commun_key}\n")

    with open(fileKey, 'a') as file:
        file.write(f"(Alice) clef : {commun_key}\n")

    return


# thread being bob
def echange_depuis_bob(param : Params, fileKey : str):
    boB = bob(param) # big B
    # to transfer to alice
    print(f"(Bob) Moi seul connais b = {boB.ab}.\n")

    print(f"(Bob) Je calcul B : ({param.g} ^ {boB.ab})[{param.p}] = {boB.AB}\n")
    print(f"(Bob) Je transmet B = {boB.AB} à Alice\n")
    
    queue_alice.put(boB)
    queue_eve.put(boB)

    print("(Bob) J'attend le nombre d'Alice...\n")
    alic = queue_bob.get()

    print(f"(Bob) Je viens de recevoir A = {alic.AB} de la part d'Alice\n")

    commun_key = clef_commune(alic.AB, boB.ab, param)
    print(f"(Bob) Je peux alors calculer notre clef commune : {commun_key}\n")

    with open(fileKey, 'a') as file:
        file.write(f"(Bob) clef : {commun_key}\n")

    return


def eve():
    for i in range(2):
        print(f"(Eve) Je viens d'intercepté la valeur : {queue_eve.get().AB}\n")

    return


def simulation_echange_dh(fileParam : str, fileKey : str):
    params = getParams(fileParam)
    print(f"\nLe générateur g est {params.g} et le nombre premier p est {params.p}, ils sont connus de tous.\n")

    thread_alice = Thread(target = echange_depuis_alice, args= (params, fileKey))
    thread_bob = Thread(target = echange_depuis_bob, args= (params, fileKey))
    thread_eve = Thread(target = eve)

    thread_eve.start()
    thread_alice.start()
    thread_bob.start()

    thread_alice.join()
    thread_bob.join()
    thread_eve.join()

    print("Fin de la communication.\n")

    with open(fileKey, 'a') as file:
        file.write("\n")

    return

    

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Générateur de clés DH.")
    parser.add_argument("-i", "--input", required=True, help="Fichier d'entrée contenant les paramètres")
    parser.add_argument("-o", "--output", required=True, help="Fichier de sortie pour la clé générée")

    args = parser.parse_args()

    # Appeler la fonction principale avec les arguments
    simulation_echange_dh(args.input, args.output)
