

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>  	// pour INT_MAX


#define MAX +1		// Joueur Maximisant
#define MIN -1  	// Joueur Minimisant

#define INFINI INT_MAX
#define MAXPARTIE 50	// Taille max du tableau Partie
			// qui sert � v�rifier si une conf a d�j� �t� g�n�r�e
			// pour ne pas le re-consid�rer une 2e fois.
			// On se limitera donc aux MAXPARTIE derniers coups


// Type d'une configuration
struct config {
	char mat[8][8];			// Echiquier
	int val;			// Estimation de la config
	char xrN, yrN, xrB, yrB;	// Positions des rois Noir et Blanc
	char roqueN, roqueB;		// Indicateurs de roque pour N et B :
					// 'g' grand roque non r�alisable
					// 'p' petit roque non r�alisable
					// 'n' non r�alisable des 2 cot�s
					// 'r' r�alisable (valeur initiale)
					// 'e' effectu�
};

/**************************/
/* Ent�te des fonctions : */
/**************************/

/*
 MinMax avec �lagage alpha-beta :
 Evalue la configuration 'conf' du joueur 'mode' en descendant de 'niv' niveaux.
 Le param�tre 'niv' est decr�ment� � chaque niveau (appel r�cursif).
 'alpha' et 'beta' repr�sentent les bornes initiales de l'intervalle d'int�r�t
 (pour pouvoir effectuer les coupes alpha et b�ta).
 'largeur' repr�sente le nombre max d'alternatives � explorer en profondeur � chaque niveau.
 Si 'largeur == +INFINI' toutes les alternatives seront prises en compte
 (c'est le comportement par d�faut).
 'numFctEst' est le num�ro de la fonction d'estimation � utiliser lorsqu'on arrive � la
 fronti�re d'exploration (c-a-d 'niv' atteint 0)
*/
int minmax_ab( struct config *conf, int mode, int niv, int min, int max, int largeur, int numFctEst );

/*
  La fonction d'estimation � utiliser, retourne une valeur dans ]-100, +100[
  quelques fonctions d'estimation disponibles (comme exemples).
  Le param�tre 'conf' repr�sente la configuration � estimer
*/
int estim1( struct config *conf );
int estim2( struct config *conf );
int estim3( struct config *conf );
int estim4( struct config *conf );
int estim5( struct config *conf );
int estim6( struct config *conf );
/* Votre propre fonction d'estimation */
int estim7( struct config *conf );

/*
  G�n�re les successeurs de la configuration 'conf' dans le tableau 'T',
  Retourne aussi dans 'n' le nb de configurations filles g�n�r�es.
*/
void generer_succ( struct config *conf, int mode, struct config T[], int *n );

/*
  G�n�re dans 'T' les configurations obtenues � partir de 'conf' lorsqu'un pion (a,b)
  va atteindre la limite de l'�chiquier (x,y)
*/
void transformPion( struct config *conf, int a, int b, int x, int y, struct config T[], int *n );

/*
  G�nere dans 'T' tous les coups possibles de la pi�ce (de couleur N ou B)
  se trouvant � la pos 'x','y'
*/
void deplacementsN(struct config *conf, int x, int y, struct config T[], int *n );
void deplacementsB(struct config *conf, int x, int y, struct config T[], int *n );

/*
  V�rifie si la case (x,y) est menac�e par une des pi�ces du joueur 'mode'
*/
int caseMenaceePar( int mode, int x, int y, struct config *conf );

/*
  Intialise la disposition des pieces dans la configuration initiale 'conf'
*/
void init( struct config *conf );

/*
  Affiche la configuration 'conf'
*/
void affich( struct config *conf, char *coup, int num );

/*
  Teste si 'conf' repr�sente une configuration d�j� jou�e (dans le tableau Partie)
  auquel cas elle retourne le num du coup + 1
*/
int dejaVisitee( struct config *conf );

/*
  Savegarde la config 'conf' dans le fichier f (global)
*/
void sauvConf( struct config *conf );

/*
  Copie la configuration 'c1' dans 'c2'
*/
void copier( struct config *c1, struct config *c2 );

/*
  Teste si les 2 grilles (�chiquiers) 'c1' et 'c2' sont �gales
*/
int egal(char c1[8][8], char c2[8][8] );

/*
  Teste s'il n'y a aucun coup possible � partir de la configuration 'conf'
*/
int AucunCoupPossible( struct config *conf );

/*
  Teste si 'conf' repr�sente une fin de partie
  et retourne dans 'cout' son score -100, 0 ou +100
*/
int feuille( struct config *conf, int *cout );

/*
  Comparaisons des configurations 'a' et 'b' pour le compte de qsort
  pour l'ordre croissant (confcmp123) et d�croissant (confcmp321)
*/
int confcmp123(const void *a, const void *b);
int confcmp321(const void *a, const void *b);

/*
  G�n�re dans 'coup' un texte d�crivant le dernier coup effectu� (pour l'affichage)
  en comparant deux configurations: l'ancienne 'oldconf' et la nouvelle 'newconf'
*/
void formuler_coup( struct config *oldconf, struct config *newconf, char *coup );





/************************/
/* Variables Globales : */
/************************/

// Tableau de config pour garder la trace des conf d�j� visit�es
struct config Partie[MAXPARTIE];

// Fichier pour sauvegarder l'historique des parties
FILE *f;

// compteur de coups effectu�s
int num_coup = 0;

// profondeur initiale d'exploration pr�liminaire avant le tri des alternatives
int h0 = 2;

// tableau des fonctions d'estimations
int (*Est[10])(struct config *);

// nb de fonctions d'estimation dans le tableau pr�c�dent
int nbEst;

// vecteurs pour g�n�rer les diff�rents d�placements par type de pi�ce ...
//    cavalier :
int dC[8][2] = { {-2,+1} , {-1,+2} , {+1,+2} , {+2,+1} , {+2,-1} , {+1,-2} , {-1,-2} , {-2,-1} };
//    fou (indices impairs), tour (indices pairs), reine et roi (indices pairs et impairs):
int D[8][2] = { {+1,0} , {+1,+1} , {0,+1} , {-1,+1} , {-1,0} , {-1,-1} , {0,-1} , {+1,-1} };

// pour statistques sur le nombre de coupes effectu�es
int nbAlpha = 0;
int nbBeta = 0;




/*******************************************/
/*********** Programme principal  **********/
/*******************************************/

