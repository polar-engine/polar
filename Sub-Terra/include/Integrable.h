#pragma once

#include <memory>

class IntegrableBase {
public:
	virtual ~IntegrableBase() {}
	virtual bool HasDerivative(unsigned char = 0) = 0;
	virtual IntegrableBase & GetDerivative(unsigned char = 0) = 0;
	virtual void Integrate(const DeltaTicks::seconds_type) = 0;
	virtual void Revert() = 0;
};

template<typename T> class Integrable : public IntegrableBase {
private:
	typedef std::unique_ptr<Integrable<T>> derivative_type;
	T previousValue;
	T value;
	derivative_type derivative;
public:
	template<typename ...Ts> Integrable(Ts && ...args) : value(std::forward<Ts>(args)...), previousValue(value) {}

	inline T & Get() { return value; }
	inline T & GetPrevious() { return previousValue; }
	template<typename _To> inline _To To() { return static_cast<_To>(value); }
	template<typename _To> inline _To PreviousTo() { return static_cast<_To>(previousValue); }

	inline bool HasDerivative(unsigned char n = 0) override final {
		if(!derivative) {
			return false;
		} else if(n == 0) {
			return true;
		} else {
			return Derivative().HasDerivative(n - 1);
		}
	}

	inline IntegrableBase & GetDerivative(unsigned char n = 0) override final {
		if(n == 0) {
			if(!derivative) { derivative = derivative_type(new Integrable<T>()); }
			return *derivative;
		} else {
			return Derivative().Derivative(n - 1);
		}
	}

	inline Integrable<T> & Derivative(unsigned char n = 0) {
		if(n == 0) {
			if(!derivative) { derivative = derivative_type(new Integrable<T>()); }
			return *derivative;
		} else {
			return Derivative().Derivative(n - 1);
		}
	}

	inline Integrable<T> Temporal(const DeltaTicks::seconds_type alpha) {
		T newValue(value);
		if(HasDerivative()) {
			newValue += Derivative().Temporal(alpha).value;
		}
		return Integrable<T>(newValue * alpha + value * (1 - alpha));
	}

	inline void Integrate(const DeltaTicks::seconds_type seconds) override final {
		if(HasDerivative()) {
			if(HasDerivative(1)) { Derivative().Integrate(seconds); }
			previousValue = value;
			value += Derivative().value * seconds;
		}
	}

	inline void Revert() override final {
		value = previousValue;
	}

	inline T & operator*() { return value; }
	inline T * operator->() { return &value; }

	inline Integrable<T> & operator=(const T &rhs) {
		value = rhs;
		return *this;
	}

	inline Integrable<T> & operator-() {
		return -value;
	}

	inline Integrable<T> & operator+=(const DeltaTicks::seconds_type rhs) {
		value += rhs;
		return *this;
	}
	inline Integrable<T> & operator+=(const T &rhs) {
		value += rhs;
		return *this;
	}
	inline Integrable<T> & operator+=(const Integrable<T> &rhs) { return *this += rhs.value; }
	inline friend Integrable<T> operator+(Integrable<T> lhs, const T &rhs) { return lhs += rhs; }
	inline friend Integrable<T> operator+(Integrable<T> lhs, const Integrable<T> &rhs) { return lhs += rhs; }

	inline Integrable<T> & operator-=(const DeltaTicks::seconds_type rhs) {
		value -= rhs;
		return *this;
	}
	inline Integrable<T> & operator-=(const T &rhs) {
		value -= rhs;
		return *this;
	}
	inline Integrable<T> & operator-=(const Integrable<T> &rhs) { return *this -= rhs.value; }
	inline friend Integrable<T> operator-(Integrable<T> lhs, const T &rhs) { return lhs -= rhs; }
	inline friend Integrable<T> operator-(Integrable<T> lhs, const Integrable<T> &rhs) { return lhs -= rhs; }

	inline Integrable<T> & operator*=(const DeltaTicks::seconds_type rhs) {
		value *= rhs;
		return *this;
	}
	inline Integrable<T> & operator*=(const T &rhs) {
		value *= rhs;
		return *this;
	}
	inline Integrable<T> & operator*=(const Integrable<T> &rhs) { return *this *= rhs.value; }
	inline friend Integrable<T> operator*(Integrable<T> lhs, const T &rhs) { return lhs *= rhs; }
	inline friend Integrable<T> operator*(Integrable<T> lhs, const Integrable<T> &rhs) { return lhs *= rhs; }
};
