/***********************************************/
/*  TP4: Problème du Voyageur de Commerce PVC  */
/*  Branch and Bound pure (f = g + h et h=0)   */
/*  Trouver une bonne fonction h pour avoir :  */
/*  Branch and Bound avec sous-estimation.     */
/*  La file de priorité est de type 'Heap'     */
/*  TP4 / 2CSSIL / TPGO / Hidouci / ESI-2019   */
/***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define INFINI 1000000000

// Taille de la file de priorité
#define MAXFILE 50000000

int **poids;    // matrice des poids (un tableau de tableaux d'entier)
int n;		// nb de sommets dans le graphe
int nbElemMax;	// le plus grand nombre d'éléments présents dans la file

// structure de données pour un chemin
typedef struct tchemin {
	int ns;		// nombre de sommets dans le chemin
	int cout_g;	// coût du chemin parcouru
        int cout_h;	// cout estimé du trajet restant
	int *chem;	// le chemin (un tableau de sommets)
} TypeChemin;

// la file de chemins
typedef struct tfile {
	TypeChemin **tab;	// Tableau géré en Heap (tas)
	int nbElem;		// nombre d'éléments dans la file
} TypeFile;


// les principales fonctions ...
int fonc_H( TypeChemin *e );		// la fonction de sous-estimation: h(e)
void init_graph( int nbsommet );	// initialise un graphe par des poids aléatoires
void charger_graph(char *nomfich);	// charger une matrice des poids à partir d'un fichier
void branch_bound( int depart );	// parcours branch&bound à partir du sommet 'depart'
void affich();				// affiche et sauveagarde la matrice des poids dans un fichier
int Existe_dans_chemin(int j, TypeChemin *ch);	// verifie si le sommet j appartient au chemin ch
void CreerFile( TypeFile *f );			// file de priorité (Heap)
void Enfiler( TypeFile *f, TypeChemin *ch );	// enfiler un chemin
void Defiler( TypeFile *f, TypeChemin **ch );	// defiler le chemin le plus prioritaire (min(g+h))
int FileVide( TypeFile f );			// tester si la file est vide


/*****************************************/
/* Programme Principal                   */
/*****************************************/

int main()
{
   int n;
   char nomfich[40];

   do {
	printf("1) Générer un graphe complet aléatoire\n");
	printf("2) Lire le graphe à partir d'un fichier donné\n");
	printf("Choix : ");
	scanf(" %d", &n);
   }
   while (n != 1 && n != 2);

   if ( n == 1 ) {
	printf("Nombre de sommets : ");
	scanf(" %d", &n);
      	init_graph( n );
   }
   else {
	printf("Nom du fichier contenant le graphe : ");
	scanf(" %s", nomfich);
      	charger_graph( nomfich );
   }

   affich();

   printf("PVC avec Branch and Bound pure (file de priorité de type Heap)\n\n");

   branch_bound(0);

   return 0;

} // main




/*****************************************/
/* Implémentation des autres fonctions   */
/*****************************************/


// la fonction de sous-estimation h
int fonc_H( TypeChemin *e )
{
   int h = 1222;
   int i=0;
   int p;
   int k=e->ns;
   int j=e->chem[k-1];
   for(i=0;i<n;i++){
    if ((Existe_dans_chemin(i,e)== 0) && (j!=i)) {
        p=(poids[j][i]);

    if (p<h) h=p;
    }
   }
   if (h==1222){
    h=poids[j][0];
   }

   // Placer le code de votre fonction de sous-estimation ici ...
   // ...

   return 0;
}

// fin de la fonction d'estimation h


// initialise la matrice (symétrique) des coûts avec des valeurs aléatoire
void init_graph( int nbsommet )
{
   int i, j;
   n = nbsommet;

   // la matrice poids est représentée sous forme d'un tableau de n tableaux de n entiers
   poids = malloc( n*sizeof(int *) );
   for (i=0; i<n; i++)
	poids[i] = malloc( n*sizeof(int) );

   srand((unsigned int)time((time_t *)NULL));
   for (i=0; i<n; i++) {
	for (j=i+1; j<n; j++) {
	     poids[i][j] = 1+(int) (20.0*rand()/(RAND_MAX+1.0));	// entre 1 et 20
	     poids[j][i] = poids[i][j];
	}
	poids[i][i] = 0;
   }
}


