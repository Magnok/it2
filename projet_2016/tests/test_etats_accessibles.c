#include "automate.h"
#include "outils.h"
#include "ensemble.h"

int test_etats_accessibles(){
	int result = 1;

	{
		Automate * automate = creer_automate();

		ajouter_transition( automate, 1, 'a', 1 );
		ajouter_transition( automate, 1, 'c', 42 );
		ajouter_transition( automate, 2, 'c', 42 );
		ajouter_transition( automate, 42, 'c', 2 );
		ajouter_transition( automate, 3, 'd', 4 );
		ajouter_transition( automate, 4, 'd', 3 );
		ajouter_etat_initial( automate, 1);
		ajouter_etat_final( automate, 2);

		Ensemble * expected = creer_ensemble(NULL,NULL,NULL);
		ajouter_element(expected,1);
		ajouter_element(expected,2);
		ajouter_element(expected,42);
	        Ensemble * ens = etats_accessibles(automate, 1);
		if(comparer_ensemble(expected,ens) != 0)
		  result = 0;
		liberer_ensemble(expected);
		liberer_ensemble(ens);
		liberer_automate( automate );
	}

	return result;
}


int main(){

	if( ! test_etats_accessibles() ){ return 1; };

	return 0;
	
}
