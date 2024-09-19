#include "stealThreadPool.h"

namespace Nano {
	namespace Concurrency {
		StealThreadPool::StealThreadPool(int maxFailCount, int maxBackOffTime, unsigned int poolSize) :
			m_done(false), m_joiner(m_threads), m_atm_index(0),
			m_maxFailCount(maxFailCount),
			m_maxBackOffTimeMilliSeconds(maxBackOffTime),
			m_fail_count(poolSize, 0),
			m_backoff_time_ms(poolSize, 1)
		{
			try
			{
				m_thread_work_ques = std::vector <Concurrency::ConcurrentQueue<FunctionWrapper>>(poolSize);

				for (unsigned i = 0; i < poolSize; ++i)
				{
					m_threads.push_back(std::thread(&StealThreadPool::worker_thread, this, i));
				}
			}
			catch (...)
			{
				m_done = true;
				for (int i = 0; i < m_thread_work_ques.size(); i++) {
					m_thread_work_ques[i].Exit();
				}
				throw;
			}
		}

		void StealThreadPool::worker_thread(int index)
		{
			m_fail_count[index] = 0;
			m_backoff_time_ms[index] = 1;
			while (!m_done)
			{
				FunctionWrapper wrapper;
				if (m_thread_work_ques[index].try_pop(wrapper) || try_steal_from_others(index, wrapper)) {
					// 成功获取任务后，重置退避时间, 重置失败次数
					m_fail_count[index] = 0;
					m_backoff_time_ms[index] = 1;
					wrapper();
				}
				else {
					// 增加失败次数
					++m_fail_count[index];

					// 每隔一定失败次数再打印日志
					if (m_fail_count[index] % 10 == 0) {
						ASYNC_LOG_DEBUG(ASYNC_LOG_NAME("STD_LOGGER"), "StealThreadPool")
							<< "worker_thread " << index << " 未获取到任务，失败次数: " << m_fail_count[index] << std::endl;
					}

					if (m_fail_count[index] > m_maxFailCount) {
						// 随机指数退避时间，避免所有线程同时休眠相同时间
						m_backoff_time_ms[index] = std::min(m_backoff_time_ms[index] * (1 + rand() % 2), m_maxBackOffTimeMilliSeconds);
						// 休眠一段时间
						std::this_thread::sleep_for(std::chrono::milliseconds(m_backoff_time_ms[index]));
					}
					else {
						ASYNC_LOG_DEBUG(ASYNC_LOG_NAME("STD_LOGGER"), "StealThreadPool")
							<< "worker_thread " << index << " 未获取到任务，线程让出CPU: " << m_fail_count[index] << std::endl;
						// 未获取到任务，线程让出CPU
						std::this_thread::yield();
					}
				}
			}

			//m_fail_count[index] = 0;
			//m_backoff_time_ms[index] = 1;

			//while (!m_done)
			//{
			//	FunctionWrapper wrapper;

			//	{
			//		std::unique_lock<std::mutex> lock(m_queue_mutex);
			//		// 如果任务队列为空，则等待任务到来
			//		m_condition.wait(lock, [this, index, &wrapper]() {
			//			return m_done || !m_thread_work_ques[index].empty() || try_steal_from_others(index, wrapper);
			//		});
			//		// 如果线程池已终止，退出
			//		if (m_done) return;
			//		// 从队列中获取任务
			//		if (!m_thread_work_ques[index].try_pop(wrapper) && !try_steal_from_others(index, wrapper)) {
			//			ASYNC_LOG_DEBUG(ASYNC_LOG_NAME("STD_LOGGER"), "StealThreadPool") << "worker_thread " << index << std::endl;
			//			continue; // 如果没成功获取任务，继续等待
			//		}
			//	}

			//	// 执行任务
			//	ASYNC_LOG_DEBUG(ASYNC_LOG_NAME("STD_LOGGER"), "StealThreadPool") << "worker_thread " << index<< "获取到任务" << std::endl;
			//	m_fail_count[index] = 0;
			//	m_backoff_time_ms[index] = 1;
			//	wrapper();
			//}

			//while (!m_done)
			//{
			//	FunctionWrapper wrapper;
			//	bool pop_res = m_thread_work_ques[index].try_pop(wrapper);
			//	if (pop_res) {
			//		wrapper();
			//		continue;
			//	}

			//	bool steal_res = false;
			//	for (int i = 0; i < m_thread_work_ques.size(); i++) {
			//		if (i == index) {
			//			continue;
			//		}
			//		steal_res = m_thread_work_ques[i].try_pop(wrapper);
			//		if (steal_res) {
			//			wrapper();
			//			break;
			//		}
			//	}
			//	if (steal_res) {
			//		continue;
			//	}
			//	std::this_thread::yield();
			//}

		}

		StealThreadPool::~StealThreadPool()
		{
			m_done = true;
			for (unsigned i = 0; i < m_thread_work_ques.size(); i++) {
				m_thread_work_ques[i].Exit();
			}

			for (unsigned i = 0; i < m_threads.size(); ++i)
			{
				m_threads[i].join();
			}
		}

		bool StealThreadPool::try_steal_from_others(int selfIndex, FunctionWrapper& wrapper)
		{
			for (int i = 0; i < m_thread_work_ques.size(); ++i) {
				if (i != selfIndex && m_thread_work_ques[i].try_steal(wrapper)) {
					return true;
				}
			}
			return false;
		}
	}
}