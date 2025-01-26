#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>

namespace dsp56k
{
	class Semaphore
	{
	public:
		explicit Semaphore (const uint32_t _count = 0) : m_count(_count) 
	    {
	    }
	    
	    void notify(const uint32_t _count = 1)
		{
			{
		        Lock lock(m_mutex);
		        m_count += _count;
			}

			//notify the waiting thread
	        m_cv.notify_one();
	    }

	    void wait(const uint32_t _count = 1)
		{
	        Lock lock(m_mutex);

            // wait on the mutex until notify is called
			m_cv.wait(lock, [&]()
			{
				return m_count >= _count;
			});

			m_count -= _count;
	    }
	private:
		using Lock = std::unique_lock<std::mutex>;
	    std::mutex m_mutex;
	    std::condition_variable m_cv;
	    uint32_t m_count;
	};

	class SpscSemaphore
	{
	public:
		explicit SpscSemaphore(const int _count = 0) : m_count(_count)
		{
		}

		void notify()
		{
			const int prev = m_count.fetch_add(1, std::memory_order_release);
	        if (prev < 0)
	            m_sem.notify();
		}

		void wait()
		{
			const auto count = m_count.fetch_sub(1, std::memory_order_acquire);
			if (count < 1)
				m_sem.wait();
		}
	private:
		std::atomic<int> m_count;
		Semaphore m_sem;
	};
	
	class NopSemaphore
	{
	public:
		explicit NopSemaphore (const uint32_t _count = 0)	{}
	    void notify(uint32_t _count = 1)					{}
		void wait(uint32_t _count = 1)						{}
	};
};
