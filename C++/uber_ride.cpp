#include "stdcpp.h"
#include "barrier.cpp"

using namespace std;

class uber_ride {
	int democrats;
	int republicants;
	mutex mtx;

	counting_semaphore<1000> d, r;

	void drive() { cout << "Driving" << endl; }
	void seat() { cout << "Seated" << endl; }

	barrier b;

public:

	uber_ride() : b{ 4 }, democrats(0), republicants(0), d{ 0 }, r{ 0 } {}

	void get_ride(bool democrat) {
		bool is_leader = true;

		mtx.lock();

		if (democrat) {
			if (republicants >= 2 && democrats == 1) {
				republicants -= 2;
				democrats -= 1;

				d.release(1);
				r.release(2);
			}
			else if (democrats >= 3) {
				democrats -= 3;
				d.release(3);
			}
			else {
				democrats++;
				is_leader = false;
				mtx.unlock();
				d.acquire();
			}
		}
		else {
			if (republicants == 1 && democrats >= 2) {
				democrats -= 2;
				republicants -= 1;
				d.release(2);
				r.release(1);
			}
			else if (republicants >= 3) {
				republicants -= 3;
				r.release(3);
			}
			else {
				is_leader = false;
				republicants++;
				mtx.unlock();
				r.acquire();
			}
		}

		seat();

		b.aquire();

		if (is_leader) {
			drive();
			mtx.unlock();
		}
	}
};