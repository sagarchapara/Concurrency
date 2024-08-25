#include "kvsnapshot.cpp"
#include "stdcpp.h"

using namespace std;

int main() {

	kv_snapshot_map<int, long> map;

	vector<thread> threads;

	auto start = std::chrono::system_clock::now();

	atomic<int> num_snapshots = 0;

	for (int i = 0; i < 12; i++) {
		threads.emplace_back([&] {
			for (int i = 0; i < 1000000; i++) {
				map.put(rand(), rand());

				if(num_snapshots.load() < 0 && rand() % 100000 == 50){
					num_snapshots.fetch_add(1);

					map.take_snapshot();
				}
			}
		});
	}

	map.take_snapshot();

	for (auto& t : threads) {
		t.join();
	}

	auto finish = std::chrono::system_clock::now();

	auto duration = chrono::duration_cast<chrono::milliseconds>(finish - start);

	cout << duration << endl;

	return 0;
}