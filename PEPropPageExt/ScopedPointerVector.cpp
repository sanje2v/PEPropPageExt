#include "ScopedPointerVector.h"


template<typename T>
ScopedPointerVector<T>::ScopedPointerVector()
{
	this->vectorPointers.clear();
}

template<typename T>
ScopedPointerVector<T>::~ScopedPointerVector()
{
	for (int i = 0; i < this->vectorPointers.size(); i++)
		if (this->vectorPointers[i])
			delete this->vectorPointers[i];
}

template<typename T>
void ScopedPointerVector<T>::operator+=(T ptr)
{
	this->vectorPointers.push_back(ptr);
}

template<typename T>
T ScopedPointerVector<T>::last()
{
	return this->vectorPointers.back();
}

template<typename T>
T ScopedPointerVector<T>::find(function<bool (PropertyPageHandler * pproppagehdlr, const int& resid)> matchfunc,
								int value)
{
	for (int i = 0; i < this->vectorPointers.size(); i++)
		if matchfunc(this->vectorPointers[i], value)
			return this->vectorPointers[i];

	return NULL;
}