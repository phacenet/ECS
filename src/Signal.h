#pragma once

#include <vector>
#include <functional>

#include "SignalBase.h"
#include "ComponentIndexing.h" //for storing a unique address for functionPtr

struct CallbackToken
{
	const void* functionPtr;
	const void* instancePtr;

	bool operator== (const CallbackToken& rhs) const
	{
		return (this->functionPtr == rhs.functionPtr && this->instancePtr == rhs.instancePtr);
	}
};

/* Disables calling like <returnType, args>  */
template <typename>
class Signal : public SignalBase
{ };

/* Forces <returnType(params...)> */
template <typename Ret, typename ...Args>
class Signal<Ret(Args...)> : public SignalBase
{
private:
	std::vector<std::function<Ret(Args...)>> m_callbacks;
	std::vector<CallbackToken> m_tokens;

public:
	template <auto Function, typename... BoundArgs>
	void connect(BoundArgs&&... args);

	void publish(Args... args);

	template <auto Function, typename... BoundArgs>
	void disconnect(BoundArgs&&... args);

	bool empty();
};


#include "Signal.ipp"
