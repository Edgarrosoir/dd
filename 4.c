/*Programmation d'un jeu snake*/

/**
 * @file <version3.c>
 *
 * @brief <Programme jeu snake - Version 3>
 *
 * @author <Amélie JULIEN, Marie-Lisa SALAÜN>
 *
 * @version <version 3>
 *
 * @date <02/01/2025>
 *
 * < Déplacement autonome du serpent afin qu’il mange les 10 
 * pommes une à une, sans intervention de l’utilisateur. 
 * Le serpent doit pouvoir se diriger vers chaque pomme en faisant
 * le moins de déplacements possibles, en utilisant les portails 
 * de téléportation pour être plus efficace et en évitant les pavés. >
 */

// Bibliothèques
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

/******************************
 * DÉCLARATION DES CONSTANTES *
 ******************************/
#define TAILLE 10
#define LARGEUR_PLATEAU 80
#define HAUTEUR_PLATEAU 40
#define X_INITIAL 40
#define Y_INITIAL 20
#define NB_POMMES 10
#define NB_PAVES 6
#define ATTENTE 200000
#define CORPS 'X'
#define TETE 'O'
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'
#define STOP 'a'

// Coordonnées des pavés fixes
int lesPavesX[NB_PAVES] = {3, 74, 3, 74, 38, 38};
int lesPavesY[NB_PAVES] = {3, 3, 34, 34, 21, 15};

// Coordonnées des pommes fixes
int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {8, 39, 2, 2, 5, 39, 33, 38, 35, 2};

// Définition du plateau
typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];

/******************************
 * DÉCLARATION DES PROCÉDURES *
 ******************************/
void initPlateau(tPlateau plateau); // Initialiser le plateau de jeu
void dessinerPlateau(tPlateau plateau); // Afficher le plateau de jeu sur l'écran préalablement effacé
void ajouterPomme(tPlateau plateau, int indexPomme); // Ajouter une pomme au plateau de jeu
void afficher(int x, int y, char c); // Afficher le caractère c à la position (x, y)
void effacer(int x, int y); // Afficher un espace à la position (x, y)
void dessinerSerpent(int lesX[], int lesY[], int taille); // Afficher le serpent à l’écran
void progresser(int lesX[], int lesY[], int *taille, char *direction, tPlateau plateau, bool *collision, bool *pomme, int pommeX, int pommeY); // Calcule et affiche la prochaine position du serpent
char calculerDirection(int serpentX, int serpentY, int pommeX, int pommeY, char directionPrecedente, tPlateau plateau); // Calcule la prochaine direction
void gotoxy(int x, int y); // Positionner le curseur à un endroit précis
int kbhit(); // Vérifier si une touche a été pressée

/***********************
 * FONCTION PRINCIPALE *
 ***********************/
int main() {
    tPlateau plateau;
    int lesX[LARGEUR_PLATEAU * HAUTEUR_PLATEAU];
    int lesY[LARGEUR_PLATEAU * HAUTEUR_PLATEAU];
    char direction = 'd';
    bool collision = false;
    bool pommeMangee = false;
    int tailleSerpent = TAILLE;
    int nbPommesMangees = 0;
    int cpt = 0;
    char touche;

    clock_t begin = clock();


    // Initialisation du serpent
    for (int i = 0; i < tailleSerpent; i++) {
        lesX[i] = X_INITIAL - i;
        lesY[i] = Y_INITIAL;
    }

    // Initialisation du plateau
    initPlateau(plateau);
    system("clear");
    dessinerPlateau(plateau);
    ajouterPomme(plateau, nbPommesMangees);
    srand(time(NULL));
    

   while (!collision & nbPommesMangees < NB_POMMES) {
        if (kbhit()) {
            touche = getchar();
            if (touche == STOP) {
                break;
            }
        }

        progresser(lesX, lesY, &tailleSerpent, &direction, plateau, &collision, &pommeMangee, lesPommesX[nbPommesMangees], lesPommesY[nbPommesMangees]);

        cpt++;

        if (pommeMangee) {
            nbPommesMangees++;
            if (nbPommesMangees < NB_POMMES) {
                ajouterPomme(plateau, nbPommesMangees);
            }
        }
        usleep(ATTENTE);
    }
    gotoxy(1, HAUTEUR_PLATEAU + 2);

    printf("Nombre de déplacements : %d caractères.\n", cpt);

    clock_t end = clock();
    double tmpsCPU = ((end - begin) * 1.0) / CLOCKS_PER_SEC;
    printf("Temps CPU = %.2f secondes.\n", tmpsCPU);
    printf("Partie terminée. Pommes mangées : %d\n", nbPommesMangees);
    return 0;
}

