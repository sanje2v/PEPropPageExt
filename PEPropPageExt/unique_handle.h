#pragma once

static auto funcDeleteBrush = [](HBRUSH& hBrush) -> void
{
	if (hBrush)
		DeleteObject(hBrush), hBrush = NULL;
};

static auto funcDestroyWindow = [](HWND& hWnd) -> void
{
	if (hWnd)
		DestroyWindow(hWnd), hWnd = NULL;
};

template<typename T, typename Functor>
class unique_handle
{
private:
	T m_Handle;
	Functor m_funcReleaser;
	T m_resetValue;

public:
	unique_handle(T handle, Functor funcReleaser, T resetValue = T(NULL))
		:	m_Handle(handle),
			m_funcReleaser(funcReleaser),
			m_resetValue(resetValue) {}
	~unique_handle()
	{
		if (m_Handle != m_resetValue)
		{
			m_funcReleaser(m_Handle);
			m_Handle = m_resetValue;
		}
	}

	unique_handle<T, Functor>& operator=(unique_handle<T, Functor>&& other)
	{
		if (std::addressof(other) != this)
		{
			this->m_Handle = other.m_Handle;
			this->m_funcReleaser = other.m_funcReleaser;

			other.m_Handle = other.m_resetValue;
		}

		return *this;
	}

	unique_handle<T, Functor>& operator=(const T& other)
	{
		this->~unique_handle();
		this->m_Handle = other;

		return *this;
	}
	T *operator&() { return &m_Handle; }
	bool operator!() { return (m_Handle == m_resetValue); }

	T get() { return m_Handle; }
	T release_ownership() { T temp = m_Handle; m_Handle = m_resetValue; return temp; }
};