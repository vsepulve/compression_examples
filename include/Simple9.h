
#ifndef S9INCLUDED
#define S9INCLUDED

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <vector>

#include "BitsUtils.h"

#define NUMBERS_IN_BLOCK 128
#define END_OF_STREAM 0xffffffff

/*#include "InvertedList.h"*/
/*#include "BufferedList.h"*/
/*#include "Simple9Utils.h"*/

class Simple9BlockHeader {
	unsigned int startBlock; // position of the array where the block starts
	unsigned int lastDocId;  // last absolute docid in the block
	unsigned int numsInBlock; // amount of numbers in the block
public:
	Simple9BlockHeader(){
		startBlock = 0;
		lastDocId = 0;
		numsInBlock = 0;
	}
	void setBlockHeader(unsigned int _startBlock, unsigned int _lastDocId, unsigned int _numsInBlock){
		startBlock = _startBlock;
		lastDocId = _lastDocId;
		numsInBlock = _numsInBlock;
	}
	unsigned int getStartBlock() {return startBlock;}
	unsigned int getlastDocIdBlock() {return lastDocId;}
	unsigned int getnumsInBlock() {return numsInBlock;}
	void save(fstream *writer){
		writer->write((char*)&startBlock, sizeof(int));
		writer->write((char*)&lastDocId, sizeof(int));
		writer->write((char*)&numsInBlock, sizeof(int));
	}
	void load(fstream *reader){
		reader->read((char*)&startBlock, sizeof(int));
		reader->read((char*)&lastDocId, sizeof(int));
		reader->read((char*)&numsInBlock, sizeof(int));
	}
};


class Simple9 {

private:
	
/*	static const unsigned int W = 32;*/
/*	static const unsigned int bitsW = 5;*/
	
	// Case-adjusted bits for each case of bits for a number
	static const unsigned int TopBits[];
	// Max numbers that fit in a s9 word given a current number of values (similar to a floor)
	static const unsigned int maxNumbers[];
	// Bits for a number of values
	static const unsigned int Bits[];
	// Number of values given a value of TopBits
	static const unsigned int Numbers[];
	// Case value for a number of values in a S9 word
	static const unsigned int Case[];
	
 	unsigned int *docs_block_acum;
 	
	unsigned int *seq; 
	unsigned int firstDocId; // first docid in the list
	unsigned int lastDocId; // last docid in the list
	
	Simple9BlockHeader *block;
	unsigned int nBlocks;
	
	unsigned int nnums; // number of integers encoded in the sequence
	unsigned int nWords; // number of words used to encode the integers
	
	unsigned int curDocId;
	
	unsigned int lastBlock; // last block that was accessed
	
	unsigned int *buffer; // OJO, cambiar este numero
	
	unsigned int cur_pos; // current position in the inverted list
  
	unsigned char curPosBuffer; // current position in the buffer
	
public:
	
	Simple9();
	Simple9(unsigned int *values, unsigned int n);
	~Simple9();
	
	// Decodes all the words into buff
	// Assumes buff has capacity of at least size()
	// Writes exactly size() values (includes firstDocId, omits the remaining of the last word)
	// Does not acumulates the values (raw decompressino)
	unsigned int uncompress(unsigned int *buff);
	
	// Decodes a single word into buff
	// Returns the number of decoded values
	// Assumes buff has capacity at least 28, or it is NULL
	// Omits the writing and only return the number of values if buff == NULL
	unsigned int decodeWord(unsigned int word_pos, unsigned int *buff);
	
	// Decodes the word from posInit to posEnd into internal buffer
	void decodeToBuffer(unsigned int posInit, unsigned int posEnd);
	
	unsigned int getSizeData() const {return sizeof(int) + nWords * sizeof(unsigned int);}
	
	unsigned int getSizeBlock() const {return sizeof(int) + 2 * (nBlocks+1) * sizeof(int);}
	
	unsigned int size() const {return nnums;}
	
	void save(fstream *writer);
	void load(fstream *reader);
	
	// Methods for DAAT query processing
	unsigned int next(unsigned int docId);
	unsigned int next();
	void reset();
	
	unsigned int getDocId() const {return curDocId;}
	
	bool hasNext();
	
	void setBufferMemory(unsigned int *buff){
		buffer = buff;
	}

};
 
 #endif
