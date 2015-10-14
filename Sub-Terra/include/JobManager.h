#pragma once

#include <boost/container/vector.hpp>
#include "System.h"
#include "Job.h"
#include "Worker.h"

typedef std::priority_queue<Job> JobsType;

class JobManager : public System {
private:
	static Atomic<bool> exists;
	Atomic<JobsType> jobs;
	boost::container::vector<Worker *> _workers;
	boost::container::vector<Worker *>::size_type nextWorker = 0;
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
public:
	const int numWorkers = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
	static bool IsSupported() { return true; }
	JobManager(Polar *);
	~JobManager() override;
	inline void Do(const JobFunction &&fn, const JobPriority &&priority = JobPriority::Normal, const JobThread &&thread = JobThread::Any) {
		auto e = exists.With<bool>([] (bool &exists) { return exists; });
		if(e == false) { return; }

		jobs.With([&fn, priority, thread] (JobsType &jobs) {
			jobs.emplace(std::move(fn), std::move(priority), std::move(thread));
		});
	}
};
