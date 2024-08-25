#include "stdcpp.h"

using namespace std;


/// <summary>
/// Some buggy code, get() isn't working.... :(
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
class Future;

template <typename T>
class Promise {
	Future<T> future;


	std::mutex mtx;
	std::condition_variable cv;

	T result;
	std::atomic<bool> done = false;
	std::exception_ptr ex;

	friend class Future<T>;

public:

	Promise(): future(this){}

	Future<T> get_future() {
		return future;
	}

	void set_result(const T& res) {
		{
			std::unique_lock<std::mutex> lock(this->mtx);

			done = true;
			result = res;
		}
		
		this->cv.notify_all();
	}

	void set_exception(std::exception_ptr exception) {
		{
			std::unique_lock<std::mutex> lock(this->mtx);

			done = true;
			ex = exception;
		}

		this->cv.notify_all();
	}
};

template <typename T>
class Future {
	Promise<T>* promise;

public:
	Future(Promise<T>* p): promise(p){}

	T get() {
		std::unique_lock<std::mutex> lock(promise->mtx);

		promise->cv.wait(lock, [this] {return promise->done == true; });

		if (promise->ex) {
			std::rethrow_exception(promise->ex);
		}

		return promise->result;
	}


	bool is_done(){
		return promise->done;
	}
};

/// <summary>
/// Future thread pool, using custom promise and future classes
/// </summary>
class FutureThreadPool {
	queue<function<void()>> q;

	mutex mtx;
	condition_variable cv;
	atomic<bool> is_running = true;

	vector<thread> threads;
	int num_threads;

	void start() {
		for (int i = 0; i < num_threads; i++) {
			threads.emplace_back([&] {
				while (is_running) {
					function<void()> f;

					{
						unique_lock<mutex> lock(mtx);

						cv.wait(lock, [&] {return q.size() > 0 || !is_running; });

						if (!is_running) return;

						f = move(q.front()); q.pop();
					}

					assert(f != nullptr);

					try {
						f();
					}
					catch (...) {

					}
				}
			});
		}
	}

	void stop() {
		is_running.store(false);

		cv.notify_all();

		for (auto& t : threads) {
			if (t.joinable()) {
				t.join();
			}
		}
	}

public:

	FutureThreadPool(int n) : num_threads(n) {
		start();
	}

	~FutureThreadPool() {
		stop();
	}

	Future<string> execute(function<string()> task) {
		using return_type = string;

		auto p = std::make_shared<Promise<return_type>>();

		auto future = p->get_future();

		auto lambda = [task, p] {
			try {
				p->set_result(task());
			}
			catch (...) {
				p->set_exception(std::current_exception());
			}
		};
		
		{
			unique_lock<mutex> lock(mtx);

			q.emplace(move(lambda));
		}

		cv.notify_all();

		return future;
	}
};