/****************************
 * DÉFINITION DES FONCTIONS *
 ****************************/
void initPlateau(tPlateau plateau) {
    for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
        for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
            plateau[i][j] = VIDE;
        }
    }

    for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
        plateau[i][1] = plateau[i][HAUTEUR_PLATEAU] = BORDURE;
    }
    for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
        plateau[1][j] = plateau[LARGEUR_PLATEAU][j] = BORDURE;
    }

    plateau[LARGEUR_PLATEAU / 2][1] = VIDE;
    plateau[LARGEUR_PLATEAU / 2][HAUTEUR_PLATEAU] = VIDE;
    plateau[1][HAUTEUR_PLATEAU / 2] = VIDE;
    plateau[LARGEUR_PLATEAU][HAUTEUR_PLATEAU / 2] = VIDE;

    for (int p = 0; p < NB_PAVES; p++) {
        for (int dx = 0; dx < 5; dx++) {
            for (int dy = 0; dy < 5; dy++) {
                plateau[lesPavesX[p] + dx][lesPavesY[p] + dy] = BORDURE;
            }
        }
    }
}

void dessinerPlateau(tPlateau plateau) {
    for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
        for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
            afficher(i, j, plateau[i][j]);
        }
    }
}

void ajouterPomme(tPlateau plateau, int indexPomme) {
    int x = lesPommesX[indexPomme];
    int y = lesPommesY[indexPomme];
    plateau[x][y] = POMME;
    afficher(x, y, POMME);
}

void afficher(int x, int y, char c) {
    gotoxy(x, y);
    printf("%c", c);
    gotoxy(1, 1);
}

void effacer(int x, int y) {
    gotoxy(x, y);
    printf(" ");
    gotoxy(1, 1);
}

void dessinerSerpent(int lesX[], int lesY[], int taille) {
    for (int i = 1; i < taille; i++) {
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0], TETE);
}

