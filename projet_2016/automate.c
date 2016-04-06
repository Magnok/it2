/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
 *
 *   Copyright (C) 2014, 2015 Adrien Boussicault
 *
 *    This Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This Library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this Library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "automate.h"
#include "table.h"
#include "ensemble.h"
#include "outils.h"
#include "fifo.h"
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

/* De manière similaire a get_min_etat, get_max_etat utilise 
 * pour_tout_element et va stocker le numéro de l'état le 
 * plus grand dans max qui sera modifié dans  
 * action_get_max_etat
 */
void action_get_max_etat(const intptr_t element,void* data){
  int * max = (int*)data;
  if(*max < element)
    *max = element;
}

int get_max_etat( const Automate* automate ){
  int max = -1;
  pour_tout_element(automate->etats,action_get_max_etat,&max);
  return max;
}

void action_get_min_etat( const intptr_t element, void* data ){
  int * min = (int*) data;
  if( *min > element ) *min = element;
}

int get_min_etat( const Automate* automate ){
  int min = INT_MAX;

  pour_tout_element( automate->etats, action_get_min_etat, &min );

  return min;
}


int comparer_cle(const Cle *a, const Cle *b) {
  if( a->origine < b->origine )
    return -1;
  if( a->origine > b->origine )
    return 1;
  if( a->lettre < b->lettre )
    return -1;
  if( a->lettre > b->lettre )
    return 1;
  return 0;
}

void print_cle( const Cle * a){
  printf( "(%d, %c)" , a->origine, (char) (a->lettre) );
}

void supprimer_cle( Cle* cle ){
  xfree( cle );
}

void initialiser_cle( Cle* cle, int origine, char lettre ){
  cle->origine = origine;
  cle->lettre = (int) lettre;
}

Cle * creer_cle( int origine, char lettre ){
  Cle * result = xmalloc( sizeof(Cle) );
  initialiser_cle( result, origine, lettre );
  return result;
}

Cle * copier_cle( const Cle* cle ){
  return creer_cle( cle->origine, cle->lettre );
}

Automate * creer_automate(){
  Automate * automate = xmalloc( sizeof(Automate) );
  automate->etats = creer_ensemble( NULL, NULL, NULL );
  automate->alphabet = creer_ensemble( NULL, NULL, NULL );
  automate->transitions = creer_table(
				      ( int(*)(const intptr_t, const intptr_t) ) comparer_cle , 
				      ( intptr_t (*)( const intptr_t ) ) copier_cle,
				      ( void(*)(intptr_t) ) supprimer_cle
				      );
  automate->initiaux = creer_ensemble( NULL, NULL, NULL );
  automate->finaux = creer_ensemble( NULL, NULL, NULL );
  automate->vide = creer_ensemble( NULL, NULL, NULL ); 
  return automate;
}

Automate * translater_automate_entier( const Automate* automate, int translation ){
  Automate * res = creer_automate();

  Ensemble_iterateur it;
  for( 
      it = premier_iterateur_ensemble( get_etats( automate ) );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
       ){
    ajouter_etat( res, get_element( it ) + translation );
  }

  for( 
      it = premier_iterateur_ensemble( get_initiaux( automate ) );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
       ){
    ajouter_etat_initial( res, get_element( it ) + translation );
  }

  for( 
      it = premier_iterateur_ensemble( get_finaux( automate ) );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
       ){
    ajouter_etat_final( res, get_element( it ) + translation );
  }

  // On ajoute les lettres
  for(
      it = premier_iterateur_ensemble( get_alphabet( automate ) );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
      ){
    ajouter_lettre( res, (char) get_element( it ) );
  }

  Table_iterateur it1;
  Ensemble_iterateur it2;
  for(
      it1 = premier_iterateur_table( automate->transitions );
      ! iterateur_est_vide( it1 );
      it1 = iterateur_suivant_table( it1 )
      ){
    Cle * cle = (Cle*) get_cle( it1 );
    Ensemble * fins = (Ensemble*) get_valeur( it1 );
    for(
	it2 = premier_iterateur_ensemble( fins );
	! iterateur_ensemble_est_vide( it2 );
	it2 = iterateur_suivant_ensemble( it2 )
	){
      int fin = get_element( it2 );
      ajouter_transition(
			 res, cle->origine + translation, cle->lettre, fin + translation
			 );
    }
  };

  return res;
}


