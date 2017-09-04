#pragma once

#ifdef AddJob
#undef AddJob
#endif

#include <queue>
#include <thread>
#include <polar/support/work/job.h>
#include <polar/util/atomic.h>

typedef std::priority_queue<Job> JobsType;

class Worker {
private:
	std::thread _thread;
	Atomic<JobsType> jobs;
public:
	void Start();
	bool Join();
	inline void AddJob(Job &&job) {
		jobs.With([&job] (JobsType &jobs) {
			jobs.emplace(std::move(job));
		});
		jobs.Notify();
	}
};
