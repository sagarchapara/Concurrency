#include "stdcpp.h"
#include "semaphore.cpp"

using namespace std;

class dining_philosophers {
	counting_semaphore<4> max_philosophers{ 4 };
	vector<unique_ptr<binary_semaphore>> forks;

public:
	dining_philosophers(){
		for (int i = 0; i < 5; i++) {
			unique_ptr<binary_semaphore> ptr = make_unique<binary_semaphore>(1);

			forks.push_back(move(ptr));
		}
	}

	void get_forks(int i) {
		max_philosophers.acquire();
		forks[i]->acquire();
		forks[(i + 1) % 5]->acquire();
	}

	void put_forks(int i) {
		forks[i]->release();
		forks[(i + 1) % 5]->release();
		max_philosophers.release();
	}
};