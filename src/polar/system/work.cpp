#include <polar/system/work.h>

namespace polar::system {
	work::work(core::polar *engine) : base(engine) {
		for(int i = 0; i < numWorkers; ++i) {
			_workers.push_back(new worker_t());
		}
	}

	work::~work() {
		for(auto worker : _workers) { worker->addjob(job_t(job_type::stop)); }

		/* copy vector to avoid invalidation */
		auto tmpWorkers = _workers;
		for(auto worker : tmpWorkers) {
			worker->join();
			delete worker;
		}

		debugmanager()->verbose("all workers joined and destructed");
	}

	void work::init() {
		for(auto worker : _workers) { worker->start(); }
	}

	void work::update(DeltaTicks &) {
		jobs.with([this](job_queue_t &jobs) {
			auto numJobs   = jobs.size();
			auto numOnMain = std::max(static_cast<uint64_t>(128),
			                          static_cast<uint64_t>(numJobs / 64));

			for(; numJobs > 0; --numJobs) {
				auto job = jobs.top();
				jobs.pop();
				switch(job.thread) {
				case job_thread::main:
					if(numOnMain > 0) {
						job.fn();
						numOnMain--;
					} else {
						jobs.emplace(job);
					}
					break;
				case job_thread::any:
					if(numOnMain > 0) {
						job.fn();
						numOnMain--;
						break;
					}
				case job_thread::worker:
					if(nextWorker >= _workers.size()) { nextWorker = 0; }
					_workers.at(nextWorker++)->addjob(std::move(job));
					break;
				}
			}
		});
	}
}
