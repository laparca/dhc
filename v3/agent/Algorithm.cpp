#include "Algorithm.h"
#include <iostream>


static Algorithm::algorithm_list vAlgorithms;

Algorithm::algorithm_list& Algorithm::GetAlgorithmList()
{
	return vAlgorithms;
}
void Algorithm::RegisterAlgorithm(Algorithm *pAlgorithm)
{
	vAlgorithms.push_back(pAlgorithm);
}

Algorithm* Algorithm::GetAlgorithm(const string& name)
{
	for(algorithm_iterator it = vAlgorithms.begin(); it != vAlgorithms.end(); it++)
	{
		if((*it)->GetName() == name)
			return *it;
	}
	return NULL;
}

void Algorithm::Test()
{
	cout << "Showing the entire algortihm list" << endl;
	for(algorithm_iterator it = vAlgorithms.begin(); it != vAlgorithms.end(); it++)
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
	for(algorithm_iterator it = vAlgorithms.begin(); it != vAlgorithms.end(); it++)
	{
		v.push_back((*it)->GetName());
	}
	return vector<string>(v);
}


