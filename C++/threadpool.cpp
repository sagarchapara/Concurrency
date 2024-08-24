#include "stdcpp.h"
#include "blocking_queue.cpp"

using namespace std;

class thread_pool {
	int num_workers;
	vector<thread> workers;
	blocking_queue q;
	std::atomic<bool> is_running{ true };

public:
	thread_pool(int n): num_workers(n) {
		workers.resize(n);

		for (int i = 0; i < n; i++) {
			workers[i] = thread([&] {
				while (is_running) {
					auto f = q.pop();

					if (!is_running) return;

					try {
						f();
					}
					catch (...) {

					}
				}
			});
		}
	}

	~thread_pool() {
		is_running.store(false);

		q.clear();

		for (int i = 0; i < num_workers; i++) {
			if (workers[i].joinable()) {
				workers[i].join();
			}
		}
	}

	void execute(function<void()> f) {
		q.push(f);
	}
};