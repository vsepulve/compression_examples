#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <algorithm>
#include <random>

#include <map>
#include <vector>
#include <list>

#include "BitsUtils.h"
#include "Simple9.h"

using namespace std;

int main(int argc, char* argv[]){

//	if(argc != 4){
//		cout << "\n";
//		cout << "Usage: compress_index text_index output_index compression_type\n";
//		cout << "List types: 1 (Uncompressed), 2 (VarByte), 3 (Simple9), 4 (PForDelta)\n";
//		cout << "For now payloads are always compressed with PForDelta\n";
//		cout << "\n";
//		return 0;
//	}
//	const string input = argv[1];
//	const string output = argv[2];
//	unsigned int comp_type = atoi(argv[3]);
	
	cout << "Start\n";
	
	random_device seed;
	mt19937 generator(seed());
	
	unsigned int MIN_VALUE = 0;
	unsigned int MAX_VALUE = 10;
//	uniform_int_distribution<unsigned int> rand_nvals(100, 1000);
//	unsigned int n_values = rand_nvals(generator);
	unsigned int n_values = 1000;
	uniform_int_distribution<unsigned int> rand_value(MIN_VALUE, MAX_VALUE);
	
//	unsigned int num_arr[n_values];
	unsigned int *num_arr = new unsigned int[n_values];
	for( unsigned int i = 0 ; i < n_values; ++i ){
		num_arr[i] = rand_value(generator);
//		cout << "Num Bits (" << num_arr[i] << "): " << (BitsUtils::nBits(num_arr[i])) << "\n";
	}
	
	/*
	uniform_int_distribution<unsigned long long> rand_value64(MIN_VALUE, 0x7fffffffffffffff);
	unsigned long long values_arr[n_values];
	unsigned int n_bits[n_values];
	for( unsigned int i = 0 ; i < n_values; ++i ){
		unsigned long long value = rand_value64(generator);
		values_arr[i] = value;
		n_bits[i] = BitsUtils::nBits(values_arr[i]);
		cout << "Value[" << i << "]: " << values_arr[i] << " (" << n_bits[i] << ")\n";
	}
	
	cout << "Writing Values\n";
	unsigned long long *buff = new unsigned long long[n_values];
	memset((char*)buff, 0, n_values * sizeof(long long));
	unsigned long long pos = 0;
	for( unsigned int i = 0 ; i < n_values; ++i ){
		BitsUtils::writeBits64(values_arr[i], n_bits[i], buff, pos);
		pos += n_bits[i];
	}
	
	cout << "Reading Values\n";
	pos = 0;
	for( unsigned int i = 0 ; i < n_values; ++i ){
		unsigned long long value = BitsUtils::readBits64(n_bits[i], buff, pos);
		cout << "Value[" << i << "]: " << value << " (from pos " << pos << ")\n";
		pos += n_bits[i];
	}
	
	delete [] buff;
	*/
	
	cout << "Storing numbers in Simple9\n";
	Simple9 s9(num_arr, n_values);
	
	cout << "Decompressing Simple9 (size " << s9.size() << " / " << n_values << ")\n";
	unsigned int *out = new unsigned int[s9.size()];
	s9.uncompress(out);
	for( unsigned int i = 0 ; i < n_values; ++i ){
		cout << "Value[" << i << "]: " << out[i] << " / " << num_arr[i] << "\n";
		if( out[i] != num_arr[i] ){
			cout << "ERROR\n";
			return 0;
		}
	}
	
	delete [] num_arr;
	delete [] out;
	
	
	cout << "End\n";
	
}

















