#include "Algorithm.h"
#include <iostream>


static vector<Algorithm *> vAlgorithms;

vector<Algorithm *>& GetAlgorithmList()
{
	return vAlgorithms;
}
void Algorithm::RegisterAlgorithm(Algorithm *pAlgorithm)
{
	vAlgorithms.push_back(pAlgorithm);
}

Algorithm* Algorithm::GetAlgorithm(string name)
{
	for(vector<Algorithm *>::iterator it = vAlgorithms.begin(); it != vAlgorithms.end(); it++)
	{
		if((*it)->GetName() == name)
			return *it;
	}
	return NULL;
}

void Algorithm::Test()
{
	cout << "Showing the entire algortihm list" << endl;
	for(vector<Algorithm *>::iterator it = vAlgorithms.begin(); it != vAlgorithms.end(); it++)
	{
		Algorithm *alg = *it;
		
		cout << "Name: " << alg->GetName() << endl;
		cout << "   Is CUDA capable: " << (alg->IsGPUCapable()? "yes" : "no") << endl;
		cout << "   Is CPU capable : " << (alg->IsCPUCapable()? "yes" : "no") << endl;
		cout << "   Hash length    : " << alg->HashLength() << endl;
	}
}

vector<string> Algorithm::GetAlgorithmNames()
{
	vector<string> v;
	for(vector<Algorithm *>::iterator it = vAlgorithms.begin(); it != vAlgorithms.end(); it++)
	{
		v.push_back((*it)->GetName());
	}
	return vector<string>(v);
}


