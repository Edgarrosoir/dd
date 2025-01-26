#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <math.h>

/******************************
*  Constantes                *
*                             *
****************************** */
#define LARGEUR_PLATEAU 80     ///< Largeur du plateau
#define HAUTEUR_PLATEAU 40     ///< Hauteur du plateau
#define TAILLE 10              ///< Taille fixe du serpent
#define NB_POMMES 10           ///< Nombre total de pommes à  manger
#define ATTENTE 200000          ///< Temporisation entre deux déplacements (en microsecondes)
#define CORPS 'X'              ///< Représentation des anneaux du serpent
#define TETE 'O'               ///< Représentation de la tête du serpent
#define BORDURE '#'            ///< Représentation des bordures du plateau
#define PAVE '#'               ///< Représentation des pavés du plateau
#define NB_PAVES 6             ///< Nombre total de pavés
#define TAILLE_PAVE_X 5        ///< Taille fixe en abscisse du pavé
#define TAILLE_PAVE_Y 5        ///< Taille fixe en ordonnée du pavé
#define VIDE ' '               ///< Représentation des espaces vides
#define POMME '6'              ///< Représentation d'une pomme
#define STOP 'a'               ///< Touche pour arrêter le jeu

typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];

// Prototypes des fonctions
void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void afficher(int x, int y, char car);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
bool collision(int x, int y, int lesX[], int lesY[]);
void progresser(int lesX[], int lesY[], int cibleX, int cibleY, tPlateau plateau, bool *pomme);
void gotoxy(int x, int y);
void finProgramme(int nbDeplacements, clock_t tempsDebut, clock_t tempsFin);
int kbhit();
void calculerDistanceOptimale(int serpentX, int serpentY, int pommeX, int pommeY, int *nouvelleX, int *nouvelleY, bool *utilisePortail);

/**************************************
*                                     *
*         Programme Principale        *
*                                     *
*                                     *
************************************** */
/**
 * @brief Programme principal du jeu.
 *
 * Cette fonction gare le jeu, le plateau, le serpent, les pommes, et le mouvement du serpent.
 * Elle gare également l'affichage du nombre de déplacements et du temps d'exécution du jeu.
 *
 * @return EXIT_SUCCESS si le programme s'est exécuté avec succas.
 */
int main() {
    tPlateau lePlateau;
    int lesX[TAILLE], lesY[TAILLE];
    int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
    int lesPommesY[NB_POMMES] = {8, 39, 2, 2, 5, 39, 33, 38, 35, 2};
    int nbDeplacements = 0;
    int indexPomme = 0;
    bool pommeMangee = false;
    char touche;

    clock_t tempsDebut = clock();

    for (int i = 0; i < TAILLE; i++) {
        lesX[i] = 40 - i;
        lesY[i] = 20;
    }

    initPlateau(lePlateau);
    dessinerPlateau(lePlateau);
    afficher(lesPommesX[indexPomme], lesPommesY[indexPomme], POMME);
    dessinerSerpent(lesX, lesY);

    while (indexPomme < NB_POMMES) {
        if (kbhit()) {
            touche = getchar();
            if (touche == STOP) {
                break;
            }
        }

        progresser(lesX, lesY, lesPommesX[indexPomme], lesPommesY[indexPomme], lePlateau, &pommeMangee);
        nbDeplacements++;
        usleep(ATTENTE);

        if (pommeMangee) {
            indexPomme++;
            pommeMangee = false;
            if (indexPomme < NB_POMMES) {
                afficher(lesPommesX[indexPomme], lesPommesY[indexPomme], POMME);
            }
        }
    }

    clock_t tempsFin = clock();
    finProgramme(nbDeplacements, tempsDebut, tempsFin);
    return EXIT_SUCCESS;
}

/**********************************
*                                 *
*       Fonctions et procédure    *                          
*                                 *
*                                 *
********************************* */
/**
 * @brief Initialise le plateau avec les bordures et les portails.
 *
 * Cette fonction crée un plateau de jeu avec des bordures (le caractare '#')
 * et laisse les zones internes vides (le caractare ' '). Elle supprime aussi
 * les bordures centrales pour simuler des portails.
 *
 * @param plateau tableau représentant le plateau de jeu.
 * @param LePavesX coordonnées X des pavés
 * @param lesPavesY coordonnées Y des pavés
 */