int main( int argc, char *argv[] )
{

   int n, i, j, score, stop, cout, hauteur, largeur, tour, estMin, estMax;

   int sx, dx, cout2, legal;
   int cmin, cmax;
   int typeExec, refaire;

   char coup[20] = "";
   char nomf[20];  // nom du fichier de sauvegarde
   char ch[100];
   char sy, dy;

   struct config T[100], conf, conf1;

// initialiser le tableau des fonctions d'estimation
   Est[0] = estim1;
   Est[1] = estim2;
   Est[2] = estim3;
   Est[3] = estim4;
   Est[4] = estim5;
   Est[5] = estim6;
   Est[6] = estim7;
   // Nombre de fonctions d'estimation disponibles
   nbEst = 7;

   // Choix du type d'ex�cution (pc-contre-pc ou user-contre-pc) ...
   printf("Type de parties (B:Blancs  N:Noirs) :\n");
   printf("1- PC(B)   contre PC(N)\n");
   printf("2- USER(N) contre PC(B)\n");
   printf("3- USER(B) contre PC(N)\n");
   printf("\tChoix : ");
   scanf(" %d", &typeExec);
   if (typeExec != 2 && typeExec != 3) typeExec = 1;

   // Choix des fonctions d'estimation � utiliser ...
   do {
      printf("\nLes fonctions d'estimations disponibles sont:\n");
      printf("1- bas�e uniquement sur le nombre de pi�ces\n");
      printf("2- bas�e sur le nb de pieces, l'occupation, la d�fense du roi et les roques\n");
      printf("3- bas�e sur le nb de pi�ces et une perturbation al�atoire\n");
      printf("4- bas�e sur le nb de pieces et les menaces\n");
      printf("5- bas�e sur le nb de pieces et l'occupation\n");
      printf("6- bas�e sur une combinaisant de 3 estimations: (2 -> 5 -> 4)\n");
      printf("7- une fonction d'estimation al�atoire (� d�finir) \n\n");
      if (typeExec != 3) {
         printf("Donnez la fonction d'estimation utilis�e par le PC pour le joueur B : ");
         scanf(" %d", &estMax);
      }
      else
         estMax = 7;
      if (typeExec != 2) {
         printf("Donnez la fonction d'estimation utilis�e par le PC pour le joueur N : ");
         scanf(" %d", &estMin);
      }
      else
         estMin = 7;
   }
   while ( estMax < 1 || estMax > nbEst || estMin < 1 || estMin > nbEst );

   estMax--;
   estMin--;

   printf("\n--- Estimation_pour_Blancs = %d \t Estimation_pour_Noirs = %d ---\n", \
           (typeExec != 3 ? estMax+1 : 0), (typeExec != 2 ? estMin+1 : 0) );

   printf("\n'B' : joueur maximisant et 'N' : joueur minimisant\n");
   if (typeExec == 1)
      printf("\nPC 'B' contre PC 'N'\n\n");
   if (typeExec == 2)
      printf("\nUSER 'N' contre PC 'B'\n\n");
   if (typeExec == 3)
      printf("\nUSER 'B' contre PC 'N'\n\n");

   printf("Param�tres de MinMax\n");

   printf("Profondeur maximale d'exploration (par exemple 5) : ");
   scanf(" %d", &hauteur);

   printf("Nombre d'alternatives maximum � consid�rer dans chaque coup (0 pour +infini) : ");
   scanf(" %d", &largeur);
   if ( largeur == 0 ) largeur = +INFINI;

   // Initialise la configuration de d�part
   init( &conf );
   for (i=0; i<MAXPARTIE; i++)
      	copier( &conf, &Partie[i] );

   num_coup = 0;

   // initialise le g�n�rateur de nombre al�atoire pour la fonction estim3(...) si elle est utilis�e
   srand( time(NULL) );

   // sauter la fin de ligne des lectures pr�c�dentes
   while ((i = getchar()) != '\n' && i != EOF) { }

   printf("\nNom du fichier texte o� sera sauvegarder la partie : ");
   //scanf(" %s", nomf);
   fgets(ch, 20, stdin);
   sscanf(ch," %s", nomf);
   f = fopen(nomf, "w");

   fprintf(f, "--- Estimation_pour_Blancs = %d \t Estimation_pour_Noirs = %d ---\n", \
           (typeExec != 3 ? estMax+1 : 0), (typeExec != 2 ? estMin+1 : 0) );

   printf("\n---------------------------------------------\n\n");

   // Boucle principale du d�roulement d'une partie ...
   stop = 0;
   tour = MAX;	// le joueur MAX commence en premier
   while ( !stop ) {

      affich( &conf, coup, num_coup );
      copier( &conf, &Partie[num_coup % MAXPARTIE] );	// rajouter conf au tableau 'Partie'
      sauvConf( &conf );
      refaire = 0;	// indicateur de coup illegal, pour refaire le mouvement

      if ( tour == MAX ) {	// au tour de MAX ...

	 if (typeExec == 3) {   // c-a-d MAX ===> USER
	   printf("Au tour du joueur maximisant USER 'B'\n");
	   // r�cup�rer le coup du joueur ...
	   do {
	     printf("Donnez srcY srcX destY destX (par exemple d2d3) : ");
	     fgets(ch, 100, stdin);
	     i = sscanf(ch, " %c %d %c %d", &sy, &sx, &dy, &dx );
	     if (i != 4)
		printf("Lecture incorrecte, recommencer ...\n");
	   }
	   while ( i != 4 );

	   copier(&conf, &conf1);

	   // Traitement du coup du joueur ...

	   if (sx == conf.xrB+1 && sy-'a' == conf.yrB && dy == sy + 2) { // petit roque ...
	   //if (sy == '0') { // petit roque ...
	   	conf1.mat[0][4] = 0;
	   	conf1.mat[0][7] = 0;
	   	conf1.mat[0][6] = 'r'; conf1.xrB = 0; conf1.yrB = 6;
	   	conf1.mat[0][5] = 't';
	   	conf1.roqueB = 'e';
	   }
	   else
		if (sx == conf.xrB+1 && sy-'a' == conf.yrB && dy == sy - 2) { // grand roque ...
	   	//if (sy == '1') {  // grand roque ...
		   conf1.mat[0][4] = 0;
		   conf1.mat[0][0] = 0;
		   conf1.mat[0][2] = 'r'; conf1.xrB = 0; conf1.yrB = 2;
		   conf1.mat[0][3] = 't';
		   conf1.roqueB = 'e';
	    	}
	   	else  {  // deplacement normal (les autres coups) ...
		   conf1.mat[dx-1][dy-'a'] = conf1.mat[sx-1][sy-'a'];
		   conf1.mat[sx-1][sy-'a'] = 0;
		   // v�rifier possibilit� de transformation d'un pion arriv� en fin d'�chiquier ...
		   if (dx == 8 && conf1.mat[dx-1][dy-'a'] == 'p') {
		   	printf("Pion arriv� en ligne 8, transformer en (p/c/f/t/n) : ");
		   	scanf(" %s", ch);
		   	switch (ch[0]) {
			   case 'c' : conf1.mat[dx-1][dy-'a'] = 'c'; break;
			   case 'f' : conf1.mat[dx-1][dy-'a'] = 'f'; break;
			   case 't' : conf1.mat[dx-1][dy-'a'] = 't'; break;
			   case 'p' : conf1.mat[dx-1][dy-'a'] = 'p'; break;
			   default  : conf1.mat[dx-1][dy-'a'] = 'n';
		   	}
		   }
		}

	   // v�rifier si victoire (le roi N n'existe plus) ...
	   if ( conf1.xrN == dx-1 && conf1.yrN == dy-'a' ) {
		conf1.xrN = -1;
		conf1.yrN = -1;
	   }

	   // v�rification de la l�galit� du coup effectu� par le joueur ...
    	   generer_succ(  &conf, MAX, T, &n );

	   legal = 0;
	   for (i=0; i<n && !legal; i++)
	    	if ( egal(T[i].mat, conf1.mat) )  legal = 1;

	   if ( legal && !feuille(&conf1,&cout) ) {
	    	printf("OK\n\n");
	    	i--;
		formuler_coup( &conf, &T[i], coup );
	    	copier( &T[i], &conf );
 	   }
	   else
	    	if ( !legal && n > 0 ) {
	   	   printf("Coup ill�gal (%c%d%c%d) -- r�essayer\n", sy,sx,dy,dx);
		   refaire = 1; // pour forcer la prochaine it�ration � rester MAX
		}
	    	else
		   stop = 1;
         }

	 else { // MAX ===> PC

		printf("Au tour du joueur maximisant PC 'B' "); fflush(stdout);

	    	generer_succ(  &conf, MAX, T, &n );
		printf("\nhauteur = %d    nb alternatives = %d : ", hauteur, n); fflush(stdout);

		// Iterative Deepening ...
   		// On effectue un tri sur les alternatives selon l'estimation de leur qualit�
   		// Le but est d'explorer les alternatives les plus prometteuses d'abord
   		// pour maximiser les coupes lors des �valuation minmax avec alpha-b�ta

		// 1- on commence donc par une petite exploration de profondeur h0
		//    pour r�cup�rer des estimations plus pr�cises sur chaque coups:
	    	for (i=0; i<n; i++)
		     T[i].val = minmax_ab( &T[i], MIN, h0, -INFINI, +INFINI, largeur, estMax );

		// 2- on r�alise le tri des alternatives T suivant les estimations r�cup�r�es:
		qsort(T, n, sizeof(struct config), confcmp321);
		if ( largeur < n ) n = largeur;

		// 3- on lance l'exploration des alternatives tri�es avec la profondeur voulue:
	    	score = -INFINI;
	    	j = -1;

	    	for (i=0; i<n; i++) {
		   nbAlpha = nbBeta = 0;
		   cout = minmax_ab( &T[i], MIN, hauteur, score, +INFINI, largeur, estMax );
		   printf("."); fflush(stdout);
		   // printf(" %4d", cout); fflush(stdout);
		   if ( cout > score ) {  // Choisir le meilleur coup (c-a-d le plus grand score)
		   	score = cout;
		   	j = i;
		   }
	    	}
	    	if ( j != -1 ) { // jouer le coup et aller � la prochaine it�ration ...
		   printf("\n");
		   formuler_coup( &conf, &T[j], coup );
	    	   copier( &T[j], &conf );
		   conf.val = score;
	    	}
	    	else   // S'il n'y a pas de successeur possible, le joueur MAX � perdu
		   stop = 1;
         }

         if (stop) {
	    printf("\n *** le joueur maximisant 'B' a perdu ***\n");
	    fprintf(f, "Victoire de 'N'\n");
	 }

         //tour = MIN;

      }

      else {  // donc tour == MIN

	 if (typeExec == 2) {   // c-a-d MIN ===> USER
	   printf("Au tour du joueur minimisant USER 'N'\n");
	   // r�cup�rer le coup du joueur ...
	   do {
	     printf("Donnez srcY srcX destY destX (par exemple d7d5) : ");
	     fgets(ch, 100, stdin);
	     i = sscanf(ch, " %c %d %c %d", &sy, &sx, &dy, &dx );
	     if (i != 4)
		printf("Lecture incorrecte, recommencer ...\n");
	   }
	   while ( i != 4 );

	   copier(&conf, &conf1);

	   // Traitement du coup du joueur ...

	   if (sx == conf.xrN+1 && sy-'a' == conf.yrN && dy == sy + 2) { // petit roque ...
	   //if (sy == '0') { // petit roque ...
	   	conf1.mat[7][4] = 0;
	   	conf1.mat[7][7] = 0;
	   	conf1.mat[7][6] = -'r'; conf1.xrN = 7; conf1.yrN = 6;
	   	conf1.mat[7][5] = -'t';
	   	conf1.roqueN = 'e';
	   }
	   else
		if (sx == conf.xrN+1 && sy-'a' == conf.yrN && dy == sy - 2) { // grand roque ...
	   	//if (sy == '1') {  // grand roque ...
		   conf1.mat[7][4] = 0;
		   conf1.mat[7][0] = 0;
		   conf1.mat[7][2] = -'r'; conf1.xrN = 7; conf1.yrN = 2;
		   conf1.mat[7][3] = -'t';
		   conf1.roqueN = 'e';
	    	}
	   	else  {  // deplacement normal (les autres coups) ...
		   conf1.mat[dx-1][dy-'a'] = conf1.mat[sx-1][sy-'a'];
		   conf1.mat[sx-1][sy-'a'] = 0;
		   // v�rifier possibilit� de transformation d'un pion arriv� en fin d'�chiquier ...
		   if (dx == 1 && conf1.mat[dx-1][dy-'a'] == -'p') {
		   	printf("Pion arriv� en ligne 8, transformer en (p/c/f/t/n) : ");
		   	scanf(" %s", ch);
		   	switch (ch[0]) {
			   case 'c' : conf1.mat[dx-1][dy-'a'] = -'c'; break;
			   case 'f' : conf1.mat[dx-1][dy-'a'] = -'f'; break;
			   case 't' : conf1.mat[dx-1][dy-'a'] = -'t'; break;
			   case 'p' : conf1.mat[dx-1][dy-'a'] = -'p'; break;
			   default  : conf1.mat[dx-1][dy-'a'] = -'n';
		   	}
		   }
		}

	   // v�rifier si victoire (le roi B n'existe plus) ...
	   if ( conf1.xrB == dx-1 && conf1.yrB == dy-'a' ) {
		conf1.xrB = -1;
		conf1.yrB = -1;
	   }

	   // v�rification de la l�galit� du coup effectu� par le joueur ...
    	   generer_succ(  &conf, MIN, T, &n );

	   legal = 0;
	   for (i=0; i<n && !legal; i++)
	    	if ( egal(T[i].mat, conf1.mat) )  legal = 1;

	   if ( legal && !feuille(&conf1,&cout) ) {
	    	printf("OK\n\n");
	    	i--;
		formuler_coup( &conf, &T[i], coup );
	    	copier( &T[i], &conf );
 	   }
	   else
	    	if ( !legal && n > 0 ) {
	   	   printf("Coup ill�gal (%c%d%c%d) -- r�essayer\n", sy,sx,dy,dx);
		   refaire = 1; // pour forcer la prochaine it�ration � rester MIN
		}
	    	else
		   stop = 1;
         }

	 else { // MIN ===> PC

		printf("Au tour du joueur minimisant PC 'N' "); fflush(stdout);

		// G�n�rer tous les coups possibles pour le joueur N dans le tableau T
	    	generer_succ(  &conf, MIN, T, &n );
		printf("\nnb alternatives = %d : ", n); fflush(stdout);

		// Iterative Deepening ...
   		// On effectue un tri sur les alternatives selon l'estimation de leur qualit�
   		// Le but est d'explorer les alternatives les plus prometteuses d'abord
   		// pour maximiser les coupes lors des �valuation minmax avec alpha-b�ta

		// 1- on commence donc par une petite exploration de profondeur 3
		//    pour r�cup�rer des estimations plus pr�cises sur chaque coups:
	    	for (i=0; i<n; i++)
		     T[i].val = minmax_ab( &T[i], MAX, 3, -INFINI, +INFINI, largeur, estMin );

		// 2- on r�alise le tri des alternatives T suivant les estimations r�cup�r�es:
		qsort(T, n, sizeof(struct config), confcmp123);
		if ( largeur < n ) n = largeur;

		// 3- on lance l'exploration des alternatives tri�es avec la profondeur voulue:
	    	score = +INFINI;
	    	j = -1;
	    	for (i=0; i<n; i++) {
		   cout = minmax_ab( &T[i], MAX, hauteur, -INFINI, score, largeur, estMin );
		   printf("."); fflush(stdout);
		   if ( cout < score ) {  // Choisir le meilleur coup (c-a-d le plus petit score)
		   	score = cout;
		   	j = i;
		   }
	    	}
	    	if ( j != -1 ) { // jouer le coup et aller � la prochaine it�ration ...
		   // printf("\nchoix = %d (le coup num %d)\n", score, j+1);
		   printf("\n");
		   formuler_coup( &conf, &T[j], coup );
	    	   copier( &T[j], &conf );
		   conf.val = score;
	    	}
	    	else  // S'il n'y a pas de successeur possible, le joueur MIN � perdu
		   stop = 1;

	 }

         if (stop) {
	    printf("\n *** le joueur minimisant 'N' a perdu ***\n");
	    fprintf(f, "Victoire de 'B'\n");
	 }

         //tour = MAX;

      }

      if ( !refaire ) {
         num_coup++;
         tour = ( tour == MIN ? MAX : MIN );
      }

   } // while

   printf("\nFin de partie\n");

   return 0;

} // fin de main



