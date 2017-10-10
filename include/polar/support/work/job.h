#pragma once

#include <functional>
#include <queue>

namespace polar {
namespace support {
	namespace work {
		enum class job_type { work, stop };
		enum class job_priority { low, normal, high };
		enum class job_thread { main, worker, any };

		using job_fn = std::function<void()>;

		class job {
		  public:
			job_type type;
			job_priority priority;
			job_thread thread;
			job_fn fn;

			job(const job_fn &&fn,
			    const job_priority &&priority = job_priority::normal,
			    const job_thread &&thread     = job_thread::any)
			    : type(job_type::work), priority(priority), thread(thread),
			      fn(fn) {}
			job(const job_type &&type,
			    const job_priority &&priority = job_priority::normal,
			    const job_thread &&thread     = job_thread::any)
			    : type(type), priority(priority), thread(thread) {}
			bool operator<(const job &rhs) const {
				return priority < rhs.priority;
			}
		};

		using job_queue_t = std::priority_queue<job>;
	}
}
}
