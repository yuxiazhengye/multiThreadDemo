#include "threadPool.h"
#include <iostream>

std::mutex g_mtx;

ThreadPool::ThreadPool(int _num):threadNum(_num), isRunning(true)
{
	for (int i = 0; i < threadNum; i++)
	{	
		{
			std::unique_lock<std::mutex> lck2(wrMtx);
			std::cout << "创建第" << i+1 << "个线程" << std::endl;
		}
		workThreads.emplace_back(ThreadPool::worker, this);
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lck(mtx);
		isRunning = false;
	}
	cv.notify_all();
	for (auto& work : workThreads)
	{
		work.join();
	}
}

void* ThreadPool::worker(void* arg)
{
	ThreadPool* pool = (ThreadPool*)arg;
	pool->run();
	return pool;
}

void ThreadPool::run()
{
	while (isRunning)
	{
		Task task = NULL;
		{
			std::unique_lock<std::mutex> lck(this->mtx);
			this->cv.wait(lck, [this] {return !this->tasks.empty(); });
			if (this->tasks.empty())
			{
				continue;
			}
			else
			{
				task = tasks.front();
				tasks.pop_front();
				{
					std::unique_lock<std::mutex> lck2(wrMtx);
					std::cout << "ThreadPool::work() tid: " << std::this_thread::get_id() << " start.\n";
				}
			}
		}
		if (NULL != task)
			task();
	}
}

void ThreadPool::addTask(const Task& task)
{
	std::unique_lock<std::mutex> lck(mtx);
	tasks.push_back(task);
	cv.notify_one();
}

struct Test {
	void process_no_static_bind(const int i, const int j)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::unique_lock<std::mutex> lck(g_mtx);
		std::cout << "bind: i==" << i << " " << "j== " << j << std::endl;
	}
};

void threadPoolTest()
{
	ThreadPool pool(6);
	Test ttBind;
	for (int i = 0; i < 15; i++)
	{
		pool.addTask(std::bind(&Test::process_no_static_bind, &ttBind, i, i+1));
	}
}