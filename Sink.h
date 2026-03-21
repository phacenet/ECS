#pragma once

#include "Signal.h"

template <typename T>
class Sink
{
private:
	Signal<T>* m_signal;

public:
	explicit Sink(Signal<T>* signal);

	template <auto Function, typename... BoundArgs>
	void connect(BoundArgs&&... args);

	template <auto Function, typename ...BoundArgs>
	void disconnect(BoundArgs&&... args);

};

#include "Sink.ipp"