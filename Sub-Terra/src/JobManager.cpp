#include "common.h"
#include "JobManager.h"

JobManager::JobManager(Polar *engine) : System(engine) {
	for(int i = 0; i < numWorkers; ++i) {
		_workers.push_back(new Worker());
	}
}

JobManager::~JobManager() {
	for(auto worker : _workers) {
		worker->AddJob(Job(JobType::Stop));
	}

	/* copy vector to avoid invalidation */
	auto tmpWorkers = _workers;
	for(auto worker : tmpWorkers) {
		worker->Join();
		delete worker;
	}

	INFO("all workers joined and destructed");
}

void JobManager::Init() {
	for(auto worker : _workers) {
		worker->Start();
	}
}

void JobManager::Update(DeltaTicks &) {
	jobs.With([this] (JobsType &jobs) {
		auto numJobs = jobs.size();
		auto numOnMain = std::max(static_cast<uint64_t>(128), static_cast<uint64_t>(numJobs / 64));

		for(; numJobs > 0; --numJobs) {
			auto job = jobs.top();
			jobs.pop();
			switch(job.thread) {
			case JobThread::Main:
				if(numOnMain > 0) {
					job.fn();
					numOnMain--;
				} else { jobs.emplace(job); }
				break;
			case JobThread::Any:
				if(numOnMain > 0) {
					job.fn();
					numOnMain--;
					break;
				}
			case JobThread::Worker:
				if(nextWorker >= _workers.size()) { nextWorker = 0; }
				_workers.at(nextWorker++)->AddJob(std::move(job));
				break;
			}
		}
	});
}
