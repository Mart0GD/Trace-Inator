#ifndef __THREAD_POOL_HPP_INCLUDED__

#include <cstdint>
#include <thread>
#include <queue>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>

/*
*   Simple implementation of Thread pool for executing parallel tasks  
*/
class thread_pool{

    //  ## Non-Copyable ##
    thread_pool(thread_pool&)             = delete;
    thread_pool& operator=(thread_pool&)  = delete;

    thread_pool(thread_pool&&)            = delete;
    thread_pool& operator=(thread_pool&&) = delete;

public:

    /*
    *   Constructor which initializes a desired number of threads
    */
    thread_pool(std::size_t threads = std::thread::hardware_concurrency());

    /*
    *   Destructor which waits for all threads to finish workload before exiting 
    */
    ~thread_pool() noexcept;

    /*
    *    Adds another task to the back of the current task queue 
    */
    bool enqueue(std::function<void()> task);

    inline std::size_t workers_count() const { return workers.size(); }

private:
    void thread_loop();

    std::queue<std::function<void()>>   tasks;      // tasks for execution 
    std::vector<std::thread>            workers;    // aquired threads

    bool                                finished;   // tells us when to stop
    std::mutex                          q_mutex;    // mutex for execution and enqueueing 
    std::condition_variable             notifier;   // task notifier                 
};

#endif