/*****************************************/
/* Impl�mentation des autres fonctions : */
/*****************************************/


// *****************************
// Partie: Fonctions utilitaires
// *****************************


/* Sauvegarder conf dans le fichier f (pour l'historique) */
void sauvConf( struct config *conf )
{

   char buf[72] = "";
   int i, j;
   for (i=7; i>=0; i--) {
	for (j=0; j<8; j++)
	   if (conf->mat[i][j] == 0)  strcat(buf," ");
	   else if (conf->mat[i][j] < 0) {
		   strcat(buf, "-");
		   buf[strlen(buf)-1] = -conf->mat[i][j];
		}
		else {
		   strcat(buf, "+");
		   buf[strlen(buf)-1] = conf->mat[i][j] - 32;
		}
	strcat(buf, "\n");
   }
   fprintf(f, "--- coup N� %d ---\n%s\n",num_coup, buf);
   fflush(f);

} // fin sauvConf


/* Intialise la disposition des pieces dans la configuration initiale conf */
void init( struct config *conf )
{
   	int i, j;

    	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			conf->mat[i][j] = 0;	// Les cases vides sont initialis�es avec 0

	conf->mat[0][0]='t'; conf->mat[0][1]='c'; conf->mat[0][2]='f'; conf->mat[0][3]='n';
	conf->mat[0][4]='r'; conf->mat[0][5]='f'; conf->mat[0][6]='c'; conf->mat[0][7]='t';

	for (j=0; j<8; j++) {
		conf->mat[1][j] = 'p';
 		conf->mat[6][j] = -'p';
		conf->mat[7][j] = -conf->mat[0][j];
	}

	conf->xrB = 0; conf->yrB = 4;
	conf->xrN = 7; conf->yrN = 4;

	conf->roqueB = 'r';
	conf->roqueN = 'r';

	conf->val = 0;

} // fin de init


/* g�n�re un texte d�crivant le dernier coup effectu� (pour l'affichage) */
void formuler_coup( struct config *oldconf, struct config *newconf, char *coup )
{
   	int i,j;
	char piece[20];

	// verifier si roqueB effectu� ...
	if ( newconf->roqueB == 'e' && oldconf->roqueB != 'e' ) {
	   if ( newconf->yrB == 2 ) sprintf(coup, "g_roqueB" );
	   else  sprintf(coup, "p_roqueB" );
	   return;
	}

	// verifier si roqueN effectu� ...
	if ( newconf->roqueN == 'e' && oldconf->roqueN != 'e' ) {
	   if ( newconf->yrN == 2 ) sprintf(coup, "g_roqueN" );
	   else  sprintf(coup, "p_roqueN" );
	   return;
	}

	// Autres mouvements de pi�ces
	for(i=0; i<8; i++)
	   for (j=0; j<8; j++)
		if ( oldconf->mat[i][j] != newconf->mat[i][j] )
		   if ( newconf->mat[i][j] != 0 ) {
			switch (newconf->mat[i][j]) {
			   case -'p' : sprintf(piece,"pionN"); break;
			   case 'p' : sprintf(piece,"pionB"); break;
			   case -'c' : sprintf(piece,"cavalierN"); break;
			   case 'c' : sprintf(piece,"cavalierB"); break;
			   case -'f' : sprintf(piece,"fouN"); break;
			   case 'f' : sprintf(piece,"fouB"); break;
			   case -'t' : sprintf(piece,"tourN"); break;
			   case 't' : sprintf(piece,"tourB"); break;
			   case -'n' : sprintf(piece,"reineN"); break;
			   case 'n' : sprintf(piece,"reineB"); break;
			   case -'r' : sprintf(piece,"roiN"); break;
			   case 'r' : sprintf(piece,"roiB"); break;
			}
			sprintf(coup, "%s en %c%d", piece, 'a'+j, i+1);
			return;
		   }
} // fin de formuler_coup


