#include "common.h"
#include "JobManager.h"

JobManager::JobManager(Polar *engine) : System(engine) {
	for(int i = 0; i < numWorkers; ++i) {
		_workers.push_back(new Worker());
	}
}

JobManager::~JobManager() {
	for(auto worker : _workers) {
		delete worker;
	}
}

void JobManager::Init() {
	for(auto worker : _workers) {
		worker->Start();
	}
}

void JobManager::Update(DeltaTicks &, std::vector<Object *> &) {
	auto numJobs = jobs.With<JobsType::size_type>([] (JobsType &jobs) { return jobs.size(); });
	if(numJobs == 0) {
		std::this_thread::yield();
		return;
	}

	auto numOnMain = (std::max)(128U, numJobs / 64);

	std::vector<Job> todo;

	for(; numJobs > 0; --numJobs) {
		auto job = jobs.With<Job>([] (JobsType &jobs) {
			auto job = jobs.top();
			jobs.pop();
			return job;
		});
		switch(job.thread) {
		case JobThread::Main:
			if(numOnMain > 0) {
				job.fn();
				numOnMain--;
			} else { todo.emplace_back(job); }
			break;
		case JobThread::Any:
			if(numOnMain > 0) {
				job.fn();
				numOnMain--;
				break;
			}
		case JobThread::Worker:
			if(nextWorker >= _workers.size()) { nextWorker = 0; }
			_workers.at(nextWorker++)->AddJob(job);
			break;
		}
	}

	jobs.With([&todo] (JobsType &jobs) {
		for(auto &job : todo) { jobs.emplace(job); }
	});
}

void JobManager::Destroy() {
	for(auto worker : _workers) {
		worker->AddJob(Job(JobType::Stop));
	}
	for(auto worker : _workers) {
		worker->Join();
	}
}
