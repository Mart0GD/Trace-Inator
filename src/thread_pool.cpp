#include "thread_pool.hpp"

thread_pool::thread_pool(std::size_t threads)
    : finished(false)
{
    for (std::size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] {thread_loop();});
    }
}

thread_pool::~thread_pool() noexcept
{
    {   // flip stop condition  
        std::unique_lock<std::mutex> lock(q_mutex);
        finished = true;
    }

    // Awake all threads and wait for them to finish...

    notifier.notify_all();
    for(std::thread& th : workers)
    {
        th.join();
    }
}

bool thread_pool::enqueue(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(q_mutex);
        if(finished) return false;

        tasks.push(std::move(task));
    }
    notifier.notify_one();
    return true;
}

void thread_pool::thread_loop()
{
    while (true) {
        std::function<void()> current_task;

        {
            // lock the flow
            std::unique_lock<std::mutex> lock(q_mutex);

            // wait for a task or finish condition
            notifier.wait(lock, [this] { return finished || !tasks.empty(); });

            // exit 
            if(finished && tasks.empty()) break;

            current_task = std::move(tasks.front());
            tasks.pop();
        }   // mutex unlocks on the end of scope 

        current_task(); // execute task 
    }
}