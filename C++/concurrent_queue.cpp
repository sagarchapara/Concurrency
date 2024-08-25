#include "stdcpp.h"

template<typename T>
class concurrent_queue {
	struct node
	{
		T val;
		node* next;
		node(const T& data) : val(data), next(nullptr) {}
	};

	std::atomic<node*> head, tail;

public:

	void push(T val)
	{
		node* new_node = new node(val);

		// put the current value of head into new_node->next
		node* old_tail = tail.load();

		// then put that new head into new_node->next and try again
		while (!tail.compare_exchange_weak(old_tail, new_node)) {
			old_tail = tail.load();
		}

		old_tail->next = new_node;
	}

	bool pop(T& val) {
		node* old_head = head.load();
		
		node* next = old_head->next;

		while (old_head && !head.compare_exchange_weak(old_head, next)) {
			old_head = head.load();
		}

		if (old_head) {
			val =old_head->value;
			return true;
		}

		return false;
	}
};