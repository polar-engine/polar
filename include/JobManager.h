#pragma once

#include "System.h"
#include "Job.h"
#include "Worker.h"

class JobManager : public System {
private:
	std::priority_queue<Job *> _jobs;
	std::vector<Worker *> _workers;
public:
	const int numWorkers = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
	static bool IsSupported() { return true; }
	JobManager();
	~JobManager() override;
	void Init() override final;
	void Update(DeltaTicks &, std::vector<Object *> &) override final;
	void Destroy() override final;
	void Do(const JobFunction &fn, const JobPriority = JobPriority::Normal, const JobThread = JobThread::Any);
};
