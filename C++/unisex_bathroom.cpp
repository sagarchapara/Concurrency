#include "stdcpp.h"
#include "light_switch.h"

using namespace std;

class unisex_bathroom {
	binary_semaphore empty;
	binary_semaphore turnstile;
	light_switch men;
	light_switch women;
	counting_semaphore<3> nMen;
	counting_semaphore<3> nWomen;

	void try_men() {
		turnstile.acquire();
		men.lock(empty);
		turnstile.release();
		nMen.acquire();

		//use bathroom

		nMen.release();
		men.unlock(empty);
	}

	void try_women() {
		turnstile.acquire();
		women.lock(empty);
		turnstile.release();
		nWomen.acquire();

		//use bathroom

		nWomen.release();
		women.unlock(empty);
	}

};