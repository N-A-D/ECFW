#pragma once

template <typename T>
class Counter {
public:
	Counter() {
		++count;
	}

	Counter(const Counter&) {
		++count;
	}

	Counter& operator=(const Counter&) {
		++count;
		return *this;
	}

	~Counter() {
		--count;
	}

	static size_t how_many() noexcept {
		return count;
	}

private:
	inline static size_t count = 0;
};