#pragma once

#include "HandlerBase.h"
#include "Signal.h"

template <typename Event>
using EventSink = Sink<void(const Event&)>;

template <typename Event>
class Handler : public HandlerBase
{
private:
	Signal<void(const Event&)> m_signal;
	std::vector<Event> m_events;

public:
	virtual void publish() override;
	virtual void clear() override;

	void trigger(const Event& event);

	void enqueue(const Event& event);

	EventSink<Event> sink();
};

#include "Handler.ipp"

