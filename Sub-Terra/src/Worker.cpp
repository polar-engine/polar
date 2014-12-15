#include "common.h"
#include "Worker.h"

void Worker::Start() {
	auto fn = [this] () {
		while(true) {
			if(jobs.With<bool>([] (JobsType &jobs) { return jobs.empty(); })) {
				std::this_thread::yield();
			} else {
				auto job = jobs.With<Job>([] (JobsType &jobs) {
					auto job = jobs.top();
					jobs.pop();
					return job;
				});
				switch(job.type) {
				case JobType::Work:
					job.fn();
					break;
				case JobType::Stop:
					INFO("worker received stop command");
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
	jobs.With([&job] (JobsType &jobs) {
		jobs.emplace(job);
	});
}