void liberer_automate( Automate * automate ){
  assert( automate );
  liberer_ensemble( automate->vide );
  liberer_ensemble( automate->finaux );
  liberer_ensemble( automate->initiaux );
  pour_toute_valeur_table(
			  automate->transitions, ( void(*)(intptr_t) ) liberer_ensemble
			  );
  liberer_table( automate->transitions );
  liberer_ensemble( automate->alphabet );
  liberer_ensemble( automate->etats );
  xfree(automate);
}

const Ensemble * get_etats( const Automate* automate ){
  return automate->etats;
}

const Ensemble * get_initiaux( const Automate* automate ){
  return automate->initiaux;
}

const Ensemble * get_finaux( const Automate* automate ){
  return automate->finaux;
}

const Ensemble * get_alphabet( const Automate* automate ){
  return automate->alphabet;
}

void ajouter_etat( Automate * automate, int etat ){
  ajouter_element( automate->etats, etat );
}

void ajouter_lettre( Automate * automate, char lettre ){
  ajouter_element( automate->alphabet, lettre );
}

void ajouter_transition(
			Automate * automate, int origine, char lettre, int fin
			){
  ajouter_etat( automate, origine );
  ajouter_etat( automate, fin );
  ajouter_lettre( automate, lettre );

  Cle cle;
  initialiser_cle( &cle, origine, lettre );
  Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
  Ensemble * ens;
  if( iterateur_est_vide( it ) ){
    ens = creer_ensemble( NULL, NULL, NULL );
    add_table( automate->transitions, (intptr_t) &cle, (intptr_t) ens );
  }else{
    ens = (Ensemble*) get_valeur( it );
  }
  ajouter_element( ens, fin );
}

void ajouter_etat_final(
			Automate * automate, int etat_final
			){
  ajouter_etat( automate, etat_final );
  ajouter_element( automate->finaux, etat_final );
}

void ajouter_etat_initial(
			  Automate * automate, int etat_initial
			  ){
  ajouter_etat( automate, etat_initial );
  ajouter_element( automate->initiaux, etat_initial );
}

const Ensemble * voisins( const Automate* automate, int origine, char lettre ){
  Cle cle;
  initialiser_cle( &cle, origine, lettre );
  Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
  if( ! iterateur_est_vide( it ) ){
    return (Ensemble*) get_valeur( it );
  }else{
    return automate->vide;
  }
}

Ensemble * delta1(
		  const Automate* automate, int origine, char lettre
		  ){
  Ensemble * res = creer_ensemble( NULL, NULL, NULL );
  ajouter_elements( res, voisins( automate, origine, lettre ) );
  return res; 
}

Ensemble * delta(
		 const Automate* automate, const Ensemble * etats_courants, char lettre
		 ){
  Ensemble * res = creer_ensemble( NULL, NULL, NULL );

  Ensemble_iterateur it;
  for( 
      it = premier_iterateur_ensemble( etats_courants );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
       ){
    const Ensemble * fins = voisins(
				    automate, get_element( it ), lettre
				    );
    ajouter_elements( res, fins );
  }

  return res;
}

Ensemble * delta_star(
		      const Automate* automate, const Ensemble * etats_courants, const char* mot
		      ){
  int len = strlen( mot );
  int i;
  Ensemble * old = copier_ensemble( etats_courants );
  Ensemble * new = old;
  for( i=0; i<len; i++ ){
    new = delta( automate, old, *(mot+i) );
    liberer_ensemble( old );
    old = new;
  }
  return new;
}

void pour_toute_transition(
			   const Automate* automate,
			   void (* action )( int origine, char lettre, int fin, void* data ),
			   void* data
			   ){
  Table_iterateur it1;
  Ensemble_iterateur it2;
  for(
      it1 = premier_iterateur_table( automate->transitions );
      ! iterateur_est_vide( it1 );
      it1 = iterateur_suivant_table( it1 )
      ){
    Cle * cle = (Cle*) get_cle( it1 );
    Ensemble * fins = (Ensemble*) get_valeur( it1 );
    for(
	it2 = premier_iterateur_ensemble( fins );
	! iterateur_ensemble_est_vide( it2 );
	it2 = iterateur_suivant_ensemble( it2 )
	){
      int fin = get_element( it2 );
      action( cle->origine, cle->lettre, fin, data );
    }
  };
}

