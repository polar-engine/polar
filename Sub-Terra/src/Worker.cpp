#include "common.h"
#include "Worker.h"

void Worker::Start() {
	auto fn = [this] () {
		std::unique_lock<std::mutex> lock(jobsLock, std::defer_lock);
		while(true) {
			lock.lock();
			/* we get a debug assertion failure if we try to get the top() of an empty vector */
			if(jobs.empty()) {
				lock.unlock();
				std::this_thread::yield();
			} else {
				auto job = jobs.top();
				jobs.pop();
				lock.unlock();
				switch(job.type) {
				case JobType::Work:
					job.fn();
					break;
				case JobType::Stop:
					return;
				}
			}
		}
	};
	_thread = std::thread(fn);
}

bool Worker::Join() {
	bool joinable = _thread.joinable();
	if(joinable) { _thread.join(); }
	return joinable;
}

void Worker::AddJob(Job job) {
	std::lock_guard<std::mutex> lock(jobsLock);
	jobs.emplace(job);
}
