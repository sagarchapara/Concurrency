#include "stdcpp.h"


template <typename T>
class future {
	T result;
	bool has_result = false;
	std::exception_ptr ex;

	std::mutex mtx;
	std::condition_variable cv;

	void set_result(const T& res) {
		{
			std::unique_lock<std::mutex> lock(mtx);

			result = std::move(res);

			has_result = true;
		}

		cv.notify_all();
	}

	void set_exception(std::exception_ptr exception) {
		{
			std::unique_lock<std::mutex> lock(mtx);

			ex = std::move(exception);

			has_result = true;
		}

		cv.notify_all();
	}

public:

	T get() {
		{
			std::unique_lock<std::mutex> lock(mtx);

			cv.wait(lock, return { has_result == true; });

			if (ex) {
				std::rethrow_exception(ex);
			}
			else {
				return result;
			}
		}
	}

	bool done() {
		return has_result;
	}
};

template <typename T>
class promise {
	shared_ptr<future<T>> f;
	std::function<T()> task;

public:
	promise(function<T()>& t) : task(t) {
		f = make_shared<future<T>>();
	}

	void execute() {
		try {
			T res = task();
			f->set_result(res);
		}
		catch (...) {
			f
		}
	}

	future<T> get_future() {
		return *f;
	}
};




class promise_future_threadpool {

};