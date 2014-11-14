#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include "Job.h"

class Worker {
private:
	std::thread _thread;
public:
	std::mutex jobsLock;
	std::priority_queue<Job *> jobs;

	void Start();
	bool Join();
	void AddJob(Job *);
};
