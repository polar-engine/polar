#pragma once

#include <functional>

enum class JobType { Work, Stop };
enum class JobPriority { Low, Normal, High };
enum class JobThread { Main, Worker, Any };

typedef std::function<void()> JobFunction;

class Job {
public:
	JobType type;
	JobPriority priority;
	JobThread thread;
	JobFunction fn;

	Job(const JobFunction &fn, const JobPriority priority = JobPriority::Normal, const JobThread thread = JobThread::Any)
		: type(JobType::Work), priority(priority), thread(thread), fn(fn) {}
	Job(const JobType type, const JobPriority priority = JobPriority::Normal, const JobThread thread = JobThread::Any)
		: type(type), priority(priority), thread(thread) {}
	bool operator<(const Job &rhs) const { return priority < rhs.priority; }
};
