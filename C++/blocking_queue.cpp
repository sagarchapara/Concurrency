#include "stdcpp.h"

using namespace std;

class blocking_queue {
	queue<function<void()>> q;

	mutex mtx;
	condition_variable cv;
	atomic<bool> should_wait = true;

public:

	void clear() {
		should_wait.store(false);

		cv.notify_all();
	}

	void push(function<void()>& f) {
		{
			unique_lock<mutex> lock(mtx);

			q.push(f);
		}

		cv.notify_all();
	}

	function<void()> pop() {
		{
			unique_lock<mutex> lock(mtx);

			cv.wait(lock, [&] {return q.size() > 0 || !should_wait; });

			if (!should_wait) return nullptr;

			auto f = q.front(); q.pop();

			return f;
		}
	}
};