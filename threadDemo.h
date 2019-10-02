#pragma once
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <condition_variable>
#include <iostream>

struct CThreadDemo
{
	CThreadDemo()
	{
		m_data.clear();
		m_nGen = 0;
	}

	void start()
	{
		std::vector<std::thread> vecThre;
		vecThre.clear();
		for (int i = 0; i < 5; i++)
		{
			vecThre.push_back(std::thread(&CThreadDemo::produceThread, this));
		}
		for (int i = 0; i < 5; i++)
		{
			vecThre.push_back(std::thread(&CThreadDemo::consumerThread, this));
		}
		for (auto& thre : vecThre)
		{
			thre.join();
		}
	}

private:
	void produceThread()
	{
		for (int i = 0; i < 100; i++)
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_nGen = ++m_nGen % 1000;
			std::cout << "produce: " << m_nGen << endl;
			m_data.push_back(m_nGen);
			lck.unlock();
			m_cv.notify_all();

			/* µÈ´ý1Ãë */
			std::chrono::milliseconds dura(1000);
			std::this_thread::sleep_for(dura);
		}
	}

	void consumerThread()
	{ 
		for (int i = 0; i < 100; i++)
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			while (m_data.empty())
			{
				m_cv.wait(lck);
			}
			int data = m_data.front();
			m_data.pop_front();
			std::cout << "consumer: " << data << endl;

			/* µÈ´ý2Ãë */
			std::chrono::milliseconds dura(2000);
			std::this_thread::sleep_for(dura);
		}
	}

private:
	std::deque<int> m_data;
	std::mutex m_mtx;
	std::condition_variable m_cv;
	int m_nGen;
};

void threadDemoTest()
{
	CThreadDemo demo;
	demo.start();
}
