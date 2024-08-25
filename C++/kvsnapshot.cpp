#include "stdcpp.h"

using namespace std;

template <typename T, typename V>
class kv_snapshot_map {
	struct kv_shard {
		unordered_map<T, V> map;
		shared_mutex m;
	};

	atomic<uint16_t> version = 0;

	static const int num_buckets = 1024;

	kv_shard buckets[num_buckets];

	atomic<shared_ptr<kv_shard[]>> prev_snapshot;
	atomic<shared_ptr<kv_shard[]>> curr_snapshot;


	static int get_shard_key(const T& key) {
		std::hash<T> hasher;
		size_t hash_value = hasher(key);
		return hash_value % num_buckets;
	}

	//snapshot mutex
	mutex snap_mutex;

public:

	kv_snapshot_map() {
		prev_snapshot.store(make_shared<kv_shard[]>(num_buckets));
		curr_snapshot.store(make_shared<kv_shard[]>(num_buckets));
	}

	bool get(const T& key, V& value) {
		int shard_key = get_shard_key(key);

		{
			shared_lock<shared_mutex>  lock(buckets[shard_key].m);

			auto itr = buckets[shard_key].map.find(key);

			if (itr != buckets[shard_key].map.end()) {
				value = itr->second;

				return true;
			}

			return false;
		}
	}

	void put(const T& key, V value) {
		int shard_key = get_shard_key(key);

		{
			lock_guard<shared_mutex> lock(buckets[shard_key].m);

			version.fetch_add(1);

			buckets[shard_key].map[key] = value;
			//curr_snapshot.load().get()[shard_key].map[key] = value;
		}
	}

	void remove(const T& key) {
		int shard_key = get_shard_key(key);

		{
			lock_guard<shared_mutex> lock(buckets[shard_key].m);

			version.fetch_add(1);

			buckets[shard_key].map.erase(key);
			//curr_snapshot[shard_key].map.erase(key);
		}
	}

	void take_snapshot() {
		{
			lock_guard<mutex> lock(snap_mutex);

			shared_ptr<kv_shard[]> new_snapshot = make_shared<kv_shard[]>(num_buckets);

			auto old = curr_snapshot.exchange(new_snapshot);

			//once we have old, lets merge with prev one
			for (int i = 0; i < num_buckets;i++) {

				auto& mp = prev_snapshot.load().get()[i].map;

				auto& old_mp = old.get()[i].map;

				for (auto& [k, v] : old_mp) {
					mp[k] = v;
				}
			}
		}
	}
};