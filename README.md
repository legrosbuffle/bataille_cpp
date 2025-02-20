# bataille_cpp

Cette page contient mon code pour l'article *Les défis du jeu de la bataille* de la revue [Pour La Science](https://www.pourlascience.fr/) (PLS 567 - janv 2025).

Les auteurs (Philippe Mathieu et Jean-paul Delahaye) ont mis a disposition un dépôt github avec leur code et les records actuels (https://github.com/cristal-smac/bataille).

Ce code est une implémentation du jeu décrit par l'article en C++, centrée sur la performance. Le code (un seul thread) tourne beaucoup plus rapidement que le code original des auteurs (12s pour une recherche exhaustive pour `C=4`, `V=4` sur mon ordinateur portable, au lieu de 46mn pour le code original, et 6s pour explorer aleatoirement 1M de parties pour `C=1`,`V=64` au lieu de 12h pour le code original), ce qui permet des explorations plus poussées. J'ai ainsi pu:

 - explorer exhaustivement le cas `C=4`,`V=5`, et vérifier que ce cas n'avait pas de cycles;
 - trouver des cycles pour le cas `C=4`, `V=11`. 
 - obtenir de nouveaux records de longueurs pour tous les jeux qui n'avaient pas été explorés exhaustivement.

Les résultats détaillés sont dans le dossier [`results/`](./results/).

## Utilisation

Le code est portable et ne nécessite aucune bibliothèque particulière. La compilation se fait simplement avec:

```
make explore
```

Le programme comprend deux modes d'exporation: `exhaustive` (pour les petites valeurs de `C` et `V`) et aléatoire (`random`).

Par exemple, pour faire une exploration exhaustive pour `C=1`,`V=12`:

```
./explore exhaustive 1 12
```

Les résultats sont écrits dans le fichier `c1v12.txt`.

Pour une exploration aléatoire pour `C=4`,`V=8` (jeu standard):

```
./explore random 4 8
```

Le mode `random` prend un paramètre optionnel `seed` en 4eme argument:

```
./explore random 4 8 123456789
```

Les résultats sont écrit dans le fichier `c4v8_<seed>.txt`

## Tests

Les tests nécessitent [googletest](https://github.com/google/googletest) et sont compilés en utilisant [bazel](https://bazel.build/).
