#pragma once

#include "System.h"
#include "Job.h"
#include "Worker.h"

typedef std::priority_queue<Job> JobsType;

class JobManager : public System {
private:
	Atomic<JobsType> jobs;
	std::vector<Worker *> _workers;
	std::vector<Worker *>::size_type nextWorker = 0;
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
	void Destroy() override final;
public:
	const int numWorkers = (std::max)(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
	static bool IsSupported() { return true; }
	JobManager(Polar *);
	~JobManager() override;
	inline void Do(const JobFunction &fn, const JobPriority priority = JobPriority::Normal, const JobThread thread = JobThread::Any) {
		jobs.With([&fn, priority, thread] (JobsType &jobs) {
			jobs.emplace(fn, priority, thread);
		});
	}
};
