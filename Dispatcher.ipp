
/*
//Constrain Ret = void, Args = const Event& 
template <typename Event>
using EventSignal = Signal<void(const Event&)>;

template <typename Event>
using EventSink = Sink<void(const Event&)>;


class Dispatcher
{
private:
	std::unordered_map<std::type_index, HandlerBase*> m_bucket;

public:

	template <typename Event>
	EventSink<Event> sink();

	template <typename Event, typename... Args>
	void trigger(Args&&... args);

	template <typename Event, typename... Args>
	void enqueue(Args&&... args);
};
*/

//private
template <typename Event>
void Dispatcher::_assure_()
{
	if (m_bucket.find(std::type_index(typeid(Event))) == m_bucket.end())
	{
		Handler<Event>* hnd = new Handler<Event>;
		m_bucket.insert({ std::type_index(typeid(Event)), hnd });
	}
}

/* Find handler, construct event, call handler's trigger */
template <typename Event>
void Dispatcher::trigger(Event&& event)
{
	using EventType = std::remove_cvref_t<Event>;
	_assure_<EventType>();

	HandlerBase* hb = m_bucket.at(std::type_index(typeid(EventType)));
	Handler<EventType>* hnd = static_cast<Handler<EventType>*>(hb);
	hnd->trigger(std::forward<Event>(event));
}

/* Find handler, construct event, call handler's enqueue */
template <typename Event>
void Dispatcher::enqueue(Event&& event)
{
	using EventType = std::remove_cvref_t<Event>;
	_assure_<EventType>();

	HandlerBase* hb = m_bucket.at(std::type_index(typeid(EventType)));
	Handler<EventType>* hnd = static_cast<Handler<EventType>*>(hb);

	hnd->enqueue(event); //Handler takes ownership of event via moving
}

/* Iterate every handler in bucket, call publish on each */
void Dispatcher::update()
{
	for (auto& [key, value] : m_bucket)
	{
		value->publish();
	}
}

/* Find handler, call handler's clear */
template <typename Event>
void Dispatcher::clear()
{
	m_bucket.at(std::type_index(typeid(Event)))->clear();
}

/* Find handler, call handler's sink, return to user */
template <typename Event>
EventSink<Event> Dispatcher::sink()
{
	_assure_<Event>();

	Handler<Event>* hnd = static_cast<Handler<Event>*>(m_bucket.at(std::type_index(typeid(Event))));
	return hnd->sink();
}