/* Affiche la configuration conf */
void affich( struct config *conf, char *coup, int num )
{
	int i, j, k;
	int pB = 0, pN = 0, cB = 0, cN = 0, fB = 0, fN = 0, tB = 0, tN = 0, nB = 0, nN = 0;

     	printf("Coup num:%3d : %s\n", num, coup);
     	printf("\n");
	for (i=0;  i<8; i++)
		printf("\t   %c", i+'a');
   	printf("\n");

	for (i=0;  i<8; i++)
		printf("\t-------");
   	printf("\n");

	for(i=8; i>0; i--)  {
		printf("    %d", i);
		for (j=0; j<8; j++) {
			if ( conf->mat[i-1][j] < 0 ) printf("\t  %cN", -conf->mat[i-1][j]);
			else if ( conf->mat[i-1][j] > 0 ) printf("\t  %cB", conf->mat[i-1][j]);
				  else printf("\t   ");
			switch (conf->mat[i-1][j]) {
			    case -'p' : pN++; break;
			    case 'p'  : pB++; break;
			    case -'c' : cN++; break;
			    case 'c'  : cB++; break;
			    case -'f' : fN++; break;
			    case 'f'  : fB++; break;
			    case -'t' : tN++; break;
			    case 't'  : tB++; break;
			    case -'n' : nN++; break;
			    case 'n'  : nB++; break;
			}
		}
		printf("\n");

		for (k=0;  k<8; k++)
			printf("\t-------");
   		printf("\n");

	}
	printf("\n\tB : p(%d) c(%d) f(%d) t(%d) n(%d) \t N : p(%d) c(%d) f(%d) t(%d) n(%d)\n\n",
		pB, cB, fB, tB, nB, pN, cN, fN, tN, nN);
	printf("\n");

} // fin de  affich


/* Copie la configuration c1 dans c2  */
void copier( struct config *c1, struct config *c2 )
{
	int i, j;

	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			c2->mat[i][j] = c1->mat[i][j];

	c2->val = c1->val;
	c2->xrB = c1->xrB;
	c2->yrB = c1->yrB;
	c2->xrN = c1->xrN;
	c2->yrN = c1->yrN;

	c2->roqueB = c1->roqueB;
	c2->roqueN = c1->roqueN;
} // fin de copier


/* Teste si les �chiquiers c1 et c2 sont �gaux */
int egal(char c1[8][8], char c2[8][8] )
{
	int i, j;

	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			if (c1[i][j] != c2[i][j]) return 0;
	return 1;
} // fin de egal


/* Teste si conf a d�j� �t� jou�e (dans le tableau partie) */
int dejaVisitee( struct config *conf )
{
   int i = 0;
   int trouv = 0;
   while ( i < MAXPARTIE && trouv == 0 )
	if ( egal( conf->mat, Partie[i].mat ) ) trouv = i+1;
	else i++;
   return trouv;
}



// ***********************************
// Partie:  Evaluations et Estimations
// ***********************************


/* Teste s'il n'y a aucun coup possible dans la configuration conf */
int AucunCoupPossible( struct config *conf )
{
      	// ... A completer pour les matchs nuls
	// ... v�rifier que generer_succ retourne 0 configurations filles ...
	// ... ou qu'une m�me configuration a �t� g�n�r�e plusieurs fois ...
	return 0;

} // fin de AucunCoupPossible


/* Teste si conf repr�sente une fin de partie et retourne dans 'cout' la valeur associ�e */
int feuille( struct config *conf, int *cout )
{
	//int i, j, rbx, rnx, rby, rny;

	*cout = 0;

	// Si victoire pour les Noirs cout = -100
	if ( conf->xrB == -1 ) {
	   *cout = -100;
	   return 1;
	}

	// Si victoire pour les Blancs cout = +100
	if ( conf->xrN == -1 ) {
	   *cout = +100;
	   return 1;
	}

	// Si Match nul cout = 0
	if (  conf->xrB != -1 &&  conf->xrN != -1 && AucunCoupPossible( conf ) )
	   return 1;

	// Sinon ce n'est pas une config feuille
	return 0;

}  // fin de feuille


/* Quelques exemples de fonctions d'estimation simples (estim1, estim2, ...) */
/* Voir fonction estim plus bas pour le choix l'estimation � utiliser */

/* cette estimation est bas�e uniquement sur le nombre de pi�ces */
int estim1( struct config *conf )
{

	int i, j, ScrQte;
	int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;

	// parties : nombre de pi�ces
	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {
		switch (conf->mat[i][j]) {
		   case 'p' : pionB++;   break;
		   case 'c' :
		   case 'f' : cfB++;  break;
		   case 't' : tB++; break;
		   case 'n' : nB++;  break;

		   case -'p' : pionN++;  break;
		   case -'c' :
		   case -'f' : cfN++;  break;
		   case -'t' : tN++; break;
		   case -'n' : nN++;  break;
		}
	   }

	// Somme pond�r�e de pi�ces de chaque joueur.
	// Les poids sont fix�s comme suit: pion:2  cavalier/fou:6  tour:8  et  reine:20
	// Le facteur 100/76 pour ne pas sortir de l'intervalle ]-100 , +100[
	ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) ) * 100.0/76;

	if (ScrQte > 95) ScrQte = 95;		// pour r�tr�cir l'intervalle �
	if (ScrQte < -95) ScrQte = -95;		// ]-95 , +95[ car ce n'est qu'une estimation

	return ScrQte;

} // fin de estim1


// estimation bas�e sur le nb de pieces, l'occupation, la d�fense du roi et les roques
int estim2(  struct config *conf )
{
	int i, j, a, b, stop, bns, ScrQte, ScrDisp, ScrDfs, ScrDivers, Score;
	int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;
	int occCentreB = 0, occCentreN = 0, protectRB = 0, protectRN = 0, divB = 0, divN = 0;

	// parties : nombre de pi�ces et occupation du centre
	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {
		bns = 0;  // bonus pour l'occupation du centre de l'�chiquier
		if (i>1 && i<6 && j>=0 && j<=7 ) bns = 1;
		if (i>2 && i<5 && j>=2 && j<=5 ) bns = 2;
		switch (conf->mat[i][j]) {
		   case 'p' : pionB++; occCentreB += bns;  break;
		   case 'c' :
		   case 'f' : cfB++; occCentreB += 4*bns; break;
		   case 't' : tB++; break;
		   case 'n' : nB++; occCentreB += 4*bns; break;

		   case -'p' : pionN++; occCentreN += bns; break;
		   case -'c' :
		   case -'f' : cfN++; occCentreN += 4*bns; break;
		   case -'t' : tN++; break;
		   case -'n' : nN++; occCentreN += 4*bns; break;
		}
	   }

	ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) );
	// donc ScrQteMax ==> 76

	ScrDisp = occCentreB - occCentreN;
	// donc ScrDispMax ==> 42

	// partie : d�fense du roi B ...
	for (i=0; i<8; i += 1) {
	   // traitement des 8 directions paires et impaires
	   stop = 0;
	   a = conf->xrB + D[i][0];
	   b = conf->yrB + D[i][1];
	   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 )
		if ( conf->mat[a][b] != 0 )  stop = 1;
		else {
		    a = a + D[i][0];
		    b = b + D[i][1];
		}
	   if ( stop )
		if ( conf->mat[a][b] > 0 ) protectRB++;
	} // for

	// partie : d�fense du roi N ...
	for (i=0; i<8; i += 1) {
	   // traitement des 8 directions paires et impaires
	   stop = 0;
	   a = conf->xrN + D[i][0];
	   b = conf->yrN + D[i][1];
	   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 )
		if ( conf->mat[a][b] != 0 )  stop = 1;
		else {
		    a = a + D[i][0];
		    b = b + D[i][1];
		}
	   if ( stop )
		if ( conf->mat[a][b] < 0 ) protectRN++;
	} // for

	ScrDfs = protectRB - protectRN;
	// donc ScrDfsMax ==> 8

	// Partie : autres consid�rations ...
	if ( conf->roqueB == 'e' ) divB = 24;	// favoriser les roques B
	if ( conf->roqueB == 'r' ) divB = 12;
	if ( conf->roqueB == 'p' || conf->roqueB == 'g' ) divB = 10;

	if ( conf->roqueN == 'e' ) divN = 24;	// favoriser les roques N
	if ( conf->roqueN == 'r' ) divN = 12;
	if ( conf->roqueN == 'p' || conf->roqueN == 'g' ) divN = 10;

	ScrDivers = divB - divN;
	// donc ScrDiversMax ==> 24

	Score = (4*ScrQte + ScrDisp + ScrDfs + ScrDivers) * 100.0/(4*76+42+8+24);
	// pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

        if (Score > 98 ) Score = 98;
        if (Score < -98 ) Score = -98;

	return Score;

} // fin de estim2

