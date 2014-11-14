#include "Worker.h"
#include <iostream>

void Worker::Start() {
	auto fn = [this]() {
		std::unique_lock<std::mutex> lock(jobsLock, std::defer_lock);
		while (true) {
			lock.lock();
			/* we get a debug assertion failure if we try to get the top() of an empty vector */
			if (jobs.empty()) {
				lock.unlock();
				std::this_thread::yield();
			} else {
				auto job = jobs.top();
				jobs.pop();
				lock.unlock();
				switch (job->type) {
				case JobType::Work:
					job->fn();
					break;
				case JobType::Stop:
					return;
				}
				delete job;
			}
		}
	};
	_thread = std::thread(fn);
}

bool Worker::Join() {
	bool joinable = _thread.joinable();
	if (joinable) { _thread.join(); }
	return joinable;
}

void Worker::AddJob(Job *job) {
	std::unique_lock<std::mutex> lock(jobsLock, std::defer_lock);
	lock.lock();
	jobs.push(job);
	lock.unlock();
}
