# Décodeur JPEG en C

Ce projet implémente un décodeur d’images JPEG en langage C. Il permet d’extraire les informations essentielles d’un fichier JPEG (dimensions de l’image, tables de quantification, tables de Huffman, etc.), de décompresser les données d’image et de générer des fichiers en format `.pgm` (niveaux de gris) ou `.ppm` (couleur).

---

## 📁 Structure du projet

```
team17/
│
├── bin/                # Contient les exécutables
├── images/             # Images JPEG en entrée (pour les tests)
├── include/            # Fichiers d'en-tête (.h)
├── obj/                # Fichiers objets (.o) après compilation
├── src/                # Code source (.c)
├── dessins/            # Contient des schémas de visualisation utilisees dans le README
│
├── Makefile            # Fichier pour automatiser la compilation
├── README.md           # Ce fichier de documentation
├── *.pgm / *.ppm       # Fichiers image générés en sortie
```

---

## ⚙️ Compilation

Assurez-vous d’avoir `gcc` installé. Pour compiler le programme, utilisez la commande suivante dans le terminal à la racine du projet :

```bash
make
```

Cela génèrera les fichiers exécutables dans le dossier `bin/`.

---

## ▶️ Utilisation

Une fois compilé, le programme peut être exécuté depuis le dossier `bin/` :

```bash
./bin/jpeg2ppm mon_image.jpg
```

Le programme va :

1. Lire et analyser le fichier JPEG.
2. Extraire les tables nécessaires (quantification, Huffman, etc.).
3. Décompresser les données.
4. Générer une image `.pgm` ou `.ppm` en sortie.

---

## 🧪 Exemple d'utilisation

```bash
make
./bin/jpeg2ppm images/zig-zag.jpg
```

sortie: 
- `zig-zag.ppm`

---
## ▶️ Schémas pour mieux visualiser les structures et tableaux renvoyés par quelques fonctions

* La structure renvoyée par la fonction `extract_image_info`
![](dessins/infos.png)

* La structure renvoyée par la fonction `extract_quant_tables`
![](dessins/quant_tables.png)

*  Les structures `table_de_huffman` et `huffman_code`
![](dessins/structs.png)

*  La structure renvoyée par `extract_huffman_info` 
![](dessins/huff_info.png)

*  La structure renvoyée par `construction_arbre_huffman` 
![](dessins/result.png)

*  La structure renvoyée par `extract_huff_idx` 
![](dessins/huff_idx_tables.png)
