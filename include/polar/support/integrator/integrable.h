#pragma once

#include <memory>
#include <optional>
#include <boost/circular_buffer.hpp>
#include <polar/core/deltaticks.h>

namespace polar::support::integrator {
	template<typename T> inline T integrable_id() {
		return T(0);
	}

	template<> inline Quat integrable_id() {
		return Quat{1, 0, 0, 0};
	}

	template<typename T, typename D = T> inline T integrable_sum(T a, D b) {
		return a + b;
	}

	template<> inline Quat integrable_sum(Quat a, Quat b) {
		return glm::normalize(b * a);
	}

	template<> inline Quat integrable_sum(Quat a, Point3 b) {
		return glm::normalize(Quat(b) * a);
	}

	template<typename T> inline T integrable_interp(T value, T target, Decimal alpha) {
		return glm::mix(value, target, alpha);
	}

	template<> inline Quat integrable_interp(Quat value, Quat target, Decimal alpha) {
		return glm::normalize(glm::slerp(value, target, alpha));
	}

	enum class target_type {
		ease_towards
	};

	template<typename T> struct target_t {
		target_type type;
		T value;
		Decimal factor;
	};

	class integrable_base {
	  public:
		virtual ~integrable_base() {}
		virtual bool hasderivative(const unsigned char = 0)             = 0;
		virtual integrable_base &getderivative(const unsigned char = 0) = 0;
		virtual void integrate(const DeltaTicks::seconds_type)          = 0;
		virtual bool revert_by(size_t = 0)                              = 0;
		virtual bool revert_to(size_t = 0)                              = 0;
	};

	template<typename T, class D = T> class integrable : public integrable_base {
	  private:
		using derivative_t = std::unique_ptr<integrable<D>>;

		struct history_entry {
			T value;
			std::optional<target_t<T>> target;
		};

		boost::circular_buffer<history_entry> history;

		T value;
		derivative_t deriv;
		std::optional<target_t<T>> _target;

	  public:
		template<typename... Ts>
		integrable(Ts &&... args) : value(std::forward<Ts>(args)...) {
			history = boost::circular_buffer<history_entry>(100, {value, _target});
		}

		inline void target(T value, Decimal factor) {
			_target = target_t<T>{target_type::ease_towards, value, factor};
		}

		inline std::optional<target_t<T>> gettarget() const {
			return _target;
		}

		inline operator const T &() const { return get(); }

		inline const T &get() const { return value; }
		inline const T &get_previous() const { return history.back().value; }
		template<typename _To> inline _To to() {
			return static_cast<_To>(value);
		}
		template<typename _To> inline _To to_previous() {
			return static_cast<_To>(history.back().value);
		}

		inline bool hasderivative(const unsigned char n = 0) override {
			if(!deriv) {
				return false;
			} else if(n == 0) {
				return true;
			} else {
				return derivative().hasderivative(n - 1);
			}
		}

		inline integrable_base &
		getderivative(const unsigned char n = 0) override {
			if(n == 0) {
				if(!deriv) { deriv = derivative_t(new integrable<D>()); }
				return *deriv;
			} else {
				return derivative().derivative(n - 1);
			}
		}

		inline integrable<D> &derivative(const unsigned char n = 0) {
			if(n == 0) {
				if(!deriv) { deriv = derivative_t(new integrable<D>()); }
				return *deriv;
			} else {
				return derivative().derivative(n - 1);
			}
		}

		inline integrable<T> temporal(const Decimal seconds) {
			if(hasderivative()) {
				D delta = integrable_interp<D>(integrable_id<D>(), *derivative().temporal(seconds), seconds);
				integrable<T> ret(integrable_sum(value, delta));
				if(auto t = _target) {
					ret.target(t->value, t->factor);
				}
				return ret;
			} else {
				return *this;
			}
		}

		inline void integrate(const DeltaTicks::seconds_type seconds) override {
			history.push_back({value, _target});

			if(hasderivative()) {
				D delta = integrable_interp<D>(integrable_id<D>(), *derivative(), seconds);
				value = integrable_sum(value, delta);

				// if second-order derivative exists, integrate polynomially
				if(hasderivative(1)) {
					D delta = integrable_interp<D>(integrable_id<D>(), *derivative(1), seconds * seconds / Decimal(2));
					value = integrable_sum(value, delta);
				}

				derivative().integrate(seconds);
			}

			if(_target) {
				switch(_target->type) {
				case target_type::ease_towards:
					value = integrable_interp(value, _target->value, _target->factor);
					break;
				}
			}
		}

		inline bool revert_by(size_t n = 1) override {
			if(n == 0) { return true; }

			auto size = history.size();
			if(n >= size) {
				return false;
			} else {
				auto &entry = history[size - n - 1];
				value   = entry.value;
				_target = entry.target;
				return hasderivative() ? derivative().revert_by(n) : true;
			}
		}

		inline bool revert_to(size_t n = 0) override {
			if(n >= history.size()) {
				return false;
			} else {
				auto &entry = history[n];
				value   = entry.value;
				_target = entry.target;
				return hasderivative() ? derivative().revert_to(n) : true;
			}
		}

		inline T &operator*() { return value; }
		inline T *operator->() { return &value; }

		inline integrable<T> &operator=(const T &rhs) {
			value = rhs;
			return *this;
		}

		inline integrable<T> &operator-() { return -value; }

		inline integrable<T> &operator+=(const DeltaTicks::seconds_type rhs) {
			value += rhs;
			return *this;
		}
		inline integrable<T> &operator+=(const T &rhs) {
			value += rhs;
			return *this;
		}
		inline integrable<T> &operator+=(const integrable<T> &rhs) {
			return *this += rhs.value;
		}
		inline friend integrable operator+(integrable lhs, const T &rhs) {
			return lhs += rhs;
		}
		inline friend integrable operator+(integrable lhs,
		                                   const integrable &rhs) {
			return lhs += rhs;
		}

		inline integrable<T> &operator-=(const DeltaTicks::seconds_type rhs) {
			value -= rhs;
			return *this;
		}
		inline integrable<T> &operator-=(const T &rhs) {
			value -= rhs;
			return *this;
		}
		inline integrable<T> &operator-=(const integrable<T> &rhs) {
			return *this -= rhs.value;
		}
		inline friend integrable operator-(integrable lhs, const T &rhs) {
			return lhs -= rhs;
		}
		inline friend integrable operator-(integrable lhs,
		                                   const integrable &rhs) {
			return lhs -= rhs;
		}

		inline integrable<T> &operator*=(const DeltaTicks::seconds_type rhs) {
			value *= rhs;
			return *this;
		}
		inline integrable<T> &operator*=(const T &rhs) {
			value *= rhs;
			return *this;
		}
		inline integrable<T> &operator*=(const integrable<T> &rhs) {
			return *this *= rhs.value;
		}
		inline friend integrable operator*(integrable lhs, const T &rhs) {
			return lhs *= rhs;
		}
		inline friend integrable operator*(integrable lhs,
		                                   const integrable &rhs) {
			return lhs *= rhs;
		}
	};
} // namespace polar::support::integrator
