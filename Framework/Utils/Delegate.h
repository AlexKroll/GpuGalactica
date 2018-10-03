#pragma once

#include <windows.h>



// We use the delegate for deferred calling.

interface IDelegate abstract
{
public:
	virtual int invoke() = 0;

	virtual ~IDelegate() {}
};



template <class ClassType>
class Delegate : public IDelegate
{
public:
	Delegate(ClassType* pObject, int (ClassType::*pMethod)())
	{
		mObject = pObject;
		mMethod = pMethod;
	}

	virtual int invoke() override
	{
		if( mObject && mMethod )
			return (mObject->*mMethod)();
		else
			return -1;
	}

private:
	ClassType* mObject = nullptr;

	int (ClassType::*mMethod)() = nullptr;
};





