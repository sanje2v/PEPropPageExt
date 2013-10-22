#pragma once

#include "PropertyPageHandler.h"
#include <vector>
#include <functional>

using namespace std;


template<typename T>
class ScopedPointerVector
{
private:
	vector<T> vectorPointers;

public:
	explicit ScopedPointerVector();
	~ScopedPointerVector();

	void operator+= (T ptr);
	T last();
	T find(function<bool (PropertyPageHandler * pproppagehdlr, const int& resid)> matchfunc, int value);
};