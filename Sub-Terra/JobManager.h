#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include "System.h"
#include "Job.h"
#include "Worker.h"

class JobManager : public System {
private:
	std::priority_queue<Job *> _jobs;
	std::vector<Worker *> _workers = std::vector<Worker *>(numWorkers);
public:
	const int numWorkers = std::thread::hardware_concurrency() - 1;
	static bool IsSupported() { return true; }
	~JobManager() override;
	void Init() override final;
	void Update(int) override final;
	void Destroy() override final;
	void Do(JobFunction const &fn, JobPriority const = JobPriority::Normal);
};
