#ifndef SHA256_H
#define SHA256_H

#include <Algorithm.h>

class sha256: public Algorithm {
public:
	string GetName()
	{
		return string("sha256");
	}
	virtual int HashLength()
	{
		return 32;
	}
	void ExecuteCPU();
	void ExecuteGPU();
	void Prepare() {}	
};

#endif
