#pragma once

#include <memory>
#include <polar/core/deltaticks.h>

namespace polar::support::integrator {
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

	  public:
		template<typename... Ts>
		integrable(Ts &&... args) : value(std::forward<Ts>(args)...) {
			previousValue = value;
		}

		inline T &get() { return value; }
		inline T &getprevious() { return previousValue; }
		template<typename _To> inline _To to() {
			return static_cast<_To>(value);
		}
		template<typename _To> inline _To to_previous() {
			return static_cast<_To>(previousValue);
		}

		inline bool hasderivative(const unsigned char n = 0) override final {
			if(!deriv) {
				return false;
			} else if(n == 0) {
				return true;
			} else {
				return derivative().hasderivative(n - 1);
			}
		}

		inline integrable_base &
		getderivative(const unsigned char n = 0) override final {
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

		inline void
		integrate(const DeltaTicks::seconds_type seconds) override final {
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
		}

		inline void revert() override final { value = previousValue; }

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
