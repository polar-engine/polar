#pragma once

/* Windows.h and stuff */
#ifdef AddJob
#undef AddJob
#endif

#include "Job.h"

typedef std::priority_queue<Job> JobsType;

class Worker {
private:
	std::thread _thread;
	Atomic<JobsType> jobs;
public:
	void Start();
	bool Join();
	void AddJob(Job);
};
