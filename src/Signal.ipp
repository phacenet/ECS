/*
template <typename Ret, typename ...Args>
class Signal
{
private:
	std::vector<std::function<Ret(Args...)>> m_callbacks;
	std::vector<CallbackToken> m_tokens;
*/

template <typename Ret, typename ...Args>
template <auto Function, typename... BoundArgs>
void Signal<Ret(Args...)>::connect(BoundArgs&&... args)
{

	/* Either has one param, or 0:
	Free function: sink.connect<&foo>();
	Member function: sink.connect<&MyClass::method>(this);
	Lambdas with no captures: sink.connect<[] (int x) { ... }>();
	*/

	CallbackToken token;
	token.functionPtr = getFunctionID<Function>();
	
	if constexpr (sizeof...(args) > 0)
	{
		static_assert(std::is_pointer_v<std::remove_cvref_t<std::tuple_element_t<0, std::tuple<BoundArgs...>>>>); //get first type from BoundArgs, strip refs/const, check if ptr
		token.instancePtr = static_cast<const void*>(std::get<0>(std::forward_as_tuple(args...)));
	}
	else
		token.instancePtr = nullptr;

	m_tokens.push_back(token);

	/* 
	If user passes something like sink.connect<&MyClass::onDamage>(myInstance);
	and the signature is void(int amount, float multiplier), then

	Function = &MyClass:onDamage
	args... = (myInstance)
	callArgs... = (amount, multiplier)

	and has to be called like: 
	myInstance->onDamage(amount, multiplier);
	which expands to: std::invoke(Function, args..., callArgs...);
	*/
	auto fn = [func = Function,
			...captured = std::forward<BoundArgs>(args)] (Args... callArgs) mutable
		{
			return std::invoke(func, captured..., std::forward<Args>(callArgs)...);
		};

	m_callbacks.push_back(fn);
}

template <typename Ret, typename ...Args>
void Signal<Ret(Args...)>::publish(Args... args)
{
	for (auto& funct : m_callbacks)
		funct(std::forward<Args>(args)...);
}

template <typename Ret, typename ...Args>
template <auto Function, typename... BoundArgs>
void Signal<Ret(Args...)>::disconnect(BoundArgs&&... args)
{
	CallbackToken token;
	token.functionPtr = getFunctionID<Function>();

	if constexpr (sizeof...(args) > 0)
	{
		static_assert(std::is_pointer_v < std::remove_cvref_t<std::tuple_element_t<0, std::tuple<BoundArgs...>>>>); //get first type from BoundArgs, strip refs/const, check if ptr
		token.instancePtr = static_cast<const void*>(std::get<0>(std::forward_as_tuple(args...)));
	}
	else
	{
		token.instancePtr = nullptr;
	}

	for (auto it = m_tokens.begin(); it != m_tokens.end(); ++it)
	{
		if (*it == token)
		{
			/* m_tokens and m_callbacks are always in sync, so calculate distance and erase from callbacks */
			size_t vec_it = std::distance(m_tokens.begin(), it);

			m_callbacks.erase(m_callbacks.begin() + vec_it);
			m_tokens.erase(it);
			break;
		}
	}
}

template <typename Ret, typename ...Args>
bool Signal<Ret(Args...)>::empty()
{
	return (m_callbacks.empty() && m_tokens.empty());
}
