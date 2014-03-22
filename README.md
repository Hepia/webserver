Mini server web en C
===========================
L'objectif de ce projet est de réaliser un mini-serveur web en C sous Linux. Le serveur web devra fonctionner en même temps sur IPv4 et sur IPv6 sans redémarrer. Il ne dépendra que de la librairie GNU/C et des appels systèmes du noyaux Linux pour fonctionner.

## Contributeurs

Joachim Schmidt

Felipe Paul Martins

## Codes d'erreurs

### Socket 100

- `101` Le numéro de port doit être supérieur à 1'024
- `102` Le numéro de port doit être inférieur à 65'535
- `103` Erreur de `getaddrinfo(3)`
- `104` Erreur de `bind(2)`