#pragma once

#include "unordered_map"
#include <typeindex>
#include <type_traits>

#include "Signal.h"
#include "Sink.h"
#include "Handler.h"
#include "HandlerBase.h"

/* Constrain Ret = void, Args = const Event& */
template <typename Event>
using EventSignal = Signal<void(const Event&)>;

template <typename Event>
using EventSink = Sink<void(const Event&)>;


class Dispatcher
{
private:
	std::unordered_map<std::type_index, HandlerBase*> m_bucket;

	/* Check that handle is registered, and if not, register it */
	template <typename Event>
	void _assure_();

public:
	/* Find handler, construct event, call handler's trigger */
	template <typename Event>
	void trigger(Event&& event);

	/* Find handler, construct event, call handler's enqueue */
	template <typename Event>
	void enqueue(Event&& event);

	/* Iterate every handler in map, call publish on each */
	void update();

	/* Find handler, call handler's clear */
	template <typename Event>
	void clear();

	/* Find handler, call handler's sink, return to user */
	template <typename Event>
	EventSink<Event> sink();

};

#include "Dispatcher.ipp"
