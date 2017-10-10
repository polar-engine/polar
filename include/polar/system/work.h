#pragma once

#include <polar/support/work/job.h>
#include <polar/support/work/worker.h>
#include <polar/system/base.h>
#include <vector>

namespace polar {
namespace system {
	class work : public base {
		using worker_t     = support::work::worker;
		using job_t        = support::work::job;
		using job_queue_t  = support::work::job_queue_t;
		using job_fn       = support::work::job_fn;
		using job_priority = support::work::job_priority;
		using job_thread   = support::work::job_thread;
		using job_type     = support::work::job_type;

	  private:
		atomic<job_queue_t> jobs;
		std::vector<worker_t *> _workers;
		std::vector<worker_t *>::size_type nextWorker = 0;

	  protected:
		void init() override final;
		void update(DeltaTicks &) override final;

	  public:
		const int numWorkers = std::max(
		    1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
		static bool supported() { return true; }

		work(core::polar *);
		~work() override;

		inline void do_job(const job_fn &&fn,
		                   const job_priority &&priority = job_priority::normal,
		                   const job_thread &&thread     = job_thread::any) {
			jobs.with([&fn, priority, thread](job_queue_t &jobs) {
				jobs.emplace(std::move(fn), std::move(priority),
				             std::move(thread));
			});
		}
	};
}
}