void initPlateau(tPlateau plateau) {
    int lesPavesX[NB_PAVES] = { 3, 74, 3, 74, 38, 38};
    int lesPavesY[NB_PAVES] = { 3, 3, 34, 34, 21, 15};

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

    // définition des pavés
    for (int indicePave=0; indicePave < NB_PAVES; indicePave++){
        for (int largeur=0; largeur < TAILLE_PAVE_X; largeur++){
            for (int hauteur=0; hauteur < TAILLE_PAVE_Y; hauteur ++){
                plateau[lesPavesX[indicePave]+largeur][lesPavesY[indicePave]+hauteur]= PAVE;
            }
        }
    }
}

/**
 * @brief Calcule la distance optimale vers une pomme en tenant compte des portails.
 *
 * Cette fonction calcule la distance la plus courte entre le serpent et une pomme,
 * en tenant compte de la possibilité d'utiliser les portails pour se téléporter.
 *
 * @param serpentX La position X du serpent.
 * @param serpentY La position Y du serpent.
 * @param pommeX La position X de la pomme.
 * @param pommeY La position Y de la pomme.
 * @param nouvelleX Pointeur vers la nouvelle position X du serpent.
 * @param nouvelleY Pointeur vers la nouvelle position Y du serpent.
 * @param utilisePortail Pointeur vers une variable booléenne indiquant si un portail est à utilisé.
 */
void calculerDistanceOptimale(int serpentX, int serpentY, int pommeX, int pommeY, int *nouvelleX, int *nouvelleY, bool *utilisePortail) {
    // Coordonnées des portails
    int portailGaucheX = 1, portailGaucheY = HAUTEUR_PLATEAU / 2;
    int portailDroitX = LARGEUR_PLATEAU, portailDroitY = HAUTEUR_PLATEAU / 2;
    int portailHautX = LARGEUR_PLATEAU / 2, portailHautY = 1;
    int portailBasX = LARGEUR_PLATEAU / 2, portailBasY = HAUTEUR_PLATEAU;

    // Distance directe entre le serpent et la pomme
    int distanceDirecte = abs(pommeX - serpentX) + abs(pommeY - serpentY);

    // Distances via les portails
    int distanceViaGauche = abs(serpentX - portailGaucheX) + abs(serpentY - portailGaucheY) +
                            abs(portailDroitX - pommeX) + abs(portailDroitY - pommeY)-1;
    int distanceViaDroit = abs(serpentX - portailDroitX) + abs(serpentY - portailDroitY) +
                           abs(portailGaucheX - pommeX) + abs(portailGaucheY - pommeY)+1;
    int distanceViaHaut = abs(serpentX - portailHautX) + abs(serpentY - portailHautY) +
                          abs(portailBasX - pommeX) + abs(portailBasY - pommeY)+1;
    int distanceViaBas = abs(serpentX - portailBasX) + abs(serpentY - portailBasY) +
                         abs(portailHautX - pommeX) + abs(portailHautY - pommeY)-1;

    // Distance direct avec les pavées
    /*
    il faut que:
    si pommeX<PAVE[x]<serpentX OU pommeX>PAVE[x]>serpentX faire
        si pommeY<PAVE[x]<serpentX OU pommeY>PAVE[x]>serpentX faire
            nouvelle mesure
        fin faire
    fin faire
    
    nouvelle mesure:
    De quel côté ou comment contourner le pave ?
    Est-ce que c'est optimal ?  
    */


    // Initialisation de la distance minimale
    int distanceMin = distanceDirecte;
    *nouvelleX = pommeX;
    *nouvelleY = pommeY;
    *utilisePortail = false;

    // Comparaison avec les distances via les portails
    if (distanceViaGauche < distanceMin) {
        distanceMin = distanceViaGauche;
        *nouvelleX = portailGaucheX-1;  // Se diriger vers le portail gauche
        *nouvelleY = portailGaucheY;
        *utilisePortail = true;
    }
    if (distanceViaDroit < distanceMin) {
        distanceMin = distanceViaDroit;
        *nouvelleX = portailDroitX+1;  // Se diriger vers le portail droit
        *nouvelleY = portailDroitY;
        *utilisePortail = true;
    }
    if (distanceViaHaut < distanceMin) {
        distanceMin = distanceViaHaut;
        *nouvelleX = portailHautX;  // Se diriger vers le portail haut
        *nouvelleY = portailHautY+1;
        *utilisePortail = true;
    }
    if (distanceViaBas < distanceMin) {
        distanceMin = distanceViaBas;
        *nouvelleX = portailBasX;  // Se diriger vers le portail bas
        *nouvelleY = portailBasY-1;
        *utilisePortail = true;
    }
}


