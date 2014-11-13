#include "JobManager.h"
#include <iostream>

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
	for (int i = 0; i < numWorkers; ++i) {
		_workers.push_back(new Worker());
		_workers.back()->Start();
	}
}

void JobManager::Update(int) {
	if (_jobs.empty() || _workers.empty()) {
		std::this_thread::yield();
	} else {
		auto &job = _jobs.top();
		_jobs.pop();
		Worker *w = _workers.front();
		std::unique_lock<std::mutex> lock(w->jobsLock, std::defer_lock);
		lock.lock();
		w->jobs.push(job);
		lock.unlock();
	}
}

void JobManager::Destroy() {
	for (auto worker : _workers) {
		worker->Stop();
	}
	for (auto worker : _workers) {
		worker->Join();
	}
}

void JobManager::Do(JobFunction const &fn, JobPriority const priority) {
	_jobs.push(new Job(fn, priority));
}
