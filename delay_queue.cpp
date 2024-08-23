#pragma once

#include "stdcpp.h"

using namespace std;
using namespace std::chrono;

class delay_queue {
	struct task {
		function<void()> f;
		time_point<system_clock> execution_time;

		bool operator<(const task& other) const {
			return execution_time > other.execution_time;
		}
	};

	priority_queue<task> tasks;
	mutex mtx;
	condition_variable cv;

public:
	void push(function<void()> f, time_point<system_clock> time) {
		bool should_notify = false;
		
		{
			unique_lock<mutex> lock(mtx);

			tasks.emplace(f, time);

			if (tasks.top().execution_time == time) {
				should_notify = true;
			}
		}

		if (should_notify) {
			cv.notify_one();
		}
	}

	function<void()> poll() {
		while (true) {
			unique_lock<mutex> lock(mtx);

			if (tasks.size() == 0) {
				cv.wait(lock);
			}
			else if(tasks.top().execution_time > system_clock::now()){
				cv.wait_until(lock, tasks.top().execution_time);
			}
			else {
				auto p = tasks.top();
				tasks.pop();

				return p.f;
			}
		}
	}
};