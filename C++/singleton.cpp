#include <mutex>

using namespace std;

template <typename T>
class singleton {
	static mutex lock;

	static volatile T obj;
	static volatile bool is_initialized;

	singleton() {}

	T get() {
		if (!is_initialized) {
			lock_guard<mutex> lock(mutex);

			if (!is_initialized) {
				obj = T();
			}
		}

		return obj;
	}
};