#pragma once

#include "stdcpp.h"


using namespace std;
using namespace std::chrono;

class RateLimiter
{
	int tokens;
	mutex mtx;
	condition_variable cv;
	int max_tokens;
	time_point<system_clock> last_access_time;
	bool is_running;
	thread worker_thread;

public:

	RateLimiter(int _max_tokens) : max_tokens(_max_tokens), tokens(_max_tokens), is_running(true) {
	
		worker_thread = thread ([this] {
			while (is_running) {
				{
					unique_lock<mutex> lock(mtx);

					if (tokens < max_tokens) {
						tokens++;
					}
				}

				cv.notify_one();

				std::this_thread::sleep_for(seconds(1));
			}
		});

		worker_thread.detach();
	}

	~RateLimiter() {
		{
			unique_lock<mutex> lock(mtx);
			is_running = false;
		}

		cv.notify_all();

		if (worker_thread.joinable()) {
			worker_thread.join();
		}
	}

	void getToken();
};

