#include "stdcpp.h"

using namespace std;

class ordered_lock {
	queue<condition_variable*> q;

	mutex q_lock;

	bool is_locked;

public:

	/// <summary>
	/// Have a conditional variable per entry in queue and signal it to remove, when it's done
	/// For the first time don't add anything to queue.
	/// https://stackoverflow.com/questions/14792016/creating-a-lock-that-preserves-the-order-of-locking-attempts-in-c11
	/// </summary>
	void lock() {
		unique_lock<mutex> lock(q_lock);

		if (is_locked) {
			condition_variable cv;

			q.emplace(&cv);

			cv.wait(lock);

			q.pop();
		}
		else {
			is_locked = true;
		}
	}

	void unlock() {
		unique_lock<mutex> lock(q_lock);

		if (q.empty()) {
			is_locked = false;
		}
		else {
			q.front()->notify_one();
		}
	}

};