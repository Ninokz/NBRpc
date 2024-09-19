#pragma once
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <functional>

#include "log.h"
#include "nocopyable.h"
#include "singleton.h"
#include "concurrentqueue.h"
#include "functionwrapper.h"
#include "jointhreads.h"

namespace Nano {
	namespace Concurrency {
		class StealThreadPool : public Singleton<StealThreadPool>, public Noncopyable
		{
			friend class Singleton<StealThreadPool>;
		public:
			static const int MAX_FAIL_COUNT = 20;
			static const int MAX_BACK_OFF_TIME = 1024;
		private:
			StealThreadPool(int maxFailCount = MAX_FAIL_COUNT, int maxBackOffTime = MAX_BACK_OFF_TIME, unsigned int poolSize = std::thread::hardware_concurrency());
			void worker_thread(int index);
			bool try_steal_from_others(int selfIndex, FunctionWrapper& wrapper);
		public:
			~StealThreadPool();

			template<typename FunctionType>
			std::future<typename std::result_of<FunctionType()>::type>
				submit(FunctionType f)
			{
				int index = (static_cast<unsigned long long>(m_atm_index.load()) + 1) % m_thread_work_ques.size();
				m_atm_index.store(index);
				typedef typename std::result_of<FunctionType()>::type result_type;
				std::packaged_task<result_type()> task(std::move(f));
				std::future<result_type> res(task.get_future());

				{
					std::lock_guard<std::mutex> lock(m_queue_mutex);
					m_thread_work_ques[index].push(std::move(task));
				}
				m_condition.notify_one();

				return res;
			}
		private:
			std::atomic_bool m_done;
			std::vector<Concurrency::ConcurrentQueue<FunctionWrapper>> m_thread_work_ques;
			std::vector<std::thread> m_threads;
			std::vector<int> m_fail_count;
			std::vector<int> m_backoff_time_ms;
			JoinThreads m_joiner;
			std::atomic<int>  m_atm_index;

			const int m_maxFailCount;
			const int m_maxBackOffTimeMilliSeconds;
		};
	}
}