int estim2ColonneOuverte(  struct config *conf ) // fonction pareille a estim2 en prenant en consid�ration les colonnes ouvertes
{
	int i, j, a, b, stop, bns, ScrQte, ScrDisp, ScrDfs, ScrDivers, ScrColonneO, Score; // rajouter ScrColonneO
	int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;
	int occCentreB = 0, occCentreN = 0, protectRB = 0, protectRN = 0, divB = 0, divN = 0;
	int colonneOB=0,colonneON=0;

	// parties : nombre de pi�ces et occupation du centre
	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {
		bns = 0;  // bonus pour l'occupation du centre de l'�chiquier
		if (i>1 && i<6 && j>=0 && j<=7 ) bns = 1;
		if (i>2 && i<5 && j>=2 && j<=5 ) bns = 2;
		switch (conf->mat[i][j]) {
		   case 'p' : pionB++; occCentreB += bns;  break;
		   case 'c' :
		   case 'f' : cfB++; occCentreB += 4*bns; break;
		   case 't' : tB++; colonneOB=colonneOB+colonneOuverteB(conf->mat,i,j);  break;
		   case 'n' : nB++; occCentreB += 4*bns; break;

		   case -'p' : pionN++; occCentreN += bns; break;
		   case -'c' :
		   case -'f' : cfN++; occCentreN += 4*bns; break;
		   case -'t' : tN++; colonneON=colonneON+colonneOuverteN(conf->mat,i,j);  break;
		   case -'n' : nN++; occCentreN += 4*bns; break;
		}
	   }

	ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) );
	// donc ScrQteMax ==> 76

	ScrDisp = occCentreB - occCentreN;

	// donc ScrDispMax ==> 42
	ScrColonneO=colonneOB-colonneON;
	// donc ScrColonneOMax ==> 4
	// partie : d�fense du roi B ...
	for (i=0; i<8; i += 1) {
	   // traitement des 8 directions paires et impaires
	   stop = 0;
	   a = conf->xrB + D[i][0];
	   b = conf->yrB + D[i][1];
	   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 )
		if ( conf->mat[a][b] != 0 )  stop = 1;
		else {
		    a = a + D[i][0];
		    b = b + D[i][1];
		}
	   if ( stop )
		if ( conf->mat[a][b] > 0 ) protectRB++;
	} // for

	// partie : d�fense du roi N ...
	for (i=0; i<8; i += 1) {
	   // traitement des 8 directions paires et impaires
	   stop = 0;
	   a = conf->xrN + D[i][0];
	   b = conf->yrN + D[i][1];
	   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 )
		if ( conf->mat[a][b] != 0 )  stop = 1;
		else {
		    a = a + D[i][0];
		    b = b + D[i][1];
		}
	   if ( stop )
		if ( conf->mat[a][b] < 0 ) protectRN++;
	} // for

	ScrDfs = protectRB - protectRN;
	// donc ScrDfsMax ==> 8

	// Partie : autres consid�rations ...
	if ( conf->roqueB == 'e' ) divB = 24;	// favoriser les roques B
	if ( conf->roqueB == 'r' ) divB = 12;
	if ( conf->roqueB == 'p' || conf->roqueB == 'g' ) divB = 10;

	if ( conf->roqueN == 'e' ) divN = 24;	// favoriser les roques N
	if ( conf->roqueN == 'r' ) divN = 12;
	if ( conf->roqueN == 'p' || conf->roqueN == 'g' ) divN = 10;

	ScrDivers = divB - divN;
	// donc ScrDiversMax ==> 24
	   // rajouter au score final le score obtenu dans les colonnes ouvertes * 2

	Score = (4*ScrQte + ScrDisp + ScrDfs + ScrDivers+ 2*ScrColonneO) * 100.0/(4*76+42+8+24+2*4);
	// pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

        if (Score > 98 ) Score = 98;
        if (Score < -98 ) Score = -98;

	return Score;

} // fin de estim2ColonneOuverte

/* prise en compte du nombre de pi�ces avec petite perturbation al�atoire */
int estim3( struct config *conf )
{

	int i, j, ScrQte, Score;
	int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;

	// parties : nombre de pi�ces
	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {
		switch (conf->mat[i][j]) {
		   case 'p' : pionB++; break;
		   case 'c' :
		   case 'f' : cfB++; break;
		   case 't' : tB++; break;
		   case 'n' : nB++; break;

		   case -'p' : pionN++; break;
		   case -'c' :
		   case -'f' : cfN++; break;
		   case -'t' : tN++; break;
		   case -'n' : nN++; break;
		}
	   }

	ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) );
	// donc ScrQteMax ==> 76

	Score = (10*ScrQte + rand()%10) * 100.0 / (10*76+10);
	// pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

        if (Score > 98 ) Score = 98;
        if (Score < -98 ) Score = -98;

	return Score;

} // fin de estim3


// estimation bas�e sur le nb de pieces et les menaces
int estim4( struct config *conf )
{

	int i, j, Score;
	int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;
	int npmB = 0, npmN = 0;

	// parties : nombre de pi�ces
	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {
		switch (conf->mat[i][j]) {
		   case 'p' : pionB++;   break;
		   case 'c' :
		   case 'f' : cfB++;  break;
		   case 't' : tB++; break;
		   case 'n' : nB++;  break;

		   case -'p' : pionN++;  break;
		   case -'c' :
		   case -'f' : cfN++;  break;
		   case -'t' : tN++; break;
		   case -'n' : nN++;  break;
		}
	   }

	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {
		if ( conf->mat[i][j] < 0 && caseMenaceePar(MAX, i, j, conf) ) {
		   npmB++;
		   if ( conf->mat[i][j] == -'c' || conf->mat[i][j] == -'f' )
		   	npmB++;
		   if ( conf->mat[i][j] == -'t' || conf->mat[i][j] == -'n' )
		   	npmB += 2;
		   if ( conf->mat[i][j] == -'r' )
		   	npmB += 5;
		}
		if ( conf->mat[i][j] > 0 && caseMenaceePar(MIN, i, j, conf) ) {
		   npmN++;
		   if ( conf->mat[i][j] == 'c' || conf->mat[i][j] == 'f' )
		   	npmN++;
		   if ( conf->mat[i][j] == 't' || conf->mat[i][j] == 'n' )
		   	npmN += 2;
		   if ( conf->mat[i][j] == 'r' )
		   	npmN += 5;
		}
	   }

	Score = ( 4*((pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20)) + \
		  (npmB - npmN) ) * 100.0/(4*76+31);

	// pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

	if (Score > 95) Score = 95;		// pour r�tr�cir l'intervalle �
	if (Score < -95) Score = -95;		// ]-95 , +95[ car ce n'est qu'une estimation

	return Score;

} // fin de estim4


// estimation bas�e sur le nb de pieces et l'occupation
int estim5(  struct config *conf )
{
	int i, j, a, b, stop, bns, ScrQte, ScrDisp, ScrDfs, ScrDivers, Score;
	int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;
	int occCentreB = 0, occCentreN = 0, protectRB = 0, protectRN = 0, divB = 0, divN = 0;

	// parties : nombre de pi�ces et occupation du centre
	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {
		bns = 0;  // bonus pour l'occupation du centre de l'�chiquier
		if (i>1 && i<6 && j>=0 && j<=7 ) bns = 1;
		if (i>2 && i<5 && j>=2 && j<=5 ) bns = 2;
		switch (conf->mat[i][j]) {
		   case 'p' : pionB++; occCentreB += bns;  break;
		   case 'c' :
		   case 'f' : cfB++; occCentreB += 4*bns; break;
		   case 't' : tB++; break;
		   case 'n' : nB++; occCentreB += 4*bns; break;

		   case -'p' : pionN++; occCentreN += bns; break;
		   case -'c' :
		   case -'f' : cfN++; occCentreN += 4*bns; break;
		   case -'t' : tN++; break;
		   case -'n' : nN++; occCentreN += 4*bns; break;
		}
	   }

	ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) );

	ScrDisp = occCentreB - occCentreN;

	Score = (4*ScrQte + ScrDisp) * 100.0/(4*76+42);
	// pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

        if (Score > 98 ) Score = 98;
        if (Score < -98 ) Score = -98;

	return Score;

} // fin de estim5

