#pragma once

#include <vector>
#include <deque>
#include <thread>
#include <functional>
#include <mutex>
#include <memory>
#include <condition_variable>

struct ThreadPool {
	using Task = std::function<void()>;
	ThreadPool(int _num = 3);
	~ThreadPool();
	void addTask(const Task& task);

private:
	ThreadPool(const ThreadPool& pool);
	ThreadPool& operator=(const ThreadPool& pool);
	void run();
	/*�����߳���Ҫ���еĺ���,���ϵĴ����������ȡ����ִ��*/
	static void *worker(void *arg);
private:
	//std::vector<std::thread*> threads;
	//std::vector<std::shared_ptr<std::thread>> threads;
	std::vector<std::thread> workThreads;
	std::deque<Task> tasks;
	int threadNum;
	std::mutex mtx;
	std::mutex wrMtx;
	std::condition_variable cv;
	bool isRunning;
};

void threadPoolTest();

