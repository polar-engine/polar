#pragma once

#include <functional>

enum class JobPriority { Low, Normal, High };
enum class JobType { Work, Stop };

class Worker;
typedef std::function<void()> JobFunction;

class Job {
public:
	JobPriority priority;
	JobType type;
	JobFunction fn;
	Job(JobFunction fn, JobPriority const priority = JobPriority::Normal) : priority(priority), type(JobType::Work), fn(fn) {}
	Job(JobPriority const priority = JobPriority::Normal) : priority(priority), type(JobType::Stop) {}
	bool operator<(Job const &rhs) const { return priority < rhs.priority; }
};
