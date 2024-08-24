#include "stdcpp.h"
#include "light_switch.cpp"

using namespace std;

/// <summary>
/// Writers priority read write lock
/// </summary>
class read_write_lock {
	light_switch readSwitch;
	light_switch writeSwitch;
	binary_semaphore no_readers;
	binary_semaphore no_writers;

public:
	void read() {
		//We need to make sure, write lock is not taken, so check if no_readers is available
		no_readers.acquire();

		//lock the no_writers lock
		readSwitch.lock(no_writers);

		//We allow multiple readers, so no need to keep it during reading
		no_readers.release();

		//read

		//release the no_writers
		readSwitch.unlock(no_writers);
	}

	void write() {

		//ensure there are no readers
		writeSwitch.lock(no_readers);

		//ensure no other one is writing
		no_writers.acquire();

		//write

		no_writers.release();

		writeSwitch.unlock(no_readers);
	}
};

