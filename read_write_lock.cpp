#include "stdcpp.h"
#include "light_switch.h"

using namespace std;

class read_write_lock {
	light_switch readSwitch;
	light_switch writeSwitch;
	binary_semaphore no_readers;
	binary_semaphore no_writers;

public:
	void read() {
		no_readers.acquire();

		readSwitch.lock(no_writers);

		no_readers.release();

		//read

		readSwitch.lock(no_writers);
	}

	void write() {
		writeSwitch.lock(no_readers);

		no_writers.acquire();

		//write

		no_writers.release();

		writeSwitch.unlock(no_readers);
	}
};