// Fonction pour vérifier les collisions avec le corps du serpent
bool collision(int x, int y, int lesX[], int lesY[]) {
    bool res = false;
    for (int i = 1; i < TAILLE; i++) {
        if (lesX[i] == x && lesY[i] == y) {
            res = true; // Il y a une collision
            break; // Sort de la boucle das qu'une collision est détectée
        }
    }
    return res;
}

/**
 * @brief Déplace le serpent vers la cible (pomme ou portail).
 *
 * Cette fonction déplace la tête du serpent en fonction de la cible (pomme ou portail)
 * et déplace ensuite le corps en suivant la tête.
 *
 * @param lesX Tableau des positions X des segments du serpent.
 * @param lesY Tableau des positions Y des segments du serpent.
 * @param cibleX Position X de la cible (pomme ou portail).
 * @param cibleY Position Y de la cible (pomme ou portail).
 * @param plateau Le plateau de jeu.
 * @param pomme Pointeur vers une variable booléenne indiquant si la pomme est mangée.
 */
void progresser(int lesX[], int lesY[], int cibleX, int cibleY, tPlateau plateau, bool *pomme) {
    int prochainX = cibleX, prochainY = cibleY;
    bool utilisePortail = false;

    // Déterminer la cible optimale (directe ou via un portail)
    calculerDistanceOptimale(lesX[0], lesY[0], cibleX, cibleY, &prochainX, &prochainY, &utilisePortail);

    // Efface le dernier segment du serpent
    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);

    // Déplace les segments du corps
    for (int i = TAILLE - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }
    // si un portail est à utiliser utilisePortail=true
    if (utilisePortail) {
        // se déplace en choisissant le chemin optimal à utiliser ici il est plus optimiser d'aller vers le haut cela réduit le nombre de déplacement
        bool endroitBloque=true;
        if (lesY[0] < prochainY && plateau[lesX[0]][lesY[0] + 1] != BORDURE && !collision(lesX[0], lesY[0] + 1, lesX, lesY)) {
            lesY[0]++;
            endroitBloque=false;
        } else if (lesY[0] > prochainY && plateau[lesX[0]][lesY[0] - 1] != BORDURE && !collision(lesX[0], lesY[0] - 1, lesX, lesY)) {
            lesY[0]--;
            endroitBloque=false;
        } else if (lesX[0] < prochainX && plateau[lesX[0] + 1][lesY[0]] != BORDURE && !collision(lesX[0] + 1, lesY[0], lesX, lesY)) {
            lesX[0]++;
            endroitBloque=false ;
        } else if (lesX[0] > prochainX && plateau[lesX[0] - 1][lesY[0]] != BORDURE && !collision(lesX[0] - 1, lesY[0], lesX, lesY)) {
            lesX[0]--;
            endroitBloque=false;
        }
        //si le chemin optimal est bloqué par les bord, le corp du serpent et la position du serpent par rapport à la cible 
        //on recherche le chemin optimal pour sortir le plus facilement avec les contraintes des bordures et du corps du serpent
        if (endroitBloque){
            if (plateau[lesX[0] + 1][lesY[0]] != BORDURE && !collision(lesX[0] + 1, lesY[0], lesX, lesY)) {
                lesX[0]++;
            } else if (plateau[lesX[0] - 1][lesY[0]] != BORDURE && !collision(lesX[0] - 1, lesY[0], lesX, lesY)) {
                lesX[0]--;
            } else if (plateau[lesX[0]][lesY[0] + 1] != BORDURE && !collision(lesX[0], lesY[0] + 1, lesX, lesY)) {
                lesY[0]++;
            } else if (plateau[lesX[0]][lesY[0] - 1] != BORDURE && !collision(lesX[0], lesY[0] - 1, lesX, lesY)) {
                lesY[0]--;
            }
        }
        // Vérifier si le serpent a atteint le portail
        if (lesX[0] == prochainX && lesY[0] == prochainY) {
            // Recalculer la cible après avoir traversé le portail 
            // les valeurs d'arrivé sont 2 et LARGEUR_PLATEAU-1 puisque le serpent ne doit pas sortir du plateau
            if (prochainY == HAUTEUR_PLATEAU+1) {
                lesY[0] = 1;
            } else if (prochainY == 0) {
                lesY[0] = HAUTEUR_PLATEAU;
            } else if (prochainX == LARGEUR_PLATEAU+1) {
                lesX[0] = 1;
            } else if (prochainX == 0) {
                lesX[0] = LARGEUR_PLATEAU;
            } 
            //On redéfini où doit passer le serpent
            calculerDistanceOptimale(lesX[0], lesY[0], cibleX, cibleY, &prochainX, &prochainY, &utilisePortail);
        }
    } else {
        // Déplacement optimal vers la cible en évitant les collisions avec le corps du serpent et les bordures
        if (lesY[0] < cibleY && plateau[lesX[0]][lesY[0] + 1] != BORDURE && !collision(lesX[0], lesY[0] + 1, lesX, lesY) && ((!(plateau[lesX[0]-1][lesY[0]+1]==PAVE && plateau[lesX[0]+1][lesY[0]+1]==PAVE)) || cibleX>lesX[0]-TAILLE_PAVE_X)) {
            lesY[0]++;
        } else if (lesY[0] > cibleY && plateau[lesX[0]][lesY[0] - 1] != BORDURE && !collision(lesX[0], lesY[0] - 1, lesX, lesY) && ((!(plateau[lesX[0]-1][lesY[0]-1]==PAVE && plateau[lesX[0]+1][lesY[0]-1]==PAVE)) || cibleX<lesX[0]+TAILLE_PAVE_X)) {
            lesY[0]--;
        } else if (lesX[0] < cibleX && plateau[lesX[0] + 1][lesY[0]] != BORDURE && !collision(lesX[0] + 1, lesY[0], lesX, lesY)) {
            lesX[0]++;
        } else if (lesX[0] > cibleX && plateau[lesX[0] - 1][lesY[0]] != BORDURE && !collision(lesX[0] - 1, lesY[0], lesX, lesY)) {
            lesX[0]--;
        } else {
            // Déplacement optimal vers la cible en évitant les collisions avec le corps, les bordures et les pavés
            if (plateau[lesX[0]][lesY[0] + 1] != BORDURE && cibleY>lesY[0] && !collision(lesX[0], lesY[0] + 1, lesX, lesY) && lesY[0] < TAILLE - 1) {
                lesY[0]++;
            } else if (plateau[lesX[0]][lesY[0] - 1] != BORDURE && cibleY<lesY[0] && !collision(lesX[0], lesY[0] - 1, lesX, lesY) && lesY[0] > 0) {
                lesY[0]--;
            } else if (plateau[lesX[0] + 1][lesY[0]] != BORDURE && cibleX<lesX[0] && !collision(lesX[0] + 1, lesY[0], lesX, lesY) && lesX[0] < TAILLE - 1) {
                lesX[0]++;
            } else if (plateau[lesX[0] - 1][lesY[0]] != BORDURE && cibleX>lesX[0] && !collision(lesX[0] - 1, lesY[0], lesX, lesY) && lesX[0] > 0) {
                lesX[0]--;
            } else {
                //Déplacement vers la cible en évitant les collisions le corps, les bordures et les pavés
                if (plateau[lesX[0]][lesY[0] + 1] != BORDURE && !collision(lesX[0], lesY[0] + 1, lesX, lesY) && lesY[0] < TAILLE-1) {
                    lesY[0]++;
                } else if (plateau[lesX[0]][lesY[0] - 1] != BORDURE && !collision(lesX[0], lesY[0] - 1, lesX, lesY) && lesY[0] > 0) {
                    lesY[0]--;
                } else if (plateau[lesX[0] + 1][lesY[0]] != BORDURE && !collision(lesX[0] + 1, lesY[0], lesX, lesY) && lesX[0] < TAILLE-1) {
                    lesX[0]++;
                } else if (plateau[lesX[0] - 1][lesY[0]] != BORDURE && !collision(lesX[0] - 1, lesY[0], lesX, lesY) && lesX[0] > 0) {
                    lesX[0]--;
                } else {
                    // Si aucune direction n'était possible, se déplace aléatoirement en évitant le corps et les bordures
                    if (plateau[lesX[0]][lesY[0] + 1] != BORDURE && !collision(lesX[0], lesY[0] + 1, lesX, lesY)) {
                        lesY[0]++;
                    } else if (plateau[lesX[0]][lesY[0] - 1] != BORDURE && !collision(lesX[0], lesY[0] - 1, lesX, lesY)) {
                        lesY[0]--;
                    } else if (plateau[lesX[0] + 1][lesY[0]] != BORDURE && !collision(lesX[0] + 1, lesY[0], lesX, lesY)) {
                        lesX[0]++;
                    } else if (plateau[lesX[0] - 1][lesY[0]] != BORDURE && !collision(lesX[0] - 1, lesY[0], lesX, lesY)) {
                        lesX[0]--;
                    }
                }
            }
        }
    }

    // Vérifie si la tête du serpent atteint la pomme
    *pomme = (lesX[0] == cibleX && lesY[0] == cibleY);

    // Redessine le serpent
    dessinerSerpent(lesX, lesY);
}



