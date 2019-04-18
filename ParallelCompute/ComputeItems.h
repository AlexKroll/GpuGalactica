#pragma once

#include <windows.h>
#include <memory>



interface IComputeTask
{
	virtual void release() = 0;

	virtual ~IComputeTask() = default;
};

typedef std::shared_ptr<IComputeTask> ComputeTask;




