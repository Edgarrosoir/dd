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
void ajouterPomme(tPlateau plateau, int indexPomme);// Choisir de manière aléatoire une position à l’intérieur du plateau non déjà occupée ni par un pavé ni par le serpent et y place une pomme
void afficher(int x, int y, char c); // Afficher le caractère c à la position (x, y)
void effacer(int x, int y); // Afficher un espace à la position (x, y)
void dessinerSerpent(int lesX[], int lesY[], int taille); // Afficher à l’écran un à un les éléments du serpent dont les coordonnées sont fournies dans le tableau en paramètre
void progresser(int lesX[], int lesY[], int *taille, char direction, tPlateau plateau, bool *collision, bool *pomme); // Calculer et afficher la prochaine position du serpent
char calculerDirection(int serpentX, int serpentY, int pommeX, int pommeY, char directionPrecedente); // Calculer la prochaine direction
void disable_echo(); // Éviter que les caractères utilisés pour diriger le serpent s’affichent à l’écran
void enable_echo(); // Réactiver l'écho
void gotoxy(int x, int y); // Positionner le curseur à un endroit précis

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
    clock_t begin = clock();

    // Désactiver l'écho pour les entrées utilisateur
    disable_echo();

    // Boucle principale
    while (!collision && nbPommesMangees < NB_POMMES) {
        direction = calculerDirection(lesX[0], lesY[0], lesPommesX[nbPommesMangees], lesPommesY[nbPommesMangees], direction);
        progresser(lesX, lesY, &tailleSerpent, direction, plateau, &collision, &pommeMangee);

        cpt++;

        if (pommeMangee) {
            nbPommesMangees++;
            if (nbPommesMangees < NB_POMMES) {
                ajouterPomme(plateau, nbPommesMangees);
            }
        }
        usleep(ATTENTE);
    }

    // Réactiver l'écho
    enable_echo();
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

    // Dernière ligne (trou au milieu)
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        if (i == LARGEUR_PLATEAU / 2) {
            plateau[i][HAUTEUR_PLATEAU] = VIDE; // Trou au milieu
        } 
        else {
            plateau[i][HAUTEUR_PLATEAU] = BORDURE;
        }
    }

    // Première colonne (trou au milieu)
    for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
        if (j == HAUTEUR_PLATEAU / 2) {
            plateau[1][j] = VIDE; // Trou au milieu
        } 
        else {
            plateau[1][j] = BORDURE;
        }
    }

    // Dernière colonne (trou au milieu)
    for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
        if (j == HAUTEUR_PLATEAU / 2) {
            plateau[LARGEUR_PLATEAU][j] = VIDE; // Trou au milieu
        } 
        else {
            plateau[LARGEUR_PLATEAU][j] = BORDURE;
        }
    }

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

