#include "stdcpp.h"

using namespace std;

/// <summary>
/// future threadpool using std::future
/// </summary>
class future_thread_pool {
public:
    future_thread_pool(size_t n) {
        start(n);
    }

    ~future_thread_pool() {
        stop();
    }

    template<class T>
    std::future<typename std::invoke_result<T>::type> execute(T task){
        using return_type = typename std::invoke_result<T>::type;

        auto wrapper = make_shared<packaged_task<return_type()>>(std::move(task));

        {
            lock_guard<mutex> lock(mtx);

            if (!is_running) {
                throw std::runtime_error("Queue is being aborted ....");
            }

            q.emplace([=] {
                (*wrapper)();
            });
        }

        cv.notify_one();

        return wrapper->get_future();
    }



private:
    queue<function<void()>> q;

    mutex mtx;
    condition_variable cv;
    volatile bool is_running = true;

    vector<thread> threads;


    void start(size_t n) {
        for (int i = 0; i < n; i++) {
            threads.emplace_back([=] {
                while (is_running) {
                    function<void()> task;

                    {
                        unique_lock<mutex> lock(mtx);

                        cv.wait(lock, [&] {return q.size() > 0 || !is_running; });

                        if (!is_running) return;

                        task = std::move(q.front()); q.pop();
                    }

                    try {
                        //run the task
                        task();
                    }
                    catch (...) {

                    }
                }
            });
        }
    }

    void stop() {
        {
            lock_guard<mutex> lock(mtx);

            is_running = false;
        }

        cv.notify_all();

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
};