// initialise la matrice (symétrique) des coûts à partir du fichier texte nomfich
void charger_graph(char *nomfich)
{
   int i, j;
   FILE *f = fopen( nomfich, "r" );
   // récupérer le nombre de sommets
   fscanf(f, " %d", &n);

   // la matrice poids est représentée sous forme d'un tableau de n tableaux de n entiers
   poids = malloc( n * sizeof(int *) );
   for (i=0; i<n; i++)
	poids[i] = malloc( n * sizeof(int) );

   // récupérer les élmts de la matrice (représentée ligne par ligne)
   for (i=0; i<n; i++)
	for (j=0; j<n; j++)
	    fscanf(f, " %d", &poids[i][j]);
   fclose(f);
}


// Recherche de la solution optimale par branch & bound avec sous-estimation (f = g+h)
void branch_bound( int depart )
{

   TypeFile f;
   int i, j, k, stop, nbIter;
   int cout_opt = INFINI;

   TypeChemin *nouv = NULL;

   TypeChemin *cycle_opt = malloc( sizeof(TypeChemin) );
   cycle_opt->chem = malloc( (n+1)*sizeof(int) );

   TypeChemin *ch = malloc( sizeof(TypeChemin) );
   ch->chem = malloc( sizeof(int) );
   ch->ns = 1;
   ch->chem[0] = depart;
   ch->cout_g = 0;
   ch->cout_h = 0;

   CreerFile( &f );
   Enfiler(&f, ch);
   stop = 0;
   nbIter = 0;
   printf("Début de Branch & Bound\n");

   // Boucle principale de Branch_and_Bound
   while ( !FileVide(f) && !stop ) {
	Defiler( &f, &ch );	// défiler le chemin le plus prioritaire
	if ( ch->ns == n+1 ) {	// s'il contient tous les sommets, alors c'est la solution optimale
	   // MAJ cycle optimal
	   cout_opt = ch->cout_g + poids[ch->chem[ch->ns-1]][depart];

   	   for (i=0; i < n; i++)
   	   	cycle_opt->chem[i] = ch->chem[i];
	   cycle_opt->chem[n] = depart;
   	   cycle_opt->ns = n+1;
   	   cycle_opt->cout_g = cout_opt;
	   cycle_opt->cout_h = 0;

	   stop = 1;
	}
	else {  		// sinon on n'a pas encore atteint la solution ...
	   if ( ch->ns < n ) {
	      i = ch->chem[ ch->ns-1 ]; // i: le dernier sommet du chemin ch
	      for (j=0; j<n; j++)       // générer les succ et enfiler les chemins ...
		if ( j != i && !Existe_dans_chemin(j,ch) )  {
		   nouv = malloc( sizeof(TypeChemin) );
		   nouv->chem = malloc( (ch->ns+1)*sizeof(int) );
		   for (k=0; k < ch->ns; k++)
   		    	nouv->chem[k] = ch->chem[k];
		   nouv->chem[ ch->ns ] = j;
		   nouv->ns = ch->ns+1;
		   nouv->cout_g = ch->cout_g + poids[i][j];
		   nouv->cout_h = fonc_H( nouv );
		   Enfiler( &f, nouv );
		}
	   }
	   else {
	      i = ch->chem[ ch->ns-1 ]; // i: le dernier sommet du chemin ch
	      j = 0;
	      nouv = malloc( sizeof(TypeChemin) );
	      nouv->chem = malloc( (ch->ns+1)*sizeof(int) );
	      for (k=0; k < ch->ns; k++)
   		   nouv->chem[k] = ch->chem[k];
	      nouv->chem[ ch->ns ] = j;
	      nouv->ns = ch->ns+1;
	      nouv->cout_g = ch->cout_g + poids[i][j];
	      nouv->cout_h = fonc_H( nouv );
	      Enfiler( &f, nouv );
 	   }
     	} // endif
	free(ch->chem);
	free(ch);

	nbIter++;	// Compteur d'itérations dans la boucle principale

   } // fin de la boucle principale while

   printf("\nFin de Branch & Bound \tNb itérations = %d \t Taille max de la file = %d\n", \
		nbIter, nbElemMax );
   // Affichage de la solution optimale
   printf("Le cycle hamiltonien de poids minimum est :\n");
   for (i=0; i <= n; i++)
	printf("%3d ", cycle_opt->chem[i]);
   printf("Son cout est = %3d\n", cycle_opt->cout_g);

} // branch_bound


