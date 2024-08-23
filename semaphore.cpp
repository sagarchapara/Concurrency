#include "stdcpp.h"

using namespace std;

class semaphore {
	int count;
	mutex mtx;
	condition_variable cv;
	int capacity;

public:

	semaphore(int init_count, int size) : count(init_count), capacity(size){}

	void wait() {
		{
			unique_lock<mutex> lock(mtx);

			cv.wait(lock, [&] {return count > 0; });

			count--;
		}
	}

	void signal() {
		{
			unique_lock<mutex> lock(mtx);

			cv.wait(lock, [&] {return count < capacity; });

			count++;
		}

		cv.notify_one();
	}
};