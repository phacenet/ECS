
/*
template <typename Event>
class Handler : public HandlerBase
{
private:
	Signal<void(const Event&)> m_signal;
	std::vector<Event> m_events

public:
*/

//using EventSink = Sink<void(const Event&)>;

/*  
	Iterate Event vector (data structs) and pass 
	each data struct to the owned signal's callback functions
*/
template <typename Event>
/*virtual*/ void Handler<Event>::publish() //override
{
	for (auto& event : m_events)
	{
		m_signal.publish(event);
	}
	clear();
}

template <typename Event>
/*virtual*/ void Handler<Event>::clear() //override
{
	m_events.clear();
}

/* Call signal's callback immediately with Event data. Skip vector */
template <typename Event>
void Handler<Event>::trigger(const Event& event)
{
	m_signal.publish(event);
}

/*
	Push event data to queue to be passed to 
	signal and run in m_signal.publish(event) later.
	Takes ownership of event passed
*/
template <typename Event>
void Handler<Event>::enqueue(const Event& event)
{
	m_events.push_back(std::move(event));
}

/* Expose private m_signal through sink */
template <typename Event>
EventSink<Event> Handler<Event>::sink()
{
	EventSink<Event> sink{ &m_signal };
	return sink;
}