// Affichage de la matrice des coûts et sauvegarde dans un fichier texte"
void affich()
{

   int i, j, sauv = 0;
   char nomfich[40];
   FILE *f;

   printf("Affichage du graphe en cours\n");
   printf("Donnez le nom du fichier où sera sauvegarder la matrice (ou 0 pour ne pas sauvegarder) : ");
   scanf(" %s", nomfich);
   if (nomfich[0] != '0' ) sauv = 1;

   if (sauv) {
   	f = fopen(nomfich, "w");
   	if ( f == NULL ) {
	   printf("Problème dans l'ouverture du fichier %s. Pas de sauvegarde\n", nomfich);
	   sauv = 0;
	}
	else
	   fprintf(f, "%d\n", n);
   }

   printf("\n Matrice des poids des aretes:\n    ");
   for (i=0; i<n; i++)
	printf("%4d ", i);
   printf("\n    ");
   for (i=0; i<n; i++)
	printf("-----");
   printf("\n");
   for (i=0; i<n; i++) {
	printf("%3d|", i);
	for (j=0; j<n; j++) {
	    printf("%4d ", poids[i][j]);
	    if ( sauv )
	    	fprintf(f, "%4d", poids[i][j]);
	}
	printf("\n");
	if ( sauv )
	   fprintf(f, " \n");
   }
   printf("\n");
   if ( sauv )
      fclose(f);

} // affich


// vérifie si le sommet j existe déjà dans le chemin ch
int Existe_dans_chemin( int j, TypeChemin *ch)
{
   int i;
   int trouv = 0;
   for (i=0; i < ch->ns && !trouv; i++)
	if ( j == ch->chem[i] ) trouv = 1;
   return trouv;
}


// Allouer un tableau de MAXFIEL elt pour le Heap et initialise la file à vide
void CreerFile( TypeFile *f )
{
   f->tab = malloc(MAXFILE * sizeof(TypeChemin **));
   f->nbElem = 0;
   nbElemMax = 0;
}


// Enfiler le chemin ch dans le Heap (en O(log n))
void Enfiler( TypeFile *f, TypeChemin *ch )
{
   int i, stop;
   TypeChemin *tmp;

   if ( f->nbElem < MAXFILE ) {
	f->tab[ f->nbElem ] = ch;
	i = f->nbElem;
	stop = 0;
	while ( !stop && i > 0 )
	   if ( (f->tab[i]->cout_g + f->tab[i]->cout_h) >= \
		(f->tab[(i+1)/2-1]->cout_g + f->tab[(i+1)/2-1]->cout_h) )
		stop = 1;
	   else {
		tmp = f->tab[i];
		f->tab[i] = f->tab[(i+1)/2-1];
		f->tab[(i+1)/2-1] = tmp;
		i = (i+1)/2-1;
	   }  // if  et  while
	f->nbElem++;
	if ( f->nbElem > nbElemMax ) nbElemMax = f->nbElem;
   }
   else {
   	printf("*** Enfiler: overflow de la file de priorité\n");
	exit(0);
   }

}


// Défiler dans ch l'élément le plus prioritaire du Heap (en O(log n))
void Defiler( TypeFile *f, TypeChemin **ch )
{
   int i, stop, ind, minprio;
   TypeChemin *tmp;

   if ( f->nbElem > 0 ) {
	*ch = f->tab[0];
	f->tab[0] = f->tab[ f->nbElem-1 ];
	f->nbElem--;
	i = 0;
	stop = 0;
	while ( !stop && i < (f->nbElem / 2) )  {
	   ind = -1;
	   // comparer avec le plus petit des fils (le gauche et le droit s'il existe) ...
	   if ( (f->tab[2*i+1]->cout_g + f->tab[2*i+1]->cout_h) <= \
		(f->tab[2*i+2]->cout_g + f->tab[2*i+2]->cout_h) ) {
		minprio = f->tab[2*i+1]->cout_g + f->tab[2*i+1]->cout_h;
		ind = 2*i+1;
 	   }
	   else
	      if ( (2*i+2) < f->nbElem ) {
		minprio = f->tab[2*i+2]->cout_g + f->tab[2*i+2]->cout_h;
		ind = 2*i+2;
	      }

	   if ( ind != -1 ) {
	      if ( (f->tab[i]->cout_g + f->tab[i]->cout_h) > minprio ) {
		 tmp = f->tab[i];
		 f->tab[i] = f->tab[ind];
		 f->tab[ind] = tmp;
		 i = ind;
	      }
	      else
		stop = 1;
	   } // partie 'alors' du if ( ind != -1 )
	   else
	      stop = 1;

	} // while
   }
   else {
   	printf("*** Defiler: underflow de la file de priorité\n");
	exit(0);
   }
}


// Tester si la file est vide
int FileVide( TypeFile f )
{
   return (f.nbElem == 0);
}


