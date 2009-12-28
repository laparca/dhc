#include "config.h"
#include <stdio.h>
#include <time.h>
#include <string>
#include "../agent/agent.h"
#include "../agent/ControllerLink.h"
#include "../agent/Thread.h"
#include "../agent/Mutex.h"

using namespace std;

#ifdef WINDOWS
/*!
	@brief Frequency of performance timer
 */
	__int64 g_perfFreq;
#endif

string g_hostname;
string g_server;

double g_tGet = 0;
double g_tSubmit = 0;
int g_nWUs = 0;
int g_nHits = 0;

Mutex g_statsMutex;

THREAD_PROTOTYPE(RequestThread, _notused);

int main(int argc, char* argv[])
{
	#ifdef WINDOWS
		//Initialize performance timer
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&g_perfFreq));
	#endif
	
	//Show warning
	printf(
		"WARNING: This benchmark will request a large number of work units from the controller and report incorrect results.\n"
		"This may cause disruption of currently running cracks.\n\nContinue? [y/n] "
		);
	char response = 'x';
	if(1 != scanf("%c", &response))
		return 0;
	if(response != 'y')
		return 0;	
		
	if(argc != 2)
	{
		cout << "Usage: stresstest [server]" << endl;
		return 0;
	}
	g_hostname = "stresstest";
	g_server = argv[1];
	
	//Spawn a bunch of threads and make a zillion simultaneous requests
	vector<Thread*> threads;
	int nThreads = 4;
	for(int i=0; i<nThreads; i++)
		threads.push_back(new Thread(RequestThread, NULL));
	for(int i=0; i<nThreads; i++)
	{
		threads[i]->WaitUntilTermination();
		delete threads[i];
	}
	
	//Print results
	float tTotal = (g_tGet + g_tSubmit) / g_nHits;
	tTotal /= nThreads;
	printf(
		"%d work units requested\n"
		"  Work units received : %d\n"
		"  Avg get             : %.2f ms\n"
		"  Avg submit          : %.2f ms\n"
		"  Total time          : %.2f sec\n"
		"  Avg speed           : %.2f WUs / sec\n"
		"  Est. capacity       : %d Tesla C1060\n",
		g_nWUs,
		g_nHits,
		(g_tGet / g_nWUs) * 1000,
		(g_tSubmit / g_nHits) * 1000,
		tTotal * g_nHits,
		1.0f / tTotal,
		static_cast<int>(2.5f / tTotal)	//max WUs processable in 2.5 seconds
		);
	
	return 0;
}

THREAD_PROTOTYPE(RequestThread, _notused)
{
	ComputeDevice device;
	device.bGPU = false;
	device.index = 0;
	ControllerLink link(&device);
		
	//All algorithms
	//TODO: Create a "test" algorithm so we dont break any real cracks
	vector<string> list;
	list.push_back("md5");
	list.push_back("md5crypt");
	list.push_back("md4");
	list.push_back("ntlm");
	list.push_back("sha1");
	
	//Grab a bunch of work units
	double tGet = 0;
	double tSubmit = 0;
	int nWUs = 500;
	int nHits = 0;
	WorkUnit wu;
	for(int i=0; i<nWUs; i++)
	{
		//Get the work unit
		double start = GetTime();
		bool bHit = link.GetWorkUnit(wu, list);
		double end = GetTime();
		tGet += end - start;
		
		//Submit WU results
		if(bHit)
		{
			start = GetTime();
			nHits++;
			link.SubmitResults(wu);
			end = GetTime();
			tSubmit += end - start;
		}
	}
	
	//Update global stats
	g_statsMutex.Lock();
		g_tSubmit += tSubmit;
		g_nHits += nHits;
		g_tGet += tGet;
		g_nWUs += nWUs;
	g_statsMutex.Unlock();
	
	THREAD_RETURN(0);
}

double GetTime()
{
#ifdef LINUX
	timespec t;
	clock_gettime(CLOCK_REALTIME,&t);
	double d = static_cast<double>(t.tv_nsec) / 1E9f;
	d += t.tv_sec;
	return d;
#elif WINDOWS
	__int64 tm;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&tm));
	double ret = tm;
	return ret / g_perfFreq;
#else
	#error Unsupported platform
#endif
}

/*!
	@brief Throws an exception of class std::string containing a nicely formatted error message.
	
	Do not call this function directly - use ThrowError() instead.
	@param err User-specified generic error message (ex: "Failed to open settings file")
	@param sys_err Human-readable form of error code (ex: "No such file or directory")
	@param file The file the error was thrown from
	@param line The line the error was thrown from
 */
void DoThrowError(const char* err, const char* sys_err, const char* file, int line)
{
	char* buf = new char[strlen(err) + strlen(sys_err) + strlen(file) + 256];
			
	//Format the error nicely
	sprintf(buf,"%s: %s\n    File: %s\n    Line: %d\n", err, sys_err, file, line);
	string serr(buf);
	delete[] buf;
	throw serr;
}
