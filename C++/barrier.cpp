#include "stdcpp.h"

using namespace std;


/// <summary>
/// Barrier is like await Task.WhenAll() in C#, which blocks the execution, until all are completed
/// This code is for reusable barrier
/// </summary>
class barrier {
	int capacity;
	int counter;
	int removed;

	mutex mtx;
	condition_variable cv;

public:

	barrier(int n): capacity(n), counter(0), removed(0) {}

	void aquire() {
		unique_lock<mutex> lock(mtx);

		//first wait till counter is < capacity, so that we don't allow no new threads
		cv.wait(lock, [&] {return counter < capacity; });

		counter++;

		//if we got counter == capacity, we got all the threads, so wake them up
		if (counter == capacity) {
			removed = 0;
			cv.notify_all();
		}
		else {
			cv.wait(lock, [&] {return counter == capacity; });
		}

		removed++;

		//If all threads are done, allow new threads
		if (removed == capacity) {
			counter = 0;
			removed = 0;
			cv.notify_all();
		}
	}
};

/// <summary>
/// Reusable Barrier using binary sempahore and tunrstile like in the book
/// </summary>
class barrier2 {
	binary_semaphore turnstile1{ 0 }; //blocks until we get n threads
	binary_semaphore turnstile2{ 1 }; //blocks until all the n threads leave

	mutex mtx;
	int counter;
	int capacity;

public:

	barrier2(int n): capacity(n), counter(0){}

	void acquire() {
		turnstile2.acquire();
		turnstile2.release();

		{
			lock_guard<mutex> lock(mtx);
			counter++;

			if (counter == capacity) {
				turnstile2.acquire();
				turnstile1.release();
			}
		}

		turnstile1.acquire();
		turnstile1.release();

		{
			lock_guard<mutex> lock(mtx);
			counter--;

			if (counter == 0) {
				turnstile1.acquire();
				turnstile2.release();
			}
		}
	}
};