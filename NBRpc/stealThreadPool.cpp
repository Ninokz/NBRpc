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
					// �ɹ���ȡ����������˱�ʱ��, ����ʧ�ܴ���
					m_fail_count[index] = 0;
					m_backoff_time_ms[index] = 1;
					wrapper();
				}
				else {
					// ����ʧ�ܴ���
					++m_fail_count[index];

					// ÿ��һ��ʧ�ܴ����ٴ�ӡ��־
					if (m_fail_count[index] % 10 == 0) {
						ASYNC_LOG_DEBUG(ASYNC_LOG_NAME("STD_LOGGER"), "StealThreadPool")
							<< "worker_thread " << index << " δ��ȡ������ʧ�ܴ���: " << m_fail_count[index] << std::endl;
					}

					if (m_fail_count[index] > m_maxFailCount) {
						// ���ָ���˱�ʱ�䣬���������߳�ͬʱ������ͬʱ��
						m_backoff_time_ms[index] = std::min(m_backoff_time_ms[index] * (1 + rand() % 2), m_maxBackOffTimeMilliSeconds);
						// ����һ��ʱ��
						std::this_thread::sleep_for(std::chrono::milliseconds(m_backoff_time_ms[index]));
					}
					else {
						ASYNC_LOG_DEBUG(ASYNC_LOG_NAME("STD_LOGGER"), "StealThreadPool")
							<< "worker_thread " << index << " δ��ȡ�������߳��ó�CPU: " << m_fail_count[index] << std::endl;
						// δ��ȡ�������߳��ó�CPU
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
			//		// ����������Ϊ�գ���ȴ�������
			//		m_condition.wait(lock, [this, index, &wrapper]() {
			//			return m_done || !m_thread_work_ques[index].empty() || try_steal_from_others(index, wrapper);
			//		});
			//		// ����̳߳�����ֹ���˳�
			//		if (m_done) return;
			//		// �Ӷ����л�ȡ����
			//		if (!m_thread_work_ques[index].try_pop(wrapper) && !try_steal_from_others(index, wrapper)) {
			//			ASYNC_LOG_DEBUG(ASYNC_LOG_NAME("STD_LOGGER"), "StealThreadPool") << "worker_thread " << index << std::endl;
			//			continue; // ���û�ɹ���ȡ���񣬼����ȴ�
			//		}
			//	}

			//	// ִ������
			//	ASYNC_LOG_DEBUG(ASYNC_LOG_NAME("STD_LOGGER"), "StealThreadPool") << "worker_thread " << index<< "��ȡ������" << std::endl;
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