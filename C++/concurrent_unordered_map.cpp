#include "stdcpp.h"

using namespace std;

template <typename T, typename V>
class concurrent_unordered_map {
	struct kv_shard{
		unordered_map<T, V> map;
		shared_mutex m;
	};

	static const int num_buckets = 1024;

	kv_shard buckets[num_buckets];

	static int get_shard_key(const T& key) {
		std::hash<T> hasher;
		size_t hash_value = hasher(key);
		return hash_value % num_buckets;
	}

public:

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

			buckets[shard_key].map[key] = value;
		}
	}

	void remove(const T& key) {
		int shard_key = get_shard_key(key);

		{
			lock_guard<shared_mutex> lock(buckets[shard_key].m);

			buckets[shard_key].map.erase(key);
		}
	}
};