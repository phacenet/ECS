#pragma once

class HandlerBase
{
public:
	virtual void publish() = 0;
	virtual void clear() = 0;
	virtual ~HandlerBase() = default;
};
