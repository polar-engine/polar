#include "JobManager.h"

JobManager::JobManager() {
	for (int i = 0; i < numWorkers; ++i) {
		_workers.push_back(new Worker());
	}
}

JobManager::~JobManager() {
	for (auto worker : _workers) {
		delete worker;
	}

	while (!_jobs.empty()) {
		auto job = _jobs.top();
		_jobs.pop();
		delete job;
	}
}

void JobManager::Init() {
	for (auto worker : _workers) {
		worker->Start();
	}
}

void JobManager::Update(int) {
	if (_jobs.empty()) {
		std::this_thread::yield();
	} else {
		auto job = _jobs.top();
		_jobs.pop();
		switch (job->thread) {
		case JobThread::Main:
			job->fn();
			break;
		case JobThread::Any:
			if (_jobs.empty()) { job->fn(); }
			else { _workers.front()->AddJob(job); }
			break;
		case JobThread::Worker:
			_workers.back()->AddJob(job);
			break;
		}
	}
}

void JobManager::Destroy() {
	for (auto worker : _workers) {
		worker->AddJob(new Job(JobType::Stop));
	}
	for (auto worker : _workers) {
		worker->Join();
	}
}

void JobManager::Do(const JobFunction &fn, const JobPriority priority, const JobThread thread) {
	_jobs.push(new Job(fn, priority, thread));
}
