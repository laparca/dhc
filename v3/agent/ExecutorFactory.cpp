#include "ExecutorFactory.h"

Executor* ExecutorFactory::Get(const string& name)
{
	map<string, Executor*>::iterator value = vExecutors.find(name);
	if(value == vExecutors.end())
	{
		throw "Executor does not exist";
	}

	return value->second;
}

void ExecutorFactory::Register(Executor *ex)
{
	vExecutors[ex->GetName()] = ex;
}