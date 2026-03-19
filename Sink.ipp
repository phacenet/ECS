

/*
template <typename T>
class Sink
{
private:
	Signal<T>* m_signal;

public:
	Sink(Signal<T>* signal);

*/

template <typename T>
Sink<T>::Sink(Signal<T>* signal)
	: m_signal(signal) {}

template <typename T>
template <auto Function, typename... BoundArgs>
void Sink<T>::connect(BoundArgs&&... args)
{
	m_signal->connect<Function>(std::forward<BoundArgs>(args)...);
}

template <typename T>
template <auto Function, typename ...BoundArgs>
void Sink<T>::disconnect(BoundArgs&&... args)
{
	m_signal->disconnect<Function>(std::forward<BoundArgs>>(args)...);
}