#pragma once

#include <polar/support/work/job.h>
#include <polar/util/atomic.h>
#include <queue>
#include <thread>

namespace polar::support::work {
	class worker {
		using job_t       = support::work::job;
		using job_queue_t = support::work::job_queue_t;

	  private:
		std::thread _thread;
		atomic<job_queue_t> jobs;

	  public:
		void start();
		bool join();
		inline void addjob(job_t &&job) {
			jobs.with(
			    [&job](job_queue_t &jobs) { jobs.emplace(std::move(job)); });
			jobs.notify();
		}
	};
} // namespace polar::support::work