void progresser(int lesX[], int lesY[], int *taille, char *direction, tPlateau plateau, bool *collision, bool *pomme, int pommeX, int pommeY) {
    effacer(lesX[*taille - 1], lesY[*taille - 1]);

    for (int i = *taille - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

    // Calcul de la direction avant de mettre à jour la position
    *direction = calculerDirection(lesX[0], lesY[0], pommeX, pommeY, *direction, plateau);

    // Calcul de la prochaine position en fonction de la direction
    int nextX = lesX[0];
    int nextY = lesY[0];
    switch (*direction) {
        case 'z': nextY--; break;
        case 's': nextY++; break;
        case 'q': nextX--; break;
        case 'd': nextX++; break;
    }

    // Vérification de la collision avec les pavés et ajustement de la direction si nécessaire
    if (plateau[nextX][nextY] == BORDURE || plateau[nextX][nextY] == CORPS) {
        // Essayer de trouver une nouvelle direction en vérifiant toutes les directions possibles
        char newDirection = *direction;
        if (*direction == 'z' || *direction == 's') {
            if (plateau[lesX[0] - 1][lesY[0]] != BORDURE && plateau[lesX[0] - 1][lesY[0]] != CORPS) {
                newDirection = 'q';
            } else if (plateau[lesX[0] + 1][lesY[0]] != BORDURE && plateau[lesX[0] + 1][lesY[0]] != CORPS) {
                newDirection = 'd';
            }
        } else if (*direction == 'q' || *direction == 'd') {
            if (plateau[lesX[0]][lesY[0] - 1] != BORDURE && plateau[lesX[0]][lesY[0] - 1] != CORPS) {
                newDirection = 'z';
            } else if (plateau[lesX[0]][lesY[0] + 1] != BORDURE && plateau[lesX[0]][lesY[0] + 1] != CORPS) {
                newDirection = 's';
            }
        }

        // Mise à jour de la direction
        *direction = newDirection;

        // Recalcul de la prochaine position en fonction de la nouvelle direction
        nextX = lesX[0];
        nextY = lesY[0];
        switch (*direction) {
            case 'z': nextY--; break;
            case 's': nextY++; break;
            case 'q': nextX--; break;
            case 'd': nextX++; break;
        }
    }

    // Mise à jour des coordonnées de la tête du serpent
    lesX[0] = nextX;
    lesY[0] = nextY;

    // Vérifier la proximité d'un portail et diriger le serpent vers celui-ci
    if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == 0) { // Portail haut
        lesY[0] = HAUTEUR_PLATEAU;
    } else if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == HAUTEUR_PLATEAU + 1) { // Portail bas
        lesY[0] = 0;
    } else if (lesY[0] == HAUTEUR_PLATEAU / 2 && lesX[0] == 0) { // Portail gauche
        lesX[0] = LARGEUR_PLATEAU;
    } else if (lesY[0] == HAUTEUR_PLATEAU / 2 && lesX[0] == LARGEUR_PLATEAU + 1) { // Portail droit
        lesX[0] = 0;
    }

    // Gestion des trous (passages à travers les bords)
    if (lesX[0] <= 0) {
        lesX[0] = LARGEUR_PLATEAU;
    } 
    if (lesX[0] > LARGEUR_PLATEAU) {
        lesX[0] = 1;
    } 
    if (lesY[0] <= 0) {
        lesY[0] = HAUTEUR_PLATEAU;
    } 
    if (lesY[0] > HAUTEUR_PLATEAU) {
        lesY[0] = 1;
    }

    *collision = (plateau[lesX[0]][lesY[0]] == BORDURE || plateau[lesX[0]][lesY[0]] == CORPS);
    *pomme = (plateau[lesX[0]][lesY[0]] == POMME);

    if (*pomme) {
        plateau[lesX[0]][lesY[0]] = VIDE;
    }

    dessinerSerpent(lesX, lesY, *taille);
}

char calculerDirection(int serpentX, int serpentY, int pommeX, int pommeY, char directionPrecedente, tPlateau plateau) {
    int dx = pommeX - serpentX;
    int dy = pommeY - serpentY;

    // Si la pomme est sur la même ligne que le serpent
    if (dy == 0) {
        if (dx > 0 && plateau[serpentX + 1][serpentY] != BORDURE && plateau[serpentX + 1][serpentY] != CORPS) {
            return 'd'; // droite
        } else if (dx < 0 && plateau[serpentX - 1][serpentY] != BORDURE && plateau[serpentX - 1][serpentY] != CORPS) {
            return 'q'; // gauche
        }
    }
    // Si la pomme est sur la même colonne que le serpent
    if (dx == 0) {
        if (dy > 0 && plateau[serpentX][serpentY + 1] != BORDURE && plateau[serpentX][serpentY + 1] != CORPS) {
            return 's'; // bas
        } else if (dy < 0 && plateau[serpentX][serpentY - 1] != BORDURE && plateau[serpentX][serpentY - 1] != CORPS) {
            return 'z'; // haut
        }
    }

    // Choisir la direction en fonction de la distance à la pomme
    if (abs(dx) > abs(dy)) {
        if (dx > 0 && plateau[serpentX + 1][serpentY] != BORDURE && plateau[serpentX + 1][serpentY] != CORPS) {
            return 'd'; // droite
        } else if (dx < 0 && plateau[serpentX - 1][serpentY] != BORDURE && plateau[serpentX - 1][serpentY] != CORPS) {
            return 'q'; // gauche
        }
    } else {
        if (dy > 0 && plateau[serpentX][serpentY + 1] != BORDURE && plateau[serpentX][serpentY + 1] != CORPS) {
            return 's'; // bas
        } else if (dy < 0 && plateau[serpentX][serpentY - 1] != BORDURE && plateau[serpentX][serpentY - 1] != CORPS) {
            return 'z'; // haut
        }
    }

    // Si aucune direction n'est possible, garder la direction précédente
    return directionPrecedente;
}

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}