#include "Algorithm.h"
#include <iostream>

void AlgorithmFactory::Test()
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

vector<string> AlgorithmFactory::GetAlgorithmNames()
{
	vector<string> v;
	for(algorithm_iterator it = vAlgorithms.begin(); it != vAlgorithms.end(); it++)
	{
		v.push_back((*it)->GetName());
	}
	return vector<string>(v);
}


