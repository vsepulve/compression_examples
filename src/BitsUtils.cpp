#include "BitsUtils.h"

BitsUtils::BitsUtils(){}

BitsUtils::~BitsUtils(){}

// Returns the (effective) number of bits to write a value
unsigned int BitsUtils::nBits(unsigned long long value){
	unsigned int ret = 1;
	while( value >>= 1ull ){
		++ret;
	}
	return ret;
}

// Writes number "value" of length "len" bits to "out" from position "out_pos" (in bits)
// Note the caller is responsible to add "len" to its own "out_pos" for the next writing operation
void BitsUtils::writeBits32(unsigned int value, unsigned int len, unsigned int *out, unsigned int out_pos){
	// Writes number "value" of length "len" bits to "out" from position "out_pos" (in bits)
	// Note the caller is responsible to add "len" to its own "out_pos" for the next writing operation
//	cout<<"BitsUtils::writeBits - Start " << value << " (" << len << " bits) to position " << out_pos << "\n";
	// Advance out to out_pos in words
	out += out_pos >> BITS_WORD;
	// mask to consider the bit position inside the word 
	out_pos &= MASK_WORD;
	// Cases depending of len (full word or part of it)
	if (len == WORD_SIZE) {
		*out |= (*out & ((1<<out_pos) - 1)) | (value << out_pos);
		if (out_pos > 0){
			++out;
			*out = (*out & ~((1<<out_pos) - 1)) | (value >> (WORD_SIZE - out_pos));
		}
	}
	else {
		if (out_pos + len > WORD_SIZE) {
			*out = (*out & ((1<<out_pos) - 1)) | (value << out_pos);
			++out;
			len -= WORD_SIZE - out_pos;
			*out = (*out & ~((1<<len)-1)) | (value >> (WORD_SIZE - out_pos));
		}
		else{
			*out = (*out & ~( ((1<<len) - 1) << out_pos) ) | (value << out_pos);
		}
	}
//	cout<<"BitsUtils::writeBits - End\n";
}

// Writes number "value" of length "len" bits to "out" from position "out_pos" (in bits)
// Note the caller is responsible to add "len" to its own "out_pos" for the next writing operation
void BitsUtils::writeBits64(unsigned long long value, unsigned int len, unsigned long long *out, unsigned long long out_pos){
	// Writes number "value" of length "len" bits to "out" from position "out_pos" (in bits)
	// Note the caller is responsible to add "len" to its own "out_pos" for the next writing operation
//	cout<<"BitsUtils::writeBits64 - " << value << " (" << len << " bits) to position " << out_pos << "\n";
	// Advance out to out_pos in words
	out += out_pos >> 6;
	// mask to consider the bit position inside the word 
	out_pos &= 0x3full;
	// Cases depending of len (full word or part of it)
	if (len == 64) {
		*out |= (*out & ((1ull<<out_pos) - 1ull)) | (value << out_pos);
		if (!out_pos){
			return;
		}
		out++;
		*out = (*out & ~((1ull<<out_pos) - 1ull)) | (value >> (64 - out_pos));
	}
	else {
		if (out_pos + len <= 64) {
			*out = (*out & ~( ((1ull<<len) - 1ull) << out_pos) ) | (value << out_pos);
			return;
		}
		*out = (*out & ((1ull<<out_pos) - 1ull)) | (value << out_pos);
		out++;
		len -= 64 - out_pos;
		*out = (*out & ~((1ull<<len)-1ull)) | (value >> (64 - out_pos));
	}
}

// Read "len" bits from position "in_pos" (in bits) of "in" and returns the value as integer
// Note the caller is responsible to add "len" to its own "in_pos" for the next read operation
unsigned int BitsUtils::readBits32(unsigned int len, unsigned int *in, unsigned int in_pos){
	// position for in, in words
	unsigned int i = (in_pos >> BITS_WORD);
	// position in bits inside the word
	unsigned int j = in_pos & MASK_WORD;
	unsigned int value;
	if( j + len <= WORD_SIZE ){
		value = (in[i] << (WORD_SIZE-j-len)) >> (WORD_SIZE - len);
	}
	else {
		value = in[i] >> j;
		value = value | ( (in[i+1] << (WORD_SIZE-j-len)) >> (WORD_SIZE-len) );
	}
	return value;
}

// Read "len" bits from position "in_pos" (in bits) of "in" and returns the value as integer
// Note the caller is responsible to add "len" to its own "in_pos" for the next read operation
unsigned long long BitsUtils::readBits64(unsigned int len, unsigned long long *in, unsigned long long in_pos){
	// position for in, in words
	unsigned int i = (in_pos >> 6);
	// position in bits inside the word
	unsigned int j = in_pos & 0x3f;
	unsigned long long value;
	if( j + len <= 64 ){
		value = (in[i] << (64-j-len)) >> (64 - len);
	}
	else {
		value = in[i] >> j;
		value = value | ( (in[i+1] << (64-j-len)) >> (64-len) );
	}
	return value;
}

unsigned int BitsUtils::writeVarbyte(unsigned char *buff, unsigned long long num){
	unsigned char *out = buff;
	// if/else for each 7 bits used (plus 1 for exclusive <)
	if(num < 0x80){
		// 1 byte
		*out = (num & 0x7f);
		++out;
	}
	else if(num < 0x4000){
		// 2 byte
		*out = ((num >> 7) & 0x7f) | 0x80;
		++out;
		*out = (num & 0x7f);
		++out;
	}
	else if(num < 0x200000){
		// 3 byte
		*out = ((num >> 14) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 7) & 0x7f) | 0x80;
		++out;
		*out = (num & 0x7f);
		++out;
	}
	else if(num < 0x10000000){
		// 4 byte
		*out = ((num >> 21) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 14) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 7) & 0x7f) | 0x80;
		++out;
		*out = (num & 0x7f);
		++out;
	}
	else if(num < 0x800000000ULL){
		// 5 byte
		*out = ((num >> 28) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 21) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 14) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 7) & 0x7f) | 0x80;
		++out;
		*out = (num & 0x7f);
		++out;
	}
	else if(num < 0x40000000000ULL){
		// 6 byte
		*out = ((num >> 35) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 28) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 21) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 14) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 7) & 0x7f) | 0x80;
		++out;
		*out = (num & 0x7f);
		++out;
	}
	else if(num < 0x2000000000000ULL){
		// 7 byte
		*out = ((num >> 42) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 35) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 28) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 21) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 14) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 7) & 0x7f) | 0x80;
		++out;
		*out = (num & 0x7f);
		++out;
	}
	else if(num < 0x100000000000000ULL){
		// 8 byte
		*out = ((num >> 49) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 42) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 35) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 28) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 21) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 14) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 7) & 0x7f) | 0x80;
		++out;
		*out = (num & 0x7f);
		++out;
	}
	else{
		// 9 byte
		*out = ((num >> 56) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 49) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 42) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 35) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 28) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 21) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 14) & 0x7f) | 0x80;
		++out;
		*out = ((num >> 7) & 0x7f) | 0x80;
		++out;
		*out = (num & 0x7f);
		++out;
	}
	return (out - buff);
}

// Reads the first Varbyte value from "buff" and stores it in "value" returning the number of read bytes
// The caller must move the buff between read operations
unsigned int BitsUtils::readVarbyte(unsigned char *buff, unsigned long long &value){
	// Reset value
	value = 0;
	// Current read byte
	unsigned char *cur_byte = buff;
	while( (*cur_byte & 0x80) ){
		value |= (*cur_byte & 0x7f);
		value <<= 7;
		++cur_byte;
	}
	value |= *cur_byte;
	++cur_byte;
	// Read bytes
	return (cur_byte - buff);
}

// Returns the number of bytes used in Varbyte for a given value (from 1-9)
// This method can be used to prepare an exact size buffer
unsigned int BitsUtils::sizeVarbyte(unsigned long long value){
	if(value < 0x80){
		return 1;
	}
	else if(value < 0x4000){
		return 2;
	}
	else if(value < 0x200000){
		return 3;
	}
	else if(value < 0x10000000){
		return 4;
	}
	else if(value < 0x800000000ULL){
		return 5;
	}
	else if(value < 0x40000000000ULL){
		return 6;
	}
	else if(value < 0x2000000000000ULL){
		return 7;
	}
	else if(value < 0x100000000000000ULL){
		return 8;
	}
	else{
		return 9;
	}
}











