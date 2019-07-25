#pragma once

#include <memory>
#include <optional>
#include <polar/core/deltaticks.h>

namespace polar::support::integrator {
	inline Point2 ease_towards(Point2 value, Point2 target, Decimal factor) {
		return glm::mix(value, target, factor);
	}

	inline Point3 ease_towards(Point3 value, Point3 target, Decimal factor) {
		return glm::mix(value, target, factor);
	}

	inline Point4 ease_towards(Point4 value, Point4 target, Decimal factor) {
		return glm::mix(value, target, factor);
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
		virtual void revert()                                           = 0;
	};

	template<typename T> class integrable : public integrable_base {
	  private:
		typedef std::unique_ptr<integrable<T>> derivative_t;
		T previousValue;
		T value;
		derivative_t deriv;
		std::optional<target_t<T>> _target;

	  public:
		template<typename... Ts>
		integrable(Ts &&... args) : value(std::forward<Ts>(args)...) {
			previousValue = value;
		}

		inline void target(T value, Decimal factor) {
			_target = target_t<T>{target_type::ease_towards, value, factor};
		}

		inline operator const T &() const { return get(); }

		inline const T &get() const { return value; }
		inline const T &getprevious() const { return previousValue; }
		template<typename _To> inline _To to() {
			return static_cast<_To>(value);
		}
		template<typename _To> inline _To to_previous() {
			return static_cast<_To>(previousValue);
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
				if(!deriv) { deriv = derivative_t(new integrable<T>()); }
				return *deriv;
			} else {
				return derivative().derivative(n - 1);
			}
		}

		inline integrable<T> &derivative(const unsigned char n = 0) {
			if(n == 0) {
				if(!deriv) { deriv = derivative_t(new integrable<T>()); }
				return *deriv;
			} else {
				return derivative().derivative(n - 1);
			}
		}

		inline integrable<T> temporal(const DeltaTicks::seconds_type alpha) {
			T newValue(value);
			if(hasderivative()) { newValue += derivative().value; }
			return integrable<T>(newValue * alpha + value * (1 - alpha));
		}

		inline void integrate(const DeltaTicks::seconds_type seconds) override {
			if(hasderivative()) {
				previousValue = value;

				/* if second-order derivative exists, integrate polynomially
				 */
				if(hasderivative(1)) {
					value += derivative().value * seconds +
					         derivative().derivative().value * seconds *
					             seconds / Decimal(2);
				} else {
					value += derivative().value * seconds;
				}
				derivative().integrate(seconds);
			}

			if(_target) {
				switch(_target->type) {
				case target_type::ease_towards:
					value = ease_towards(value, _target->value, _target->factor);
					break;
				}
			}
		}

		inline void revert() override { value = previousValue; }

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
		inline friend integrable<T> operator+(integrable<T> lhs, const T &rhs) {
			return lhs += rhs;
		}
		inline friend integrable<T> operator+(integrable<T> lhs,
		                                      const integrable<T> &rhs) {
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
		inline friend integrable<T> operator-(integrable<T> lhs, const T &rhs) {
			return lhs -= rhs;
		}
		inline friend integrable<T> operator-(integrable<T> lhs,
		                                      const integrable<T> &rhs) {
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
		inline friend integrable<T> operator*(integrable<T> lhs, const T &rhs) {
			return lhs *= rhs;
		}
		inline friend integrable<T> operator*(integrable<T> lhs,
		                                      const integrable<T> &rhs) {
			return lhs *= rhs;
		}
	};
} // namespace polar::support::integrator
