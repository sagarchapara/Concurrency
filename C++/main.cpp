#include "future.cpp"

int main() {

	future_thread_pool t(100);

	vector<unique_ptr<future<string>>> futures;

	for (int i = 0; i < 1000; i++) {

		auto unqptr = make_unique<future<string>>(t.execute([i] {
			//cout << sqrt(i) << endl;

			cout << this_thread::get_id() << endl;

			this_thread::sleep_for(chrono::milliseconds(1000));

			string ans = "Sagar is GOAT " + to_string(i);

			return ans;
		}));

		futures.push_back(move(unqptr));
	}

	for (auto& f : futures) {
		cout << f->get() << endl;
	}

	this_thread::sleep_for(chrono::seconds(20));

	return 0;
}