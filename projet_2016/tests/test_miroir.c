
#include "automate.h"
#include "outils.h"

int test_miroir(){
	int result = 1;

	{
		Automate * automate = creer_automate();

		ajouter_transition( automate, 1, 'a', 1 );
		ajouter_transition( automate, 1, 'b', 2 );
		ajouter_etat_initial( automate, 1);
		ajouter_etat_final( automate, 2);
		print_automate(automate);
		Automate * aut = miroir( automate );
		print_automate(aut);
		liberer_automate( aut );
		liberer_automate( automate );
	}

	return result;
}


int main(){

	if( ! test_miroir() ){ return 1; };

	return 0;
	
}
