#include "stdcpp.h"

using namespace std;

/// <summary>
/// Allows multiple people in a room, used for readers
/// </summary>
class light_switch {
	int counter = 0;
	mutex mtx;

public:
	void lock(binary_semaphore& semaphore) {
		lock_guard<mutex> lock(mtx);

		counter++;

		if (counter == 1) {
			semaphore.acquire();
		}
	}

	void unlock(binary_semaphore& semaphore) {
		lock_guard<mutex> lock(mtx);

		counter--;

		if (counter == 0) {
			semaphore.release();
		}
	}
};