// estimation bas�e sur le nb de pieces et l'occupation en augmentant le coefficient de l'occupation du centre
int estim55(  struct config *conf )
{
	int i, j, a, b, stop, bns, ScrQte, ScrDisp, ScrDfs, ScrDivers, Score;
	int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;
	int occCentreB = 0, occCentreN = 0, protectRB = 0, protectRN = 0, divB = 0, divN = 0;

	// parties : nombre de pi�ces et occupation du centre
	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {
		bns = 0;  // bonus pour l'occupation du centre de l'�chiquier
		if (i>1 && i<6 && j>=0 && j<=7 ) bns = 1;
		if (i>2 && i<5 && j>=2 && j<=5 ) bns = 2;
		switch (conf->mat[i][j]) {
		   case 'p' : pionB++; occCentreB += bns;  break;
		   case 'c' :
		   case 'f' : cfB++; occCentreB += 4*bns; break;
		   case 't' : tB++; break;
		   case 'n' : nB++; occCentreB += 4*bns; break;

		   case -'p' : pionN++; occCentreN += bns; break;
		   case -'c' :
		   case -'f' : cfN++; occCentreN += 4*bns; break;
		   case -'t' : tN++; break;
		   case -'n' : nN++; occCentreN += 4*bns; break;
		}
	   }

	ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) );

	ScrDisp = occCentreB - occCentreN;
  // nous avons multipli� ScrDisp par 2 pour lui donner un poid parraport � estim5
	Score = (4*ScrQte + 2*ScrDisp) * 100.0/(4*76+2*42);
	// pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

        if (Score > 98 ) Score = 98;
        if (Score < -98 ) Score = -98;

	return Score;

}

/* on peut aussi combiner entre plusieurs estimation comme cet exemple: */
int estim6( struct config *conf )
{
	// Combinaison de 3 fonctions d'estimation:
	// -au d�but on utilise estim2 pour favoriser l'occupation du centre et la defense du roi
	// -au milieu on utilise estim5 pour continuer � bien se positionner
	// -dans la derni�re phase on utilise estim4 pour favoriser l'attaque
	if ( num_coup < 25 )			// 1ere phase
	   return estim2(conf);
	if ( num_coup >= 25 && num_coup < 35 )	// 2e phase
	   return estim5(conf);
	if ( num_coup >= 35 )			// 3e phase
	   return estim4(conf);

} // fin de estim6


/* Une nouvelle fonction d'estimation  */
int estim7( struct config *conf )
{

	if ( num_coup < 15 )			// 1ere phase (d�but)
	   return estim55(conf);
	if ( num_coup >= 15 && num_coup < 25 )	// fin de premi�re phase
	   return estim2(conf);
	   if ( num_coup >= 25 && num_coup < 35 )	// 2e phase (milieu)
	   return estim2ColonneOuverte(conf);
	if ( num_coup >= 35 )			// 3e phase (fin)
     return estim4(conf);

} // fin de estim7
//********************************************
// V�rifier les colonnes ouvertes et semi Ouvertes
//************************************************
int colonneOuverteB(char mat[8][8], int x, int y) // colonne ouverte pour les blancs
{
    int i;
    int score=2;

    for (i=0;i<8;i++){
        if (i!=x) {
        if ((mat[i][y]!=0 ) && (mat[i][y]!= 't')){
            score =2;
        if (mat[i][y] >0 ) score =0; break;
        }

        }
    }

    return score;
}
int colonneOuverteN(char mat[8][8], int x, int y) // colonne ouverte pour les noirs
{
    int i;
    int score=2;

    for (i=0;i<8;i++){
        if (i!=x) {
        if ((mat[i][y]!=0 ) && (mat[i][y]!= -'t')){ // si la colonne est semi-ouverte
            score =2;
        if (mat[i][y] <0 ) score =0; break; // la colonne n'est pas ouverte
        }

        }
    }

    return score;
}

// ***********************************
// Partie:  G�n�ration des Successeurs
// ***********************************

/* G�n�re dans T les configurations obtenues � partir de conf lorsqu'un pion (a,b) va atteindre
   la limite de l'�chiquier: pos (x,y)  */
void transformPion( struct config *conf, int a, int b, int x, int y, struct config T[], int *n )
{
	int signe = +1;
	if (conf->mat[a][b] < 0 ) signe = -1;
	copier(conf, &T[*n]);
	T[*n].mat[a][b] = 0;
	T[*n].mat[x][y] = signe * 'n';	// transformation en Reine
	(*n)++;
	copier(conf, &T[*n]);
	T[*n].mat[a][b] = 0;
	T[*n].mat[x][y] = signe * 'c';	// transformation en Cavalier
	(*n)++;
	copier(conf, &T[*n]);
	T[*n].mat[a][b] = 0;
	T[*n].mat[x][y] = signe * 'f';	// transformation en Fou
	(*n)++;
	copier(conf, &T[*n]);
	T[*n].mat[a][b] = 0;
	T[*n].mat[x][y] = signe * 't';	// transformation en Tour
	(*n)++;

} // fin de transformPion


// V�rifie si la case (x,y) est menac�e par une des pi�ces du joueur 'mode'
int caseMenaceePar( int mode, int x, int y, struct config *conf )
{
	int i, j, a, b, stop;

	// menace par le roi ...
	for (i=0; i<8; i += 1) {
	   // traitement des 8 directions paires et impaires
	   a = x + D[i][0];
	   b = y + D[i][1];
	   if ( a >= 0 && a <= 7 && b >= 0 && b <= 7 )
		if ( conf->mat[a][b]*mode == 'r' ) return 1;
	} // for

	// menace par cavalier ...
	for (i=0; i<8; i++)
	   if ( x+dC[i][0] <= 7 && x+dC[i][0] >= 0 && y+dC[i][1] <= 7 && y+dC[i][1] >= 0 )
		if ( conf->mat[ x+dC[i][0] ] [ y+dC[i][1] ] * mode == 'c' )
		   return 1;

	// menace par pion ...
	if ( (x-mode) >= 0 && (x-mode) <= 7 && y > 0 && conf->mat[x-mode][y-1]*mode == 'p' )
	   return 1;
	if ( (x-mode) >= 0 && (x-mode) <= 7 && y < 7 && conf->mat[x-mode][y+1]*mode == 'p' )
	   return 1;

	// menace par fou, tour ou reine ...
	for (i=0; i<8; i += 1) {
	   // traitement des 8 directions paires et impaires
	   stop = 0;
	   a = x + D[i][0];
	   b = y + D[i][1];
	   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 )
		if ( conf->mat[a][b] != 0 )  stop = 1;
		else {
		    a = a + D[i][0];
		    b = b + D[i][1];
		}
	   if ( stop )  {
		if ( conf->mat[a][b]*mode == 'f' && i % 2 != 0 ) return 1;
		if ( conf->mat[a][b]*mode == 't' && i % 2 == 0 ) return 1;
		if ( conf->mat[a][b]*mode == 'n' ) return 1;
	   }
	} // for

	// sinon, aucune menace ...
	return 0;

} // fin de caseMenaceePar


/* G�nere dans T tous les coups possibles de la pi�ce (de couleur N) se trouvant � la pos x,y */
void deplacementsN(struct config *conf, int x, int y, struct config T[], int *n )
{
	int i, j, a, b, stop;

	switch(conf->mat[x][y]) {
	// mvmt PION ...
	case -'p' :
		if ( x > 0 && conf->mat[x-1][y] == 0 ) {
			// avance d'une case
			copier(conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x-1][y] = -'p';
			(*n)++;
			if ( x == 1 ) transformPion( conf, x, y, x-1, y, T, n );
		}
		if ( x == 6 && conf->mat[5][y] == 0 && conf->mat[4][y] == 0) {
			// avance de 2 cases
			copier(conf, &T[*n]);
			T[*n].mat[6][y] = 0;
			T[*n].mat[4][y] = -'p';
			(*n)++;
		}
		if ( x > 0 && y >0 && conf->mat[x-1][y-1] > 0 ) {
			// attaque � droite (en descendant)
			copier(conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x-1][y-1] = -'p';
			// cas o� le roi adverse est pris...
			if (T[*n].xrB == x-1 && T[*n].yrB == y-1) {
				T[*n].xrB = -1; T[*n].yrB = -1;
			}

			(*n)++;
			if ( x == 1 ) transformPion( conf, x, y, x-1, y-1, T, n );
		}
		if ( x > 0 && y < 7 && conf->mat[x-1][y+1] > 0 ) {
			// attaque � gauche (en descendant)
			copier(conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x-1][y+1] = -'p';
			// cas o� le roi adverse est pris...
			if (T[*n].xrB == x-1 && T[*n].yrB == y+1) {
				T[*n].xrB = -1; T[*n].yrB = -1;
			}

			(*n)++;
			if ( x == 1 ) transformPion( conf, x, y, x-1, y+1, T, n );
		}
		break;

	// mvmt CAVALIER ...
	case -'c' :
		for (i=0; i<8; i++)
		   if ( x+dC[i][0] <= 7 && x+dC[i][0] >= 0 && y+dC[i][1] <= 7 && y+dC[i][1] >= 0 )
			if ( conf->mat[ x+dC[i][0] ] [ y+dC[i][1] ] >= 0 )  {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   T[*n].mat[ x+dC[i][0] ][ y+dC[i][1] ] = -'c';
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrB == x+dC[i][0] && T[*n].yrB == y+dC[i][1]) {
				T[*n].xrB = -1; T[*n].yrB = -1;
			   }

			   (*n)++;
			}
		break;

	// mvmt FOU ...
	case -'f' :
		for (i=1; i<8; i += 2) {
		   // traitement des directions impaires (1, 3, 5 et 7)
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf->mat[ a ] [ b ] < 0 )  stop = 1;
			else {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] > 0 ) stop = 1;
			   T[*n].mat[a][b] = -'f';
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt TOUR ...
	case -'t' :
		for (i=0; i<8; i += 2) {
		   // traitement des directions paires (0, 2, 4 et 6)
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf->mat[ a ] [ b ] < 0 )  stop = 1;
			else {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] > 0 ) stop = 1;
			   T[*n].mat[a][b] = -'t';
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

			   if ( conf->roqueN != 'e' && conf->roqueN != 'n' ) {
			      if ( x == 7 && y == 0 && conf->roqueN != 'p')
				 // le grand roque ne sera plus possible
			   	 T[*n].roqueN = 'g';
			      else if ( x == 7 && y == 0 )
				      // ni le grand roque ni le petit roque ne seront possibles
			   	      T[*n].roqueN = 'n';

			      if ( x == 7 && y == 7 && conf->roqueN != 'g' )
				 // le petit roque ne sera plus possible
			   	 T[*n].roqueN = 'p';
			      else if ( x == 7 && y == 7 )
				      // ni le grand roque ni le petit roque ne seront possibles
			   	      T[*n].roqueN = 'n';
			   }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt REINE ...
	case -'n' :
		for (i=0; i<8; i += 1) {
		   // traitement des 8 directions paires et impaires
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf->mat[ a ] [ b ] < 0 )  stop = 1;
			else {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] > 0 ) stop = 1;
			   T[*n].mat[a][b] = -'n';
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt ROI ...
	case -'r' :
		// v�rifier possibilit� de faire un roque ...
		if ( conf->roqueN != 'n' && conf->roqueN != 'e' ) {
		   if ( conf->roqueN != 'g' && conf->mat[7][1] == 0 \
			&& conf->mat[7][2] == 0 && conf->mat[7][3] == 0 )
		      if ( !caseMenaceePar( MAX, 7, 1, conf ) && !caseMenaceePar( MAX, 7, 2, conf ) && \
			   !caseMenaceePar( MAX, 7, 3, conf ) && !caseMenaceePar( MAX, 7, 4, conf ) )  {
			// Faire un grand roque ...
			copier(conf, &T[*n]);
			T[*n].mat[7][4] = 0;
			T[*n].mat[7][0] = 0;
			T[*n].mat[7][2] = -'r'; T[*n].xrN = 7; T[*n].yrN = 2;
			T[*n].mat[7][3] = -'t';
			// aucun roque ne sera plus possible � partir de cette config
			T[*n].roqueN = 'e';
			(*n)++;
		      }
		   if ( conf->roqueN != 'p' && conf->mat[7][5] == 0 && conf->mat[7][6] == 0 )
		      if ( !caseMenaceePar( MAX, 7, 4, conf ) && !caseMenaceePar( MAX, 7, 5, conf ) && \
			   !caseMenaceePar( MAX, 7, 6, conf ) )  {
			// Faire un petit roque ...
			copier(conf, &T[*n]);
			T[*n].mat[7][4] = 0;
			T[*n].mat[7][7] = 0;
			T[*n].mat[7][6] = -'r'; T[*n].xrN = 7; T[*n].yrN = 6;
			T[*n].mat[7][5] = -'t';
			// aucun roque ne sera plus possible � partir de cette config
			T[*n].roqueN = 'e';
			(*n)++;
		      }
		}

		// v�rifier les autres mouvements du roi ...
		for (i=0; i<8; i += 1) {
		   // traitement des 8 directions paires et impaires
		   a = x + D[i][0];
		   b = y + D[i][1];
		   if ( a >= 0 && a <= 7 && b >= 0 && b <= 7 )
			if ( conf->mat[a][b] >= 0 ) {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   T[*n].mat[a][b] = -'r'; T[*n].xrN = a; T[*n].yrN = b;
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrB == a && T[*n].yrB == b) {
			      T[*n].xrB = -1;
			      T[*n].yrB = -1;
			   }
			   // aucun roque ne sera plus possible � partir de cette config
			   T[*n].roqueN = 'n';
			   (*n)++;
			}
		} // for
		break;

	}

} // fin de deplacementsN


/* G�nere dans T tous les coups possibles de la pi�ce (de couleur B) se trouvant � la pos x,y */
void deplacementsB(struct config *conf, int x, int y, struct config T[], int *n )
{
	int i, j, a, b, stop;

	switch(conf->mat[x][y]) {
	// mvmt PION ...
	case 'p' :
		if ( x <7 && conf->mat[x+1][y] == 0 ) {
			// avance d'une case
			copier(conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x+1][y] = 'p';
			(*n)++;
			if ( x == 6 ) transformPion( conf, x, y, x+1, y, T, n );
		}
		if ( x == 1 && conf->mat[2][y] == 0 && conf->mat[3][y] == 0) {
			// avance de 2 cases
			copier(conf, &T[*n]);
			T[*n].mat[1][y] = 0;
			T[*n].mat[3][y] = 'p';
			(*n)++;
		}
		if ( x < 7 && y > 0 && conf->mat[x+1][y-1] < 0 ) {
			// attaque � gauche (en montant)
			copier(conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x+1][y-1] = 'p';
			// cas o� le roi adverse est pris...
			if (T[*n].xrN == x+1 && T[*n].yrN == y-1) {
				T[*n].xrN = -1; T[*n].yrN = -1;
			}

			(*n)++;
			if ( x == 6 ) transformPion( conf, x, y, x+1, y-1, T, n );
		}
		if ( x < 7 && y < 7 && conf->mat[x+1][y+1] < 0 ) {
			// attaque � droite (en montant)
			copier(conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x+1][y+1] = 'p';
			// cas o� le roi adverse est pris...
			if (T[*n].xrN == x+1 && T[*n].yrN == y+1) {
				T[*n].xrN = -1; T[*n].yrN = -1;
			}

			(*n)++;
			if ( x == 6 ) transformPion( conf, x, y, x+1, y+1, T, n );
		}
		break;

	// mvmt CAVALIER ...
	case 'c' :
		for (i=0; i<8; i++)
		   if ( x+dC[i][0] <= 7 && x+dC[i][0] >= 0 && y+dC[i][1] <= 7 && y+dC[i][1] >= 0 )
			if ( conf->mat[ x+dC[i][0] ] [ y+dC[i][1] ] <= 0 )  {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   T[*n].mat[ x+dC[i][0] ][ y+dC[i][1] ] = 'c';
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrN == x+dC[i][0] && T[*n].yrN == y+dC[i][1]) {
				T[*n].xrN = -1;
				T[*n].yrN = -1;
			   }

			   (*n)++;
			}
		break;

	// mvmt FOU ...
	case 'f' :
		for (i=1; i<8; i += 2) {
		   // traitement des directions impaires (1, 3, 5 et 7)
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf->mat[ a ] [ b ] > 0 )  stop = 1;
			else {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] < 0 ) stop = 1;
			   T[*n].mat[a][b] = 'f';
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrN == a && T[*n].yrN == b) {
				T[*n].xrN = -1;
				T[*n].yrN = -1;
			   }
			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt TOUR ...
	case 't' :
		for (i=0; i<8; i += 2) {
		   // traitement des directions paires (0, 2, 4 et 6)
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf->mat[ a ] [ b ] > 0 )  stop = 1;
			else {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] < 0 ) stop = 1;
			   T[*n].mat[a][b] = 't';
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrN == a && T[*n].yrN == b) {
				T[*n].xrN = -1;
				T[*n].yrN = -1;
			   }
			   if ( conf->roqueB != 'e' && conf->roqueB != 'n' ) {
			     if ( x == 0 && y == 0 && conf->roqueB != 'p')
				// le grand roque ne sera plus possible
			   	T[*n].roqueB = 'g';
			     else if ( x == 0 && y == 0 )
				     // ni le grand roque ni le petit roque ne seront possibles
			   	     T[*n].roqueB = 'n';
			     if ( x == 0 && y == 7 && conf->roqueB != 'g' )
				// le petit roque ne sera plus possible
			   	T[*n].roqueB = 'p';
			     else if ( x == 0 && y == 7 )
				     // ni le grand roque ni le petit roque ne seront possibles
			   	     T[*n].roqueB = 'n';
			   }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt REINE ...
	case 'n' :
		for (i=0; i<8; i += 1) {
		   // traitement des 8 directions paires et impaires
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf->mat[ a ] [ b ] > 0 )  stop = 1;
			else {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] < 0 ) stop = 1;
			   T[*n].mat[a][b] = 'n';
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrN == a && T[*n].yrN == b) {
			      T[*n].xrN = -1;
			      T[*n].yrN = -1;
			   }
			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt ROI ...
	case 'r' :
		// v�rifier possibilit� de faire un roque ...
		if ( conf->roqueB != 'n' && conf->roqueB != 'e' ) {
		   if ( conf->roqueB != 'g' && conf->mat[0][1] == 0 && \
			conf->mat[0][2] == 0 && conf->mat[0][3] == 0 )
		      if ( !caseMenaceePar( MIN, 0, 1, conf ) && !caseMenaceePar( MIN, 0, 2, conf ) && \
			   !caseMenaceePar( MIN, 0, 3, conf ) && !caseMenaceePar( MIN, 0, 4, conf ) )  {
			// Faire un grand roque ...
			copier(conf, &T[*n]);
			T[*n].mat[0][4] = 0;
			T[*n].mat[0][0] = 0;
			T[*n].mat[0][2] = 'r'; T[*n].xrB = 0; T[*n].yrB = 2;
			T[*n].mat[0][3] = 't';
			// aucun roque ne sera plus possible � partir de cette config
			T[*n].roqueB = 'e';
			(*n)++;
		      }
		   if ( conf->roqueB != 'p' && conf->mat[0][5] == 0 && conf->mat[0][6] == 0 )
		      if ( !caseMenaceePar( MIN, 0, 4, conf ) && !caseMenaceePar( MIN, 0, 5, conf ) && \
			   !caseMenaceePar( MIN, 0, 6, conf ) )  {
			// Faire un petit roque ...
			copier(conf, &T[*n]);
			T[*n].mat[0][4] = 0;
			T[*n].mat[0][7] = 0;
			T[*n].mat[0][6] = 'r'; T[*n].xrB = 0; T[*n].yrB = 6;
			T[*n].mat[0][5] = 't';
			// aucun roque ne sera plus possible � partir de cette config
			T[*n].roqueB = 'e';
			(*n)++;
		      }
		}

		// v�rifier les autres mouvements du roi ...
		for (i=0; i<8; i += 1) {
		   // traitement des 8 directions paires et impaires
		   a = x + D[i][0];
		   b = y + D[i][1];
		   if ( a >= 0 && a <= 7 && b >= 0 && b <= 7 )
			if ( conf->mat[a][b] <= 0 ) {
			   copier(conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   T[*n].mat[a][b] = 'r'; T[*n].xrB = a; T[*n].yrB = b;
			   // cas o� le roi adverse est pris...
			   if (T[*n].xrN == a && T[*n].yrN == b) {
			      T[*n].xrN = -1;
			      T[*n].yrN = -1;
			   }
			   // aucun roque ne sera plus possible � partir de cette config
			   T[*n].roqueB = 'n';
			   (*n)++;
			}
		} // for
		break;

	}

} // fin de deplacementsB


/* G�n�re les successeurs de la configuration conf dans le tableau T,
   retourne aussi dans n le nombre de configurations filles g�n�r�es */
void generer_succ( struct config *conf, int mode, struct config T[], int *n )
{
	int i, j, k, stop;

	*n = 0;

	if ( mode == MAX ) {		// mode == MAX
	   for (i=0; i<8; i++)
	      for (j=0; j<8; j++)
		 if ( conf->mat[i][j] > 0 )
		    deplacementsB(conf, i, j, T, n );

	   // v�rifier si le roi est en echec, auquel cas on ne garde que les succ �vitants l'�chec
	   // ou alors si une conf est d�j� visit�e dans Partie, auquel cas on l'enl�ve aussi ...
	   for (k=0; k < *n; k++) {
	      	i = T[k].xrB; j = T[k].yrB;  // pos du roi B dans T[k]
		// v�rifier si roi menac� dans la config T[k] ou alors T[k] est dej� visit�e ...
		if ( caseMenaceePar( MIN, i, j, &T[k] ) || dejaVisitee( &T[k] ) ) {
		    T[k] = T[(*n)-1];	// alors supprimer T[k] de la liste des succ...
		    (*n)--;
		    k--;
		}
	    } // for k
	}

	else { 				// mode == MIN
	   for (i=0; i<8; i++)
	      for (j=0; j<8; j++)
		 if ( conf->mat[i][j] < 0 )
		    deplacementsN(conf, i, j, T, n );

	   // v�rifier si le roi est en echec, auquel cas on ne garde que les succ �vitants l'�chec
	   // ou alors si une conf est d�j� visit�e dans Partie, auquel cas on l'enl�ve aussi ...
	   for (k=0; k < *n; k++) {
		i = T[k].xrN; j = T[k].yrN;
		// v�rifier si roi menac� dans la config T[k] ou alors T[k] est dej� visit�e ...
		if ( caseMenaceePar( MAX, i, j, &T[k] ) || dejaVisitee( &T[k] ) ) {
		    T[k] = T[(*n)-1];	// alors supprimer T[k] de la liste des succ...
		    (*n)--;
		    k--;
		}
	   } // for k
	} // if (mode == MAX) ... else ...

} // fin de generer_succ



// ******************************
// Partie:  MinMax avec AlphaBeta
// ******************************

/* Fonctions de comparaison utilis�es avec qsort     */
// 'a' et 'b' sont des configurations

int confcmp123(const void *a, const void *b)	// pour l'ordre croissant
{
    int x = ((struct config *)a)->val, y = ((struct config *)b)->val;
    if ( x < y )
	return -1;
    if ( x == y )
	return 0;
    return 1;
}  // fin confcmp123

int confcmp321(const void *a, const void *b)	// pour l'ordre decroissant
{
    int x = ((struct config *)a)->val, y = ((struct config *)b)->val;
    if ( x > y )
	return -1;
    if ( x == y )
	return 0;
    return 1;
}  // fin confcmp321


/* MinMax avec �lagage alpha-beta :
 Evalue la configuration 'conf' du joueur 'mode' en descendant de 'niv' niveaux.
 Le param�tre 'niv' est decr�ment� � chaque niveau (appel r�cursif).
 'alpha' et 'beta' repr�sentent les bornes initiales de l'intervalle d'int�r�t
 (pour pouvoir effectuer les coupes alpha et b�ta).
 'largeur' repr�sente le nombre max d'alternatives � explorer en profondeur � chaque niveau.
 Si 'largeur == +INFINI' toutes les alternatives seront prises en compte
 (c'est le comportement par d�faut).
 'numFctEst' est le num�ro de la fonction d'estimation � utiliser lorsqu'on arrive � la
 fronti�re d'exploration (c-a-d 'niv' atteint 0)
*/
int minmax_ab( struct config *conf, int mode, int niv, int alpha, int beta, int largeur, int numFctEst )
{
 	int n, i, score, score2;
 	struct config T[100];

   	if ( feuille(conf, &score) )
		return score;

   	if ( niv == 0 )
		return Est[numFctEst]( conf );

   	if ( mode == MAX ) {

	   generer_succ( conf, MAX, T, &n );

 	   if ( largeur != +INFINI ) {
	    	for (i=0; i<n; i++)
		     T[i].val = Est[numFctEst]( &T[i] );

		qsort(T, n, sizeof(struct config), confcmp321);
		if ( largeur < n ) n = largeur;	  // pour limiter la largeur d'exploration
	   }

	   score = alpha;
	   for ( i=0; i<n; i++ ) {
   	    	score2 = minmax_ab( &T[i], MIN, niv-1, score, beta, largeur, numFctEst);
		if (score2 > score) score = score2;
		if (score >= beta) {
			// Coupe Beta
			nbBeta++;	// compteur de courpes beta
   	      		return beta;
	    	}
	   }
	}
	else  { // mode == MIN

	   generer_succ( conf, MIN, T, &n );

 	   if ( largeur != +INFINI ) {
	    	for (i=0; i<n; i++)
		     T[i].val = Est[numFctEst]( &T[i] );

		qsort(T, n, sizeof(struct config), confcmp123);
		if ( largeur < n ) n = largeur;	  // pour limiter la largeur d'exploration
	   }

	   score = beta;
	   for ( i=0; i<n; i++ ) {
   	    	score2 = minmax_ab( &T[i], MAX, niv-1, alpha, score, largeur, numFctEst );
		if (score2 < score) score = score2;
		if (score <= alpha) {
			// Coupe Alpha
			nbAlpha++;	// compteur de courpes alpha
   	      		return alpha;
	    	}
	   }
	}

        if ( score == +INFINI ) score = +100;
        if ( score == -INFINI ) score = -100;

	return score;

} // fin de minmax_ab


