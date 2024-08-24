#include "stdcpp.h"

using namespace std;

class semaphore {
	int count;
	mutex mtx;
	condition_variable cv;
	int capacity;

public:

	semaphore(int init_count, int size) : count(init_count), capacity(size){}

	void acquire() {
		{
			unique_lock<mutex> lock(mtx);

			cv.wait(lock, [&] {return count > 0; });

			count--;
		}

		//we can use notify_one, however notify_one will not work in single producer, multiple consumers problems
		//i.e, a release thread can notify another release thread, not a acquire thread, which can block a acquire thread indefinetely
		cv.notify_all();
	}

	void release() {
		{
			unique_lock<mutex> lock(mtx);

			cv.wait(lock, [&] {return count < capacity; });

			count++;
		}

		cv.notify_all();
	}
};