#ifndef _BITS_UTILS_H
#define _BITS_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

using namespace std;

class BitsUtils {

private: 
	
	// Parametros de bit[get/put] para 32 bits (fijos y estaticos)
	static const unsigned int WORD_SIZE = 32;
	static const unsigned int BITS_WORD = 5;
	static const unsigned int MASK_WORD = 0x1f;
	
public: 
	
	BitsUtils();
	
	~BitsUtils();
	
	// Returns the (effective) number of bits to write a value
	static unsigned int nBits(unsigned long long value);
	
	// Writes number "value" of length "len" bits to "out" from position "out_pos" (in bits)
	// Note the caller is responsible to add "len" to its own "out_pos" for the next write operation
	static void writeBits32(unsigned int value, unsigned int len, unsigned int *out, unsigned int out_pos);
	
	// Read "len" bits from position "in_pos" (in bits) of "in" and returns the value as integer
	// Note the caller is responsible to add "len" to its own "in_pos" for the next read operation
	static unsigned int readBits32(unsigned int len, unsigned int *in, unsigned int in_pos);
	
	// Writes number "value" of length "len" bits to "out" from position "out_pos" (in bits)
	// Note the caller is responsible to add "len" to its own "out_pos" for the next write operation
	static void writeBits64(unsigned long long value, unsigned int len, unsigned long long *out, unsigned long long out_pos);
	
	// Read "len" bits from position "in_pos" (in bits) of "in" and returns the value as integer
	// Note the caller is responsible to add "len" to its own "in_pos" for the next read operation
	static unsigned long long readBits64(unsigned int len, unsigned long long *in, unsigned long long in_pos);
	
	// Writes the number "value" into "buff", returning the number of used bytes
	// Note the maximum number of possible bytes is 9 (Varbyte worst case for 8 bytes numbers)
	// The caller must move the buff between write operations
	static unsigned int writeVarbyte(unsigned char *buff, unsigned long long value);
	
	// Reads the first Varbyte value from "buff" and stores it in "value" returning the number of read bytes
	// The caller must move the buff between read operations
	static unsigned int readVarbyte(unsigned char *buff, unsigned long long &value);
	
	// Returns the number of bytes used in Varbyte for a given value (from 1-9)
	// This method can be used to prepare an exact size buffer
	static unsigned int sizeVarbyte(unsigned long long value);
	
	
};








#endif //_BITS_UTILS_H