void progresser(int lesX[], int lesY[], int *taille, char direction, tPlateau plateau, bool *collision, bool *pomme) {
    effacer(lesX[*taille - 1], lesY[*taille - 1]);

    for (int i = *taille - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

    switch (direction) {
        case 'z': lesY[0]--; break;
        case 's': lesY[0]++; break;
        case 'q': lesX[0]--; break;
        case 'd': lesX[0]++; break;
    }

    // Vérifier la proximité d'un portail et diriger le serpent vers celui-ci
    if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == 0) { // Portail haut
        // Le serpent se téléporte en bas
        lesY[0] = HAUTEUR_PLATEAU;
    } 
    else if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == HAUTEUR_PLATEAU + 1) { // Portail bas
        // Le serpent se téléporte en haut
        lesY[0] = 0;
    }
    else if (lesY[0] == HAUTEUR_PLATEAU / 2 && lesX[0] == 0) { // Portail gauche
        // Le serpent se téléporte à droite
        lesX[0] = LARGEUR_PLATEAU;
    } 
    else if (lesY[0] == HAUTEUR_PLATEAU / 2 && lesX[0] == LARGEUR_PLATEAU + 1) { // Portail droit
        // Le serpent se téléporte à gauche
        lesX[0] = 0;
    }

    // Gestion des trous (passages à travers les bords)
    if(lesX[0] <= 0){
        lesX[0] = LARGEUR_PLATEAU; // Passage à gauche
    } 

    if(lesX[0] > LARGEUR_PLATEAU){
        lesX[0] = 1;  // Passage à droite
    } 

    if(lesY[0] <= 0){
        lesY[0] = HAUTEUR_PLATEAU; // Passage en haut
    } 

    if(lesY[0] > HAUTEUR_PLATEAU){
        lesY[0] = 1;  // Passage en bas
    } 

    // Téléportation via les portails (au centre des côtés)
    if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == 1){ // Portail haut
        lesX[0] = LARGEUR_PLATEAU / 2;
        lesY[0] = HAUTEUR_PLATEAU; // Téléportation en bas
    } 
    
    else if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == HAUTEUR_PLATEAU){ // Portail bas
        lesX[0] = LARGEUR_PLATEAU / 2;
        lesY[0] = 1; // Téléportation en haut
    } 
    
    else if (lesX[0] == 1 && lesY[0] == HAUTEUR_PLATEAU / 2){ // Portail gauche
        lesX[0] = LARGEUR_PLATEAU;
        lesY[0] = HAUTEUR_PLATEAU / 2; // Téléportation à droite
    } 
    
    else if (lesX[0] == LARGEUR_PLATEAU && lesY[0] == HAUTEUR_PLATEAU / 2){ // Portail droit
        lesX[0] = 1;
        lesY[0] = HAUTEUR_PLATEAU / 2; // Téléportation à gauche
    }

    if (lesX[0] == 0) lesX[0] = LARGEUR_PLATEAU;
    if (lesX[0] > LARGEUR_PLATEAU) lesX[0] = 1;
    if (lesY[0] == 0) lesY[0] = HAUTEUR_PLATEAU;
    if (lesY[0] > HAUTEUR_PLATEAU) lesY[0] = 1;

    *collision = (plateau[lesX[0]][lesY[0]] == BORDURE || plateau[lesX[0]][lesY[0]] == CORPS);
    *pomme = (plateau[lesX[0]][lesY[0]] == POMME);

    if (*pomme) {
        plateau[lesX[0]][lesY[0]] = VIDE;
    }

    dessinerSerpent(lesX, lesY, *taille);
}

// Fonction pour vérifier si une position est un pavé
bool estPave(int x, int y) {
    for (int i = 0; i < NB_PAVES; i++) {
        if (x >= lesPavesX[i] && x < lesPavesX[i] + 5 && y >= lesPavesY[i] && y < lesPavesY[i] + 5) {
            return true;
        }
    }
    return false;
}

// Modification de la fonction calculerDirection pour éviter les pavés
char calculerDirection(int serpentX, int serpentY, int pommeX, int pommeY, char directionPrecedente) {
    int dx = pommeX - serpentX;
    int dy = pommeY - serpentY;
    char nouvelleDirection;

    if (abs(dx) > abs(dy)) {
        nouvelleDirection = (dx > 0) ? 'd' : 'q';
    } else {
        nouvelleDirection = (dy > 0) ? 's' : 'z';
    }

    // Vérifier si la nouvelle direction mène à un pavé
    int prochainX = serpentX;
    int prochainY = serpentY;

    switch (nouvelleDirection) {
        case 'd': prochainX++; break;
        case 'q': prochainX--; break;
        case 's': prochainY++; break;
        case 'z': prochainY--; break;
    }

    // Si la direction mène à un pavé, choisir une autre direction
    if (estPave(prochainX, prochainY)) {
        // Essayer les autres directions
        if (nouvelleDirection == 'd' || nouvelleDirection == 'q') {
            nouvelleDirection = (dy > 0) ? 's' : 'z';
        } else {
            nouvelleDirection = (dx > 0) ? 'd' : 'q';
        }
        // Recalculer les positions pour la nouvelle direction
        prochainX = serpentX;
        prochainY = serpentY;
        switch (nouvelleDirection) {
            case 'd': prochainX++; break;
            case 'q': prochainX--; break;
            case 's': prochainY++; break;
            case 'z': prochainY--; break;
        }
        // Si la nouvelle direction mène également à un pavé
        if (estPave(prochainX, prochainY)) {
            // Inverser encore une fois
            nouvelleDirection = (nouvelleDirection == 'd' || nouvelleDirection == 'q') ? ((dy > 0) ? 's' : 'z') : ((dx > 0) ? 'd' : 'q');
        }
    }

    return nouvelleDirection;
}

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

void disable_echo() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void enable_echo() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