/**
 * @brief Affiche le plateau.
 *
 * Cette fonction affiche le plateau complet sur la console.
 *
 * @param plateau Le plateau de jeu.
 */
void dessinerPlateau(tPlateau plateau) {
    for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
        for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
            afficher(i, j, plateau[i][j]);
        }
    }
}

/**
 * @brief Affiche un caractare à  une position donnée.
 *
 * Cette fonction affiche un caractare à  la position spécifiée dans la console.
 *
 * @param x Position X de l'affichage.
 * @param y Position Y de l'affichage.
 * @param car Caractare à  afficher.
 */
void afficher(int x, int y, char car) {
    gotoxy(x, y);
    printf("%c", car);
    fflush(stdout);
}

/**
 * @brief Efface un caractare à  une position donnée.
 *
 * Cette fonction efface le caractare à  la position spécifiée.
 *
 * @param x Position X du caractare à  effacer.
 * @param y Position Y du caractare à  effacer.
 */
void effacer(int x, int y) { 
    afficher(x, y, VIDE); 
}

/**
 * @brief Dessine le serpent sur le plateau.
 *
 * Cette fonction dessine le serpent sur le plateau en affichant la tête
 * et les segments du corps.
 *
 * @param lesX Tableau des positions X des segments du serpent.
 * @param lesY Tableau des positions Y des segments du serpent.
 */