Automate* copier_automate( const Automate* automate ){
  Automate * res = creer_automate();
  Ensemble_iterateur it1;
  // On ajoute les états de l'automate
  for(
      it1 = premier_iterateur_ensemble( get_etats( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_etat( res, get_element( it1 ) );
  }
  // On ajoute les états initiaux
  for(
      it1 = premier_iterateur_ensemble( get_initiaux( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_etat_initial( res, get_element( it1 ) );
  }
  // On ajoute les états finaux
  for(
      it1 = premier_iterateur_ensemble( get_finaux( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_etat_final( res, get_element( it1 ) );
  }
  // On ajoute les lettres
  for(
      it1 = premier_iterateur_ensemble( get_alphabet( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_lettre( res, (char) get_element( it1 ) );
  }
  // On ajoute les transitions
  Table_iterateur it2;
  for(
      it2 = premier_iterateur_table( automate->transitions );
      ! iterateur_est_vide( it2 );
      it2 = iterateur_suivant_table( it2 )
      ){
    Cle * cle = (Cle*) get_cle( it2 );
    Ensemble * fins = (Ensemble*) get_valeur( it2 );
    for(
	it1 = premier_iterateur_ensemble( fins );
	! iterateur_ensemble_est_vide( it1 );
	it1 = iterateur_suivant_ensemble( it1 )
	){
      int fin = get_element( it1 );
      ajouter_transition( res, cle->origine, cle->lettre, fin );
    }
  }
  return res;
}

Automate * translater_automate(
			       const Automate * automate, const Automate * automate_a_eviter
			       ){
  if(
     taille_ensemble( get_etats(automate) ) == 0 ||
     taille_ensemble( get_etats(automate_a_eviter) ) == 0
     ){
    return copier_automate( automate );
  }
	
  int translation = 
    get_max_etat( automate_a_eviter ) - get_min_etat( automate ) + 1; 

  return translater_automate_entier( automate, translation );
	
}

int est_une_transition_de_l_automate(
				     const Automate* automate,
				     int origine, char lettre, int fin
				     ){
  return est_dans_l_ensemble( voisins( automate, origine, lettre ), fin );
}

int est_un_etat_de_l_automate( const Automate* automate, int etat ){
  return est_dans_l_ensemble( get_etats( automate ), etat );
}

int est_un_etat_initial_de_l_automate( const Automate* automate, int etat ){
  return est_dans_l_ensemble( get_initiaux( automate ), etat );
}

int est_un_etat_final_de_l_automate( const Automate* automate, int etat ){
  return est_dans_l_ensemble( get_finaux( automate ), etat );
}

int est_une_lettre_de_l_automate( const Automate* automate, char lettre ){
  return est_dans_l_ensemble( get_alphabet( automate ), lettre );
}

void print_ensemble_2( const intptr_t ens ){
  print_ensemble( (Ensemble*) ens, NULL );
}

void print_lettre( intptr_t c ){
  printf("%c", (char) c );
}

void print_automate( const Automate * automate ){
  printf("- Etats : ");
  print_ensemble( get_etats( automate ), NULL );
  printf("\n- Initiaux : ");
  print_ensemble( get_initiaux( automate ), NULL );
  printf("\n- Finaux : ");
  print_ensemble( get_finaux( automate ), NULL );
  printf("\n- Alphabet : ");
  print_ensemble( get_alphabet( automate ), print_lettre );
  printf("\n- Transitions : ");
  print_table( 
	      automate->transitions,
	      ( void (*)( const intptr_t ) ) print_cle, 
	      ( void (*)( const intptr_t ) ) print_ensemble_2,
	      ""
	       );
  printf("\n");
}

int le_mot_est_reconnu( const Automate* automate, const char* mot ){
  Ensemble * arrivee = delta_star( automate, get_initiaux(automate) , mot ); 
	
  int result = 0;

  Ensemble_iterateur it;
  for(
      it = premier_iterateur_ensemble( arrivee );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
      ){
    if( est_un_etat_final_de_l_automate( automate, get_element(it) ) ){
      result = 1;
      break;
    }
  }
  liberer_ensemble( arrivee );
  return result;
}

Automate * mot_to_automate( const char * mot ){
  Automate * automate = creer_automate();
  int i = 0;
  int size = strlen( mot );
  for( i=0; i < size; i++ ){
    ajouter_transition( automate, i, mot[i], i+1 );
  }
  ajouter_etat_initial( automate, 0 );
  ajouter_etat_final( automate, size );
  return automate;
}

/* Pour créer l'union, on fait l'union des alphabets, l'union des états, ainsi que 
 * les états initaux et les états finaux et l'union des translations des deux
 * automates
*/
Automate * creer_union_des_automates(
				     const Automate * automate_1, const Automate * automate_2
				     ){
  Automate* ret = creer_automate();
  /* On évite le cas où les automates ont des états qui ont le meme numéro */
  Automate* automate_2bis = translater_automate(automate_2,automate_1);

  Ensemble const* etats_1 = get_etats(automate_1);
  Ensemble const* etats_2 = get_etats(automate_2bis);
  Ensemble* etats = creer_union_ensemble(etats_1,etats_2);

  Ensemble const* etats_in_1 = get_initiaux(automate_1);
  Ensemble const* etats_in_2 = get_initiaux(automate_2bis);
  Ensemble* etats_in = creer_union_ensemble(etats_in_1,etats_in_2);

  Ensemble const* etats_fin_1 = get_finaux(automate_1);
  Ensemble const* etats_fin_2 = get_finaux(automate_2bis);
  Ensemble* etats_fin = creer_union_ensemble(etats_fin_1,etats_fin_2);

  Ensemble const* alphabet_1 = get_alphabet(automate_1);
  Ensemble const* alphabet_2 = get_alphabet(automate_2bis);
  Ensemble* alphabet = creer_union_ensemble(alphabet_1,alphabet_2);

  Ensemble_iterateur it1,it2,it3;
  Ensemble* etats_accessibles;
  /* On récupère chaque transition de l'automate 1 */
  for(it1 = premier_iterateur_ensemble(etats_1);
      ! iterateur_ensemble_est_vide(it1);
      it1 = iterateur_suivant_ensemble(it1)){
    for(it2 = premier_iterateur_ensemble(alphabet_1);
	! iterateur_ensemble_est_vide(it2);
	it2 = iterateur_suivant_ensemble(it2)){
      etats_accessibles = delta1(automate_1,get_element(it1),get_element(it2));
      for(it3 = premier_iterateur_ensemble(etats_accessibles);
	! iterateur_ensemble_est_vide(it3);
	it3 = iterateur_suivant_ensemble(it3)){
	ajouter_transition(ret, get_element(it1), get_element(it2), get_element(it3));
      }    
    }    
  }
  /* De meme pour l'automate 2 */
  for(it1 = premier_iterateur_ensemble(etats_2);
      ! iterateur_ensemble_est_vide(it1);
      it1 = iterateur_suivant_ensemble(it1))
    {
      for(it2 = premier_iterateur_ensemble(alphabet_2);
	  ! iterateur_ensemble_est_vide(it2);
	  it2 = iterateur_suivant_ensemble(it2))
	{
	  etats_accessibles = delta1(automate_2bis,get_element(it1),get_element(it2));
	  for(it3 = premier_iterateur_ensemble(etats_accessibles);
	      ! iterateur_ensemble_est_vide(it3);
	      it3 = iterateur_suivant_ensemble(it3))
	    {
	      ajouter_transition(ret, get_element(it1), get_element(it2), get_element(it3));
	    }    
	}    
    }
  liberer_automate(automate_2bis);
  liberer_ensemble(ret->etats);
  liberer_ensemble(ret->alphabet);
  liberer_ensemble(ret->initiaux);
  liberer_ensemble(ret->finaux);

  ret->etats = etats;
  ret->alphabet = alphabet;
  ret->initiaux = etats_in;
  ret->finaux = etats_fin;  
  return ret;
}
/* pour chaque etats :
 * utilise la fonction delta1 appliquée sur chaque lettre de l'alphabet
 * creation d'un ensemble local (propre à l'état) d'états accessibles
 * fonction auxiliaire relancé sur tout les états n'ayant pas encore été traité. 
*/
Ensemble * aux_etats_accessibles(const Automate * automate, const Ensemble * alphabet, int etat, Ensemble * etatsAcc){
  Ensemble * etatsAccLocaux = creer_ensemble(NULL,NULL,NULL);
  Ensemble_iterateur it;
  
  for(it = premier_iterateur_ensemble(alphabet);
      ! iterateur_ensemble_est_vide(it);
      it = iterateur_suivant_ensemble(it)){
    etatsAccLocaux = creer_union_ensemble(etatsAccLocaux,delta1(automate,etat,get_element(it)));
  }
  Ensemble * etatsNonTraites = creer_difference_ensemble(etatsAccLocaux,etatsAcc);
  etatsAcc = creer_union_ensemble(etatsAcc,etatsAccLocaux);
  for(it = premier_iterateur_ensemble(etatsNonTraites);
      ! iterateur_ensemble_est_vide(it);
      it = iterateur_suivant_ensemble(it)){
    etatsAcc = aux_etats_accessibles(automate,alphabet,get_element(it),etatsAcc);
  }
  return etatsAcc;
}
  
					  				 
Ensemble * etats_accessibles( const Automate * automate, int etat ){
  Ensemble * etatsAcc = creer_ensemble(NULL,NULL,NULL);
  ajouter_element(etatsAcc,etat);
  const Ensemble * alphabet = get_alphabet(automate);
  return aux_etats_accessibles(automate, alphabet, etat, etatsAcc);
    }

/* Pour récupérer les états accessibles depuis les états initiaux, 
 * en se servant donc de etats_accessibles sur les états initiaux 
*/

Ensemble* accessibles( const Automate * automate ){
 	Ensemble* ret = copier_ensemble(get_initiaux(automate));
 	Ensemble_iterateur it;
 	for(
 		it = premier_iterateur_ensemble(get_initiaux(automate)); 
 		! iterateur_est_vide(it);
 		it = iterateur_suivant_ensemble(it)
 		){
 		ret = creer_union_ensemble(ret,etats_accessibles(automate,get_element(it)));
 }
 return ret;
}

struct suppr_transition{
  Ensemble * etats_acc;
  Automate * automate_accessible;
};
void action_suppr_transition_si_origine_inaccessible(int origine, char lettre, int fin, void * data){
  struct suppr_transition * data_struct = (struct suppr_transition *) data;
  if(est_dans_l_ensemble(data_struct->etats_acc, origine))
    ajouter_transition(data_struct->automate_accessible, origine, lettre, fin);
}
/*
  Crée un nouvel automate ayant pour états finals uniquement ceux étant accessibles.
  Copie les transitions de l'automate donnée en paramètre si et seulement si l'état d'origine
  de la transition est accessible.
*/
Automate *automate_accessible( const Automate * automate ){
  Automate* ret = creer_automate();
  Ensemble * etats_acc = accessibles(automate);
  ret->initiaux = copier_ensemble(get_initiaux(automate));
  ret->finaux = creer_intersection_ensemble(get_finaux(automate),etats_acc);
  struct suppr_transition * data = malloc(sizeof(*data));
  data->etats_acc = etats_acc;
  data->automate_accessible = ret;
  pour_toute_transition(automate, action_suppr_transition_si_origine_inaccessible,(void *) data);
  liberer_ensemble(etats_acc);
  free(data);
  return ret;
}

/* Pour créer l'automate, on échange états initiaux et finaux
*	 et on change toute transition d(q,alpha) = q0 en d'(q0,alpha) = q
*/
Automate *miroir( const Automate * automate){
	Automate* ret = creer_automate();
	ret->initiaux = copier_ensemble(get_finaux(automate));
	ret->finaux = copier_ensemble(get_initiaux(automate));
	Table_iterateur it_trans;
	Ensemble_iterateur it_ens;
	for(
		it_trans = premier_iterateur_table(automate->transitions);
		!iterateur_est_vide(it_trans); 
		it_trans = iterateur_suivant_table(it_trans)
		){
		Cle * cle = (Cle*) get_cle( it_trans );
		Ensemble * fins = (Ensemble*) get_valeur( it_trans );
		for(
			it_ens = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it_ens );
			it_ens = iterateur_suivant_ensemble( it_ens )
			){
				int fin = get_element( it_ens );
				ajouter_transition( ret, fin, cle->lettre, cle->origine );
			}
		}
	return ret;
}


/* Afin de créer l'automate du mélange on crée les états produits 
   des automates A1 et A2 (i,j) tq i et j appartiennent respectivement à A1 et A2.

   Ensuite, nous ajoutons une transition (p,a,q)
   (a un mot de l'union des alphabet de A1 et A2, p = (i,j) et q = (i',j'))
   Si et seulement si il existe une transition (i,a,i') ou (j,a,j')

   Les états initiaux de cet automate sont les couples (i,j) tq i et j initiaux
   (de même pour les finals). 

   Convention de nommage : l'état (i,j) est représenté par l'entier : (i >> 16) + j  
*/

int nommer_etat(int i,int j){
  return (i >> 16) + j;
}
Automate * creer_automate_du_melange(
	const Automate* automate_1,  const Automate* automate_2
	){
  Automate * automate_melange = creer_automate();
  Ensemble const * etats_1 = get_etats(automate_1);
  Ensemble const * etats_2 = get_etats(automate_2);
  Ensemble const * alphabet_1 = get_alphabet(automate_1);
  Ensemble const * alphabet_2 = get_alphabet(automate_2);
  Ensemble const * initiaux_1 = get_initiaux(automate_1);
  Ensemble const * initiaux_2 = get_initiaux(automate_2);
  Ensemble const * finaux_1 = get_finaux(automate_1);
  Ensemble const * finaux_2 = get_finaux(automate_2);
  Ensemble * initiaux_melange, * finaux_melange;
  initiaux_melange = creer_ensemble(NULL,NULL,NULL);
  finaux_melange = creer_ensemble(NULL,NULL,NULL);
  Ensemble * etats_accessibles;
	
  Ensemble_iterateur it1, it2, it_alph1, it_alph2, it_access;

  for(
      it1 = premier_iterateur_ensemble(etats_1);
      !iterateur_ensemble_est_vide(it1);
      it1 = iterateur_suivant_ensemble(it1)){
    for(
	it2 = premier_iterateur_ensemble(etats_2);
	!iterateur_ensemble_est_vide(it2);
	it2 = iterateur_suivant_ensemble(it2)){
      int i = get_element(it1);
      int j = get_element(it2);
      for(
	  it_alph1 = premier_iterateur_ensemble(alphabet_1);
	  !iterateur_ensemble_est_vide(it_alph1);
	  it_alph1 = iterateur_suivant_ensemble(it_alph1)){
	etats_accessibles = delta1(automate_1, i,get_element(it_alph1));
	for(
	    it_access = premier_iterateur_ensemble(etats_accessibles);
	    !iterateur_ensemble_est_vide(it_access);
	    it_access = iterateur_suivant_ensemble(it_access)){
	  ajouter_transition(automate_melange,nommer_etat(i,j), get_element(it_alph1) , nommer_etat(get_element(it_access), j));
	}				   
      }
      for(
	  it_alph2 = premier_iterateur_ensemble(alphabet_2);
	  !iterateur_ensemble_est_vide(it_alph2);
	  it_alph2 = iterateur_suivant_ensemble(it_alph2)){
	etats_accessibles = delta1(automate_2, i,get_element(it_alph2));
	for(
	    it_access = premier_iterateur_ensemble(etats_accessibles);
	    !iterateur_ensemble_est_vide(it_access);
	    it_access = iterateur_suivant_ensemble(it_access)){
	  ajouter_transition(automate_melange,nommer_etat(i,j), get_element(it_alph2) , nommer_etat(get_element(it_access), j));
	}				   
      }

      if(est_dans_l_ensemble(initiaux_1,i) && est_dans_l_ensemble(initiaux_2,j))
	ajouter_element(initiaux_melange, nommer_etat(i,j));
      if(est_dans_l_ensemble(finaux_1,i) && est_dans_l_ensemble(finaux_2,j))
	ajouter_element(finaux_melange, nommer_etat(i,j));
    }

  }
  automate_melange->initiaux = initiaux_melange;
  automate_melange->finaux = finaux_melange;
  return automate_melange;
}




  
