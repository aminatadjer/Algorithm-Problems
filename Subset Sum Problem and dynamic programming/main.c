#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
int tab[300];
int x[100];
int i=0;
void remplirTableauRandom(int taille)
{
    int i;
    for (i=0;i<taille;i++){
     tab[i]= rand() % 30;
    }


}
int lireTableau()
{   int taille=0;
    int i,j;
    printf("Veuillez rentrer la taille du tableau :");
    scanf("%d",&taille);
    printf("\n");
    for(i=0;i<taille;i++){
            printf("Tab[%d]= ",i);
        scanf("%d",&j);
        printf("\n");
        tab[i]=j;
    }
    return taille;
}

int isSubsetSum(int n,int k)
{
    if ((n==0) && (k==0)) {return 1;}
    else {
        if ((n==0) && (k!=0)) {return 0;}
        else{
            return (isSubsetSum(n-1,k) || isSubsetSum(n-1,k-tab[n-1]));
        }
    }
}

int subSetSunAffiche(int n,int k)
{

     if ((n==0)&&(k==0)){ return 1;}
     else
    {
        if((n==0)&&(k!=0)) return 0;
        else
        {
            if (subSetSunAffiche(n-1,k))
            {
                x[n-1]=0;
                return 1;
            }
            else
            {
                if (subSetSunAffiche(n-1,k-tab[n-1]))
                {
                    x[n-1]=1;



                    return 1;
                }
                return 0;
            }

        }
    }
}

int subsetmin(int n,int k,int *cardinalite)
{

    int entre1=0,entre2=0,min1,min2,i;
     if ((n==0)&&(k==0)) {(*cardinalite)=0; return 1;}
     else{if((n==0)&&(k!=0)) return 0;}
     if (subsetmin(n-1,k,cardinalite))
     {
                entre1=1;
                min1=(*cardinalite);
     }
     if (subsetmin(n-1,k-tab[n-1],cardinalite))
     {
                entre2=1;
                (*cardinalite)++;
                min2=(*cardinalite);
     }
     if (entre1&&entre2)
     {
         if(min1<min2){
            (*cardinalite)=min1;
         return 1;
         }
         else{
            (*cardinalite)=min2;
            return 1;
         }
     }
     return entre1||entre2;
}

int minimun(int a,int b){
if (b<a) return b;
else return a;
}

int cardMinDyn(int n, int s){
int mat[n+1][s+1];
int i=0,j=0;
int a1,a2,b1,b2;
for(i=0;i<=n;i++){
    mat[i][0]=0;
}
for(j=1;j<=s;j++){
    mat[0][j]=n+1;
}
for (i=1;i<=n;i++){
    for(j=1;j<=s;j++){
        a1=i-1;
        a2=j;
        b1=i-1;
        b2=j-tab[i-1];
        if (b2<0) mat[i][j]=mat[a1][a2];
        else
        {
        if ((minimun(mat[a1][a2],mat[b1][b2]))== (mat[a1][a2]) ){
            mat[i][j]=mat[a1][a2];
        }
        else
        {mat[i][j]=(mat[b1][b2])+1;}


        }

    }


}

return mat[n][s];
}

void affichSol(int taille){
    int i;
    for (i=0;i<taille;i++){

            printf("tab[%d] = %d \n",i,tab[i]);

      }

      }



int main()
{   int taille,somme;
FILE * fichier =NULL;
fichier=fopen("temps.txt","a+");
printf("Veuillez rentrer la taille du tableau :");
scanf("%d",&taille);
  // taille=lireTableau();
  remplirTableauRandom(taille);
  affichSol(taille);
    int y;
    int i;
    printf(" Veuillez rentrer la somme que vous cherchez : ");
    scanf("%d",&somme);
/*
    printf("\n*****************************************************\n");
    printf("La solution de isSubsetSum est : ");
    printf("%d \n",isSubsetSum(taille,somme));
    printf("\n*****************************************************\n");
    printf(" Le premier ensemble rencontre est : \n");
    subSetSunAffiche(taille,somme);
    affichSol(taille);
    printf("\n*****************************************************\n");
    printf(" La solution avec un min de cardinalite : \n");
    subsetmin(taille,somme,&y);
    printf("la cardinalite minimale est :%d\n",y);
*/
int c;
struct timeval tv1,tv2;
long long temps1,temps2;
gettimeofday(&tv1,NULL);
c=cardMinDyn(taille,somme);
gettimeofday(&tv2,NULL);
temps1=(tv2.tv_sec*1000000 + tv2.tv_usec-tv1.tv_sec*1000000+tv1.tv_usec);
printf("La card minimale  est : %d \n",c);
printf("Le temps d'execution dynamique =   %lld us\n",temps1);

gettimeofday(&tv1,NULL);
subsetmin(taille,somme,&y);
gettimeofday(&tv2,NULL);
temps2=(tv2.tv_sec*1000000 + tv2.tv_usec-tv1.tv_sec*1000000+tv1.tv_usec);
printf("la cardinalite minimale est :%d \n",y);
printf("Le temps d'execution recursif =   %lld us \n",temps2);
fprintf(fichier,"%d             %lld us         %lld us\n",taille,temps1,temps2);
    return 0;
}
