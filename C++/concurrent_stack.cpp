#include "stdcpp.h"

template<typename T>
class concurrent_stack {
	struct node
	{
		T val;
		node* next;
		node(const T& data) : val(data), next(nullptr) {}
	};

	std::atomic<node*> head;

public:

	void push(T val)
	{
		node* new_node = new node(val);

		// put the current value of head into new_node->next
		new_node->next = head.load();

		// then put that new head into new_node->next and try again
		while (!head.compare_exchange_weak(new_node->next, new_node));
	}

	bool pop(T& val) {
		node* oldHead = head.load(); // Load the current head

		while (oldHead && !head.compare_exchange_weak(oldHead, oldHead->next)); // Attempt to update head

		if (oldHead) {
			val = oldHead->val;

			return true;
		}

		return false;
	}
};