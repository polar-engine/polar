#include <polar/core/log.h>
#include <polar/support/work/worker.h>

namespace polar::support::work {
		void worker::start() {
			auto fn = [this]() {
				while(true) {
					if(jobs.with<bool>(
					       [](job_queue_t &jobs) { return jobs.empty(); })) {
						std::this_thread::sleep_for(
						    std::chrono::milliseconds(5));
					} else {
						auto job = jobs.with<job_t>([](job_queue_t &jobs) {
							auto job = jobs.top();
							jobs.pop();
							return job;
						});
						switch(job.type) {
						case job_type::work:
							job.fn();
							break;
						case job_type::stop:
							log()->verbose("worker received stop command");
							return;
						}
					}
				}
			};
			_thread = std::thread(fn);
		}

		bool worker::join() {
			bool joinable = _thread.joinable();
			if(joinable) { _thread.join(); }
			return joinable;
		}
}
