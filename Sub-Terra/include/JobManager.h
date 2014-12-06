#pragma once

#include "System.h"
#include "Job.h"
#include "Worker.h"

class JobManager : public System {
private:
	std::priority_queue<Job *> _jobs;
	std::vector<Worker *> _workers;
	std::vector<Worker *>::size_type nextWorker = 0;
protected:
	void Init() override final;
	void Update(DeltaTicks &, std::vector<Object *> &) override final;
	void Destroy() override final;
public:
	const int numWorkers = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
	static bool IsSupported() { return true; }
	JobManager(Polar *);
	~JobManager() override;
	void Do(const JobFunction &fn, const JobPriority = JobPriority::Normal, const JobThread = JobThread::Any);
};