void dessinerSerpent(int lesX[], int lesY[]) {
    afficher(lesX[0], lesY[0], TETE);
    for (int i = 1; i < TAILLE; i++) {
        afficher(lesX[i], lesY[i], CORPS);
    }
}

/**
 * @brief Déplace le curseur à  une position spécifiée.
 *
 * Cette fonction déplace le curseur à  la position (x, y) spécifiée
 * dans la console.
 *
 * @param x Position X à  déplacer le curseur.
 * @param y Position Y à  déplacer le curseur.
 */
void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

/**
 * @brief Affiche la fin du programme et les résultats.
 *
 * Cette fonction affiche le nombre de déplacements effectués et le temps d'exécution
 * du programme.
 *
 * @param nbDeplacements Nombre total de déplacements effectués.
 * @param tempsDebut Temps au début du jeu.
 * @param tempsFin Temps à  la fin du jeu.
 */
void finProgramme(int nbDeplacements, clock_t tempsDebut, clock_t tempsFin) {
    double tempsCPU = (double)(tempsFin - tempsDebut) / CLOCKS_PER_SEC;
    printf("\nFin du programme\n");
    printf("Nombre de déplacements : %d\n", nbDeplacements);
    printf("Temps CPU: %.2f secondes\n", tempsCPU);
}

/**
 * @brief Vérifie si une touche a été appuyée.
 *
 * Cette fonction vérifie si une touche a été appuyée sur le clavier.
 *
 * @return 1 si une touche a été appuyée, 0 sinon.
 */
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