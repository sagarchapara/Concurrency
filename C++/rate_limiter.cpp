#pragma once

#include "stdcpp.h"

using namespace std;
using namespace std::chrono;

class rate_limiter
{
	int tokens, max_tokens;
	mutex mtx;
	condition_variable cv;
	thread worker_thread;
	bool is_running;

public:
	rate_limiter(int _max_tokens): max_tokens(_max_tokens), tokens(_max_tokens), is_running(true) {
		worker_thread = thread([&] {
			while (is_running) {
				bool should_notify = false;

				{
					lock_guard<mutex> lock(mtx);

					if (tokens < max_tokens) {
						tokens++;
						should_notify = true;
					}
				}

				if (should_notify) {
					cv.notify_one();
				}

				//sleep
				this_thread::sleep_for(seconds(1));
			}
		});
	}

	~rate_limiter() {
		{
			lock_guard<mutex> lock(mtx);

			is_running = false;
		}

		cv.notify_all();

		if (worker_thread.joinable()) {
			worker_thread.join();
		}
	}

	void acquire() {

		{
			unique_lock<mutex> lock(mtx);

			cv.wait(lock, [&] {return tokens > 0; });

			tokens--;
		}
	}

};

