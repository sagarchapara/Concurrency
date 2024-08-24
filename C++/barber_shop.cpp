#include "stdcpp.h"
#include "semaphore.cpp"

using namespace std;

class barber_shop {
	int capacity, count, cutting_num;
	mutex mtx;

	semaphore customer_to_walk_in{0,1};
	semaphore baber_to_get_ready{0,1};
	semaphore baber_to_cut_hair {0,1};
	semaphore customer_to_leave{0,1};

	thread barber_thread;

public:
	barber_shop(int n) : capacity(n), count(0), cutting_num(0){
		barber_thread = thread([&] {
			cout << this_thread::get_id() << endl;

			while (true) {

				customer_to_walk_in.acquire();

				cout << "Customer ready .." << endl;

				baber_to_get_ready.release();

				cout << "Cutting hair ....." << cutting_num << endl;

				cutting_num++;

				this_thread::sleep_for(chrono::milliseconds(1));

				baber_to_cut_hair.release();

				cout << "Done Cutting ..." << cutting_num << endl;

				customer_to_leave.acquire();

				cout << "Customer left" << cutting_num << endl;
			}

			cout << "Barber Thread exited" << endl;
		});
	}

	void get_cutting(int i) {
	
		{
			lock_guard<mutex> lock(mtx);

			if (count == capacity) return;

			count++;
		}

		customer_to_walk_in.release();

		cout << "Waiting for barber .." << i << endl;


		baber_to_get_ready.acquire();

		{
			lock_guard<mutex> lock(mtx);

			count--;
		}

		cout << "Getting cutting .." << i << endl;

		baber_to_cut_hair.acquire();

		cout << "Leaving the room .." << i << endl;

		customer_to_leave.release();
	}
};