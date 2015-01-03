#pragma once

#include <memory>

class IntegrableBase {
public:
	virtual ~IntegrableBase() {}
	virtual bool HasDerivative(unsigned char = 0) = 0;
	virtual IntegrableBase & GetDerivative(unsigned char = 0) = 0;
	virtual void Integrate(const DeltaTicks::seconds_type) = 0;
};

template<typename T> class Integrable : public IntegrableBase {
private:
	typedef std::unique_ptr<Integrable<T>> derivative_type;
	T value;
	derivative_type derivative;
public:
	template<typename ...Ts> Integrable(Ts && ...args) : value(std::forward<Ts>(args)...) {}

	inline T & Get() { return value; }
	template<typename _To> inline _To To() { return static_cast<_To>(value); }

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

	inline void Integrate(const DeltaTicks::seconds_type seconds) override final {
		if(HasDerivative()) {
			if(HasDerivative(1)) { Derivative().Integrate(seconds); }
			value += Derivative().value * seconds;
		}
	}

	inline T & operator*() { return value; }
	inline T * operator->() { return &value; }

	inline Integrable<T> & operator=(const T &rhs) {
		value = rhs;
		return *this;
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
