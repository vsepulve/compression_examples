
#include "Simple9.h"

// Case-adjusted bits for each case of bits for a number (similar to a ceil)
const unsigned int Simple9::TopBits[33] = {1,1,2,3,4,5,7,7,9,9,14,14,14,14,14,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28};
// Max numbers that fit in a s9 word given a current number of values (similar to a floor)
const unsigned int Simple9::maxNumbers[29] = {1,1,2,3,4,5,5,7,7,9,9,9,9,9,14,14,14,14,14,14,14,14,14,14,14,14,14,14,28};
// Bits for a number of values
const unsigned int Simple9::Bits[29] = {28,28,14,9,7,5,5,4,4,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1};
// Number of values given a value of TopBits
const unsigned int Simple9::Numbers[29] = {0,28,14,9,7,5,0,4,0,3,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
// Case value for a number of values in a S9 word
const unsigned int Simple9::Case[29] = {0,1,2,3,4,5,0,6,0,7,0,0,0,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,9};

//Simple9Utils Simple9::utils;

Simple9::Simple9(){
	seq = NULL;
	nnums = 0;
}

Simple9::Simple9(unsigned int *values, unsigned int n){
	cout << "Simple9 - Start\n";
	
	unsigned int maxBits, curCant, nbits;
	
	vector<unsigned int> tempArray;

	nnums = n;
	nWords = 0;
	maxBits = 0;

	firstDocId = values[0]; // stores first docID separately
	curDocId = firstDocId;
	
	lastDocId = 0;
	for (unsigned int i = 0; i < n; ++i){
		lastDocId += values[i];
	}
	
	curCant = 0;
	
	cout << "Simple9 - Processing Words\n";
	unsigned int pos = 1;
	while (pos < n) {
		nbits = BitsUtils::nBits(values[pos]);
		if (nbits >= maxBits){
			maxBits = TopBits[nbits];
		}
		if(curCant + 1 == Numbers[maxBits]){
			// curCant + 1 fill the word using maxBits per number
			unsigned int word = 0;
			BitsUtils::writeBits32(Case[curCant+1], 4, &word, 28);
			for (unsigned int k = 0; k < curCant+1; k++){
				BitsUtils::writeBits32(values[pos-curCant+k], maxBits, &word, maxBits*k);
			}
			tempArray.push_back(word);
			curCant = 0;
			maxBits = 0;
			++pos;
		}
		else if(curCant + 1 > Numbers[maxBits]){
			// curCant + 1 exceeds the word with maxBits
			// We need to use the worst case for curCant 
			// maxNumbers[curCant] instead of curCant, using Bits[curCant]
			// Note we also reset pos
			pos -= curCant;
			maxBits = Bits[curCant];
			unsigned int word = 0;
			BitsUtils::writeBits32(Case[maxNumbers[curCant]], 4, &word, 28);
			for (unsigned int k = 0; k < maxNumbers[curCant]; ++k, ++pos){
				BitsUtils::writeBits32(values[pos], maxBits, &word, maxBits*k);
			}
			tempArray.push_back(word);
			curCant = 0;
			maxBits = 0;
		}
		else {
			// curCant + 1 doesn't fill the word
			++pos;
			++curCant;
		}
	}
	
	// Remaining curCant numbers (that must fit in a single word)
	cout << "Simple9 - Processing last word\n";
	if (curCant > 0){
		unsigned int word = 0;
		BitsUtils::writeBits32(Case[Numbers[maxBits]], 4, &word, 28);
		for (unsigned int j = 0; j < curCant; j++){
			BitsUtils::writeBits32(values[pos-curCant+j], maxBits, &word, maxBits*j);
		}
		// Fill the word with 0s
		for (unsigned int j = curCant; j < Numbers[maxBits]; j++){
			BitsUtils::writeBits32(0, maxBits, &word, maxBits*j);
		}
		tempArray.push_back(word);
	} 
	
	nWords = tempArray.size();
	seq = new unsigned int[nWords];
	for (unsigned int i = 0; i < nWords; i++){
		seq[i] = tempArray[i];
	}
	tempArray.clear();
	
	// now creates the blocks
	// counts the amount of numbers in a block
	unsigned int numbersInBlock = 0; 

	cout << "Simple9 - Processing num Blocks\n";
	nBlocks = 0; // number of blocks in the list
	for (unsigned i = 0; i < nWords; i++) {
		switch (seq[i]>>28) {
		  case 1: numbersInBlock += 1;
				  break;
		  case 2: numbersInBlock += 2;
				  break;
		  case 3: numbersInBlock += 3;
				  break;
		  case 4: numbersInBlock += 4;
				  break;
		  case 5: numbersInBlock += 5;
				  break;
		  case 6: numbersInBlock += 7;
				  break;
		  case 7: numbersInBlock += 9;
				  break;
		  case 8: numbersInBlock += 14;
				  break;
		  case 9: numbersInBlock += 28;
				  break;
		}
		if (numbersInBlock >= NUMBERS_IN_BLOCK) {
		  nBlocks++;
		  numbersInBlock = 0;
		}
	}
	if (numbersInBlock > 0){
		nBlocks += 1;
	}
	
	cout << "Simple9 - Creating Blocks\n";
	block = new Simple9BlockHeader[nBlocks+1];
	numbersInBlock = 0;
	unsigned int curB = 0, prevHeader = 0, curD = firstDocId;
	for (unsigned int i = 0; i < nWords; ++i) {
		switch (seq[i]>>28) {
		  case 1: numbersInBlock += 1;
				  curD += (seq[i] & 0x0fffffff); 
				  break;
		  case 2: numbersInBlock += 2;
				  curD += (seq[i] & 0x00003fff) + ((seq[i]>>14) & 0x00003fff); 
				  break;
		  case 3: numbersInBlock += 3;
				  curD += (seq[i] & 0x000001ff) + ((seq[i]>>9) & 0x000001ff) + ((seq[i]>>18) & 0x000001ff);
				  break;
		  case 4: numbersInBlock += 4;
				  curD += (seq[i] & 0x0000007f) + ((seq[i]>>7) & 0x0000007f) + ((seq[i]>>14) & 0x0000007f)  
						+ ((seq[i]>>21) & 0x0000007f);	
				  break;
		  case 5: numbersInBlock += 5;
				  curD += (seq[i] & 0x0000001f) + ((seq[i]>>5) & 0x0000001f) + ((seq[i]>>10) & 0x0000001f)
						+ ((seq[i]>>15) & 0x0000001f) + ((seq[i]>>20) & 0x0000001f); 
				  break;
		  case 6: numbersInBlock += 7;
				  curD += (seq[i] & 0x0000000f) + ((seq[i]>>4) & 0x0000000f) + ((seq[i]>>8) & 0x0000000f)
						+ ((seq[i]>>12) & 0x0000000f) + ((seq[i]>>16) & 0x0000000f) + ((seq[i]>>20) & 0x0000000f)
						+ ((seq[i]>>24) & 0x0000000f); 
				  break;
		  case 7: numbersInBlock += 9;
				  curD += (seq[i] & 0x00000007) + ((seq[i]>>3) & 0x00000007) +  ((seq[i]>>6) & 0x00000007)
				  + ((seq[i]>>9) & 0x00000007) + ((seq[i]>>12) & 0x00000007) + ((seq[i]>>15) & 0x00000007) 
				  + ((seq[i]>>18) & 0x00000007) + ((seq[i]>>21) & 0x00000007) + ((seq[i]>>24) & 0x00000007); 
				  break;
		  case 8: numbersInBlock += 14;
				  curD += (seq[i] & 0x00000003) + ((seq[i]>>2) & 0x00000003) + ((seq[i]>>4) & 0x00000003)
				  + ((seq[i]>>6) & 0x00000003) + ((seq[i]>>8) & 0x00000003) + ((seq[i]>>10) & 0x00000003)
				  + ((seq[i]>>12) & 0x00000003) + ((seq[i]>>14) & 0x00000003) + ((seq[i]>>16) & 0x00000003)
				  + ((seq[i]>>18) & 0x00000003) + ((seq[i]>>20) & 0x00000003) + ((seq[i]>>22) & 0x00000003)
				  + ((seq[i]>>24) & 0x00000003) + ((seq[i]>>26) & 0x00000003); 
				  break;
		  case 9: numbersInBlock += 28;
				  curD += (seq[i] & 0x00000001) + ((seq[i]>>1) & 0x00000001) + ((seq[i]>>2) & 0x00000001)
				  + ((seq[i]>>3) & 0x00000001)+ ((seq[i]>>4) & 0x00000001)+ ((seq[i]>>5) & 0x00000001)
				  + ((seq[i]>>6) & 0x00000001)+ ((seq[i]>>7) & 0x00000001)+ ((seq[i]>>8) & 0x00000001)
				  + ((seq[i]>>9) & 0x00000001)+ ((seq[i]>>10) & 0x00000001)+ ((seq[i]>>11) & 0x00000001)
				  + ((seq[i]>>12) & 0x00000001)+ ((seq[i]>>13) & 0x00000001)+ ((seq[i]>>14) & 0x00000001)
				  + ((seq[i]>>15) & 0x00000001)+ ((seq[i]>>16) & 0x00000001)+ ((seq[i]>>17) & 0x00000001)
				  + ((seq[i]>>18) & 0x00000001)+ ((seq[i]>>19) & 0x00000001)+ ((seq[i]>>20) & 0x00000001)
				  + ((seq[i]>>21) & 0x00000001)+ ((seq[i]>>22) & 0x00000001)+ ((seq[i]>>23) & 0x00000001)
				  + ((seq[i]>>24) & 0x00000001)+ ((seq[i]>>25) & 0x00000001)+ ((seq[i]>>26) & 0x00000001)
				  + ((seq[i]>>27) & 0x00000001);
				  break;
		}
		if (numbersInBlock >= NUMBERS_IN_BLOCK) {
		  block[curB].setBlockHeader(prevHeader, curD, numbersInBlock);
		  prevHeader = nWords; //will start a new block, points to the beginning of it
		  numbersInBlock = 0;
		  curB++;
		}
	}
	if (numbersInBlock > 0) {
		block[curB].setBlockHeader(prevHeader, curD, numbersInBlock);
		prevHeader = nWords;
		curB++;
	}
	block[curB].setBlockHeader(prevHeader, 0, 0);
	
//	cout << "Simple9 - Deleting tmp\n";
//	delete [] tempArray;
	//~ cout << "Saliendo Simple9 constructor" << endl;
	
	cout << "Simple9 - Creating docs_block_acum\n";
	docs_block_acum = new unsigned int[nBlocks+1];
	for(unsigned int i = 0; i < nBlocks; ++i){
		docs_block_acum[i] = block[i].getnumsInBlock();
	}
	for(unsigned int i = 1; i < nBlocks; ++i){
		docs_block_acum[i] += docs_block_acum[i-1];
	}
	
	cout << "Simple9 - End\n";
}

Simple9::~Simple9(){
//	cout << "~Simple9 - Start\n";
	if(seq != NULL){
		delete [] seq;
		seq = NULL;
	}
	if(docs_block_acum != NULL){
		delete [] docs_block_acum;
		docs_block_acum = NULL;
	}
//	cout << "~Simple9 - End\n";
}

void Simple9::reset() {
	cur_pos = 0;
	curDocId = firstDocId;
	lastBlock = -1;
}

// Decodes all the words into buff
// Assumes buff has capacity of at least size()
// Writes exactly size() values (includes firstDocId, omits the remaining of the last word)
// Does not acumulates the values (raw decompressino)
unsigned int Simple9::uncompress(unsigned int* out){
	unsigned int *p = out;
	register unsigned int regAux;
	p[0] = firstDocId;
	++p;
	// Note the last word may contain more numbers for the 0's padding the capacity
	for(unsigned int i = 0; i < nWords-1; ++i){
		regAux = seq[i];
		switch (regAux>>28) {
			case 1:
				*p = regAux & 0x0fffffff; 
				p++;
				break;
			case 2:
				*p = regAux & 0x00003fff; 
				*(p + 1) = (regAux>>14) & 0x00003fff;
				p += 2;
				break;
			case 3:
				*p = regAux & 0x000001ff;
				*(p + 1) = (regAux>>9) & 0x000001ff;
				*(p + 2) = (regAux>>18) & 0x000001ff;
				p += 3;
				break;
			case 4:
				*p = regAux & 0x0000007f;
				*(p + 1) = (regAux>>7) & 0x0000007f;
				*(p + 2) = (regAux>>14) & 0x0000007f; 
				*(p + 3) = (regAux>>21) & 0x0000007f; 
				p += 4;
				break;
			case 5:
				*p = regAux & 0x0000001f;
				*(p + 1) = (regAux>>5) & 0x0000001f;
				*(p + 2) = (regAux>>10) & 0x0000001f; 
				*(p + 3) = (regAux>>15) & 0x0000001f; 
				*(p + 4) = (regAux>>20) & 0x0000001f; 
				p += 5;
				break;
			case 6:
				*p = regAux & 0x0000000f; 
				*(p + 1) = (regAux>>4) & 0x0000000f; 
				*(p + 2) = (regAux>>8) & 0x0000000f; 
				*(p + 3) = (regAux>>12) & 0x0000000f; 
				*(p + 4) = (regAux>>16) & 0x0000000f; 
				*(p + 5) = (regAux>>20) & 0x0000000f; 
				*(p + 6) = (regAux>>24) & 0x0000000f;
				p += 7;
				break;
			case 7:
				*p = regAux & 0x00000007;
				*(p  + 1) = (regAux>>3) & 0x00000007;
				*(p  + 2) = (regAux>>6) & 0x00000007; 
				*(p  + 3) = (regAux>>9) & 0x00000007; 
				*(p  + 4) = (regAux>>12) & 0x00000007;
				*(p  + 5) = (regAux>>15) & 0x00000007; 
				*(p  + 6) = (regAux>>18) & 0x00000007;
				*(p  + 7) = (regAux>>21) & 0x00000007;
				*(p  + 8) = (regAux>>24) & 0x00000007; 
				p += 9;
				break;
			case 8:
				*p = regAux & 0x00000003; 
				*(p  + 1) = (regAux>>2) & 0x00000003;
				*(p  + 2) = (regAux>>4) & 0x00000003;
				*(p  + 3) = (regAux>>6) & 0x00000003;
				*(p  + 4) = (regAux>>8) & 0x00000003;
				*(p  + 5) = (regAux>>10) & 0x00000003;
				*(p  + 6) = (regAux>>12) & 0x00000003;
				*(p  + 7) = (regAux>>14) & 0x00000003;
				*(p  + 8) = (regAux>>16) & 0x00000003;
				*(p  + 9) = (regAux>>18) & 0x00000003;
				*(p  + 10) = (regAux>>20) & 0x00000003;
				*(p  + 11) = (regAux>>22) & 0x00000003;
				*(p  + 12) = (regAux>>24) & 0x00000003;
				*(p  + 13) = (regAux>>26) & 0x00000003; 
				p += 14;
				break;
			case 9:
				*p  = 1; *(p  + 1) = 1;  
				*(p  + 2) = 1; *(p  + 3) = 1;  
				*(p  + 4) = 1; *(p  + 5) = 1;  
				*(p  + 6) = 1; *(p  + 7) = 1;  
				*(p  + 8) = 1; *(p  + 9) = 1;  
				*(p  + 10) = 1; *(p  + 11) = 1;  
				*(p  + 12) = 1; *(p  + 13) = 1;  
				*(p  + 14) = 1; *(p  + 15) = 1;  
				*(p  + 16) = 1; *(p  + 17) = 1;  
				*(p  + 18) = 1; *(p  + 19) = 1;  
				*(p  + 20) = 1; *(p  + 21) = 1;  
				*(p  + 22) = 1; *(p  + 23) = 1;  
				*(p  + 24) = 1; *(p  + 25) = 1;  
				*(p  + 26) = 1; *(p  + 27) = 1;  
				p += 28;
				break;
		}
	}
	// Last word (check effective number of values)
	regAux = seq[nWords-1];
	unsigned int remaining = nnums - (p - out);
//	cout << "nums: " << (p - out) << "\n";
//	cout << "remaining: " << remaining << "\n";
	switch (regAux>>28) {
		case 1:
			*p = regAux & 0x0fffffff;
			p++;
			break;
		case 2:
			for(unsigned int i = 0; i < 2 && i < remaining; ++i){
				*p = (regAux >> (14*i)) & 0x00003fff;
				++p;
			}
			break;
		case 3:
			for(unsigned int i = 0; i < 3 && i < remaining; ++i){
				*p = (regAux >> (9*i)) & 0x000001ff;
				++p;
			}
			break;
		case 4:
			for(unsigned int i = 0; i < 4 && i < remaining; ++i){
				*p = (regAux >> (7*i)) & 0x0000007f;
				++p;
			}
			break;
		case 5:
			for(unsigned int i = 0; i < 5 && i < remaining; ++i){
				*p = (regAux >> (5*i)) & 0x0000001f;
				++p;
			}
			break;
		case 6:
			for(unsigned int i = 0; i < 7 && i < remaining; ++i){
				*p = (regAux >> (4*i)) & 0x0000000f;
				++p;
			}
			break;
		case 7:
			for(unsigned int i = 0; i < 9 && i < remaining; ++i){
				*p = (regAux >> (3*i)) & 0x00000007;
				++p;
			}
			break;
		case 8:
			for(unsigned int i = 0; i < 14 && i < remaining; ++i){
				*p = (regAux >> (2*i)) & 0x00000003;
				++p;
			}
			break;
		case 9:
			for(unsigned int i = 0; i < 14 && i < remaining; ++i){
				*p = 1;
				++p;
			}
			break;
	}
	return out - p;
}
// Decodes a single word into buff
// Returns the number of decoded values
// Assumes buff has capacity at least 28, or it is NULL
// Omits the writing and only return the number of values if buff == NULL
unsigned int Simple9::decodeWord(unsigned int word_pos, unsigned int *buff){
	unsigned int word = seq[word_pos];
	switch (word>>28) {
		case 1:
			if( buff != NULL ){
				*buff = word & 0x0fffffff;
			}
			return 1;
		case 2:
			if( buff != NULL ){
				*buff = word & 0x3fff; 
				*(buff + 1) = (word>>14) & 0x3fff;
			}
			return 2;
		case 3:
			if( buff != NULL ){
				*buff = word & 0x1ff;
				*(buff + 1) = (word>>9) & 0x1ff;
				*(buff + 2) = (word>>18) & 0x1ff;
			}
			return 3;
		case 4:
			if( buff != NULL ){
				*buff = word & 0x7f; *(buff + 1) = (word>>7) & 0x7f;
				*(buff + 2) = (word>>14) & 0x7f; *(buff + 3) = (word>>21) & 0x7f;
			}
			return 4;
		case 5:
			if( buff != NULL ){
				*buff = word & 0x1f; *(buff + 1) = (word>>5) & 0x1f;
				*(buff + 2) = (word>>10) & 0x1f; *(buff + 3) = (word>>15) & 0x1f; 
				*(buff + 4) = (word>>20) & 0x1f; 
			}
			return 5;
		case 6:
			if( buff != NULL ){
				*buff = word & 0xf; *(buff + 1) = (word>>4) & 0xf;
				*(buff + 2) = (word>>8) & 0xf; *(buff + 3) = (word>>12) & 0xf;
				*(buff + 4) = (word>>16) & 0xf; *(buff + 5) = (word>>20) & 0xf;
				*(buff + 6) = (word>>24) & 0xf;
			}
			return 7;
		case 7:
			if( buff != NULL ){
				*buff = word & 0x7; *(buff + 1) = (word>>3) & 0x7;
				*(buff + 2) = (word>>6) & 0x7; *(buff + 3) = (word>>9) & 0x7; 
				*(buff + 4) = (word>>12) & 0x7; *(buff + 5) = (word>>15) & 0x7; 
				*(buff + 6) = (word>>18) & 0x7; *(buff + 7) = (word>>21) & 0x7;
				*(buff + 8) = (word>>24) & 0x7;
			}
			return 9;
		case 8:
			if( buff != NULL ){
				*buff = word & 0x3; *(buff + 1) = (word>>2) & 0x3;
				*(buff + 2) = (word>>4) & 0x3; *(buff + 3) = (word>>6) & 0x3;
				*(buff + 4) = (word>>8) & 0x3; *(buff + 5) = (word>>10) & 0x3;
				*(buff + 6) = (word>>12) & 0x3; *(buff + 7) = (word>>14) & 0x3;
				*(buff + 8) = (word>>16) & 0x3; *(buff + 9) = (word>>18) & 0x3;
				*(buff + 10) = (word>>20) & 0x3; *(buff + 11) = (word>>22) & 0x3;
				*(buff + 12) = (word>>24) & 0x3; *(buff + 13) = (word>>26) & 0x3;
			}
			return 14;
		case 9:
			if( buff != NULL ){
				// Direct version (supports 0 values)
				*buff = word & 0x1; *(buff + 1) = (word>>1) & 0x1; *(buff + 2) = (word>>2) & 0x1;
				*(buff + 3) = (word>>3) & 0x1; *(buff + 4) = (word>>4) & 0x1; *(buff + 5) = (word>>5) & 0x1;
				*(buff + 6) = (word>>6) & 0x1; *(buff + 7) = (word>>7) & 0x1; *(buff + 8) = (word>>8) & 0x1;
				*(buff + 9) = (word>>9) & 0x1; *(buff + 10) = (word>>10) & 0x1; *(buff + 11) = (word>>11) & 0x1;
				*(buff + 12) = (word>>12) & 0x1; *(buff + 13) = (word>>13) & 0x1; *(buff + 14) = (word>>14) & 0x1;
				*(buff + 15) = (word>>15) & 0x1; *(buff + 16) = (word>>16) & 0x1; *(buff + 17) = (word>>17) & 0x1;
				*(buff + 18) = (word>>18) & 0x1; *(buff + 19) = (word>>19) & 0x1; *(buff + 20) = (word>>20) & 0x1;
				*(buff + 21) = (word>>21) & 0x1; *(buff + 22) = (word>>22) & 0x1; *(buff + 23) = (word>>23) & 0x1;
				*(buff + 24) = (word>>24) & 0x1; *(buff + 25) = (word>>25) & 0x1; *(buff + 26) = (word>>26) & 0x1;
				*(buff + 27) = (word>>27) & 0x1;
				// Possible slightly faster but less portable version (doesn't support 0 values)
//				*buff = 1; *(buff + 1) = 1; *(buff + 2) = 1; *(buff + 3) = 1;
//				*(buff + 4) = 1; *(buff + 5) = 1; *(buff + 6) = 1;
//				*(buff + 7) = 1; *(buff + 8) = 1; *(buff + 9) = 1;
//				*(buff + 10) = 1; *(buff + 11) = 1; *(buff + 12) = 1;
//				*(buff + 13) = 1; *(buff + 14) = 1; *(buff + 15) = 1;
//				*(buff + 16) = 1; *(buff + 17) = 1; *(buff + 18) = 1;
//				*(buff + 19) = 1; *(buff + 20) = 1; *(buff + 21) = 1;
//				*(buff + 22) = 1; *(buff + 23) = 1; *(buff + 24) = 1;
//				*(buff + 25) = 1; *(buff + 26) = 1; *(buff + 27) = 1;
			}
			return 28;
	}
	return 0;
}

// Decodes the word from posInit to posEnd into internal buffer
void Simple9::decodeToBuffer(unsigned int posInit, unsigned int posEnd){
	register unsigned int regAux;
	unsigned int *p = buffer;
	
	for (; posInit < posEnd; ++posInit) {
		  
		  regAux = seq[posInit];
		  
		  switch(regAux >> 28) {
			 case 1: *p= regAux & 0x0fffffff; 
			 p++;
					 break;
			 case 2: *p = regAux & 0x00003fff; 
					 *(p + 1) = (regAux>>14) & 0x00003fff;
				 p += 2;
					 break;
			 case 3: *p = regAux & 0x000001ff;
					 *(p + 1) = (regAux>>9) & 0x000001ff;
					 *(p + 2) = (regAux>>18) & 0x000001ff;
				 p += 3;
					 break;
			 case 4: *p = regAux & 0x0000007f;
				 *(p + 1) = (regAux>>7) & 0x0000007f;
					 *(p + 2) = (regAux>>14) & 0x0000007f; 
					 *(p + 3) = (regAux>>21) & 0x0000007f; 
					 p += 4;
					 break;
			 case 5: *p = regAux & 0x0000001f;
					 *(p + 1) = (regAux>>5) & 0x0000001f;
					 *(p + 2) = (regAux>>10) & 0x0000001f; 
					 *(p + 3) = (regAux>>15) & 0x0000001f; 
					 *(p + 4) = (regAux>>20) & 0x0000001f; 
			 p += 5;
					 break;
			 case 6: *p = regAux & 0x0000000f; 
					 *(p + 1) = (regAux>>4) & 0x0000000f; 
					 *(p + 2) = (regAux>>8) & 0x0000000f; 
					 *(p + 3) = (regAux>>12) & 0x0000000f; 
					 *(p + 4) = (regAux>>16) & 0x0000000f; 
					 *(p + 5) = (regAux>>20) & 0x0000000f; 
					 *(p + 6) = (regAux>>24) & 0x0000000f;
			 p += 7;
					 break;
			 case 7: *p = regAux & 0x00000007;
					 *(p  + 1) = (regAux>>3) & 0x00000007;
					 *(p  + 2) = (regAux>>6) & 0x00000007; 
					 *(p  + 3) = (regAux>>9) & 0x00000007; 
					 *(p  + 4) = (regAux>>12) & 0x00000007;
					 *(p  + 5) = (regAux>>15) & 0x00000007; 
					 *(p  + 6) = (regAux>>18) & 0x00000007;
					 *(p  + 7) = (regAux>>21) & 0x00000007;
					 *(p  + 8) = (regAux>>24) & 0x00000007; 
					 p += 9;
					 break;
			 case 8: *p = regAux & 0x00000003; 
					 *(p  + 1) = (regAux>>2) & 0x00000003;
					 *(p  + 2) = (regAux>>4) & 0x00000003;
					 *(p  + 3) = (regAux>>6) & 0x00000003;
					 *(p  + 4) = (regAux>>8) & 0x00000003;
					 *(p  + 5) = (regAux>>10) & 0x00000003;
					 *(p  + 6) = (regAux>>12) & 0x00000003;
					 *(p  + 7) = (regAux>>14) & 0x00000003;
					 *(p  + 8) = (regAux>>16) & 0x00000003;
					 *(p  + 9) = (regAux>>18) & 0x00000003;
					 *(p  + 10) = (regAux>>20) & 0x00000003;
					 *(p  + 11) = (regAux>>22) & 0x00000003;
					 *(p  + 12) = (regAux>>24) & 0x00000003;
					 *(p  + 13) = (regAux>>26) & 0x00000003; 
					 p += 14;
					 break;
			 case 9: *p  = 1; *(p  + 1) = 1; *(p  + 2) = 1; *(p  + 3) = 1;  
					 *(p  + 4) = 1; *(p  + 5) = 1; *(p  + 6) = 1; *(p  + 7) = 1;  
					 *(p  + 8) = 1; *(p  + 9) = 1; *(p  + 10) = 1; *(p  + 11) = 1;  
					 *(p  + 12) = 1; *(p  + 13) = 1; *(p  + 14) = 1; *(p  + 15) = 1;  
					 *(p  + 16) = 1; *(p  + 17) = 1; *(p  + 18) = 1; *(p  + 19) = 1;  
					 *(p  + 20) = 1; *(p  + 21) = 1; *(p  + 22) = 1; *(p  + 23) = 1;  
					 *(p  + 24) = 1; *(p  + 25) = 1; *(p  + 26) = 1; *(p  + 27) = 1;  
					 p += 28;
					 break;
		  }
		}
 }


unsigned int Simple9::next(unsigned int docId){
//	cout<<"Simple9::next - inicio (desired: "<<docId<<")\n";
	unsigned int curBlock;
	
	if(docId <= curDocId){
		return curDocId;
	}
	
	if (docId > lastDocId) {
		cur_pos = nnums;
		curDocId = END_OF_STREAM;
		return curDocId;
	}
	
	for (curBlock = lastBlock+(lastBlock==(unsigned int)(-1)); block[curBlock].getlastDocIdBlock() < docId; ++curBlock) {
//		cout<<"Simple9::next - Avance de bloque ("<<block[curBlock].getlastDocIdBlock()<<" < desired)\n";
//		cur_pos += block[curBlock].getnumsInBlock();
		cur_pos = docs_block_acum[curBlock];
		curDocId = block[curBlock].getlastDocIdBlock();
	}
	
	if (curBlock != lastBlock) {
		// block is uncompressed to the buffer
//		cout<<"Simple9::next - llenando buffer\n";
		decodeToBuffer(block[curBlock].getStartBlock(), block[curBlock+1].getStartBlock()); 	
		lastBlock = curBlock;
		curPosBuffer = 0;	
	}
	
//	cout<<"Simple9::next - avance secuencial (curDocId "<<curDocId<<")\n";
	for (; curDocId < docId; ++curPosBuffer) {
		curDocId+= buffer[curPosBuffer];
		++cur_pos;
	}
	
//	cout<<"Simple9::next - curDocId final "<<curDocId<<"\n";
	return curDocId;
 
 }
  
void Simple9::save(fstream *writer){
	//
	// simple data 
	writer->write((char *)&firstDocId, sizeof(int));
	writer->write((char *)&lastDocId, sizeof(int));
	writer->write((char *)&nBlocks, sizeof(int));
	writer->write((char *)&nnums, sizeof(int));
	writer->write((char *)&nWords, sizeof(int));
	//~ writer->write((char *)&curDocId, sizeof(int));
	//~ writer->write((char *)&lastBlock, sizeof(int));
	//~ writer->write((char *)&cur_pos, sizeof(int));
	//~ writer->write((char *)&curPosBuffer, sizeof(char));
	//~ writer->write((char *)&curPosRun, sizeof(int));
	
	//
	// stream data
	writer->write((char *)seq, nWords * sizeof(int));
	//writer->write((char *)buffer, 200 * sizeof(int)); // OJO el 200 va relacionado con el tamanio estatico que le pusieron a buffer.
	
	for(unsigned int i = 0; i < nBlocks + 1; ++i){
		block[i].save(writer);
	}
}

void Simple9::load(fstream *saver){
	//
	// simple data 
	saver->read((char *)&firstDocId, sizeof(int));
	saver->read((char *)&lastDocId, sizeof(int));
	saver->read((char *)&nBlocks, sizeof(int));
	saver->read((char *)&nnums, sizeof(int));
	saver->read((char *)&nWords, sizeof(int));
	//~ saver->read((char *)&curDocId, sizeof(int));
	//~ saver->read((char *)&lastBlock, sizeof(int));
	//~ saver->read((char *)&cur_pos, sizeof(int));
	//~ saver->read((char *)&curPosBuffer, sizeof(char));
	//~ saver->read((char *)&curPosRun, sizeof(int));
	
	//
	// stream data
	seq = new unsigned int[nWords];
	saver->read((char *)seq, nWords * sizeof(int));
	//saver->read((char *)buffer, 200 * sizeof(int)); // OJO el 200 va relacionado con el tamanio estatico que le pusieron a buffer.
	
	block = new Simple9BlockHeader[nBlocks + 1];
	for(unsigned int i = 0; i < nBlocks + 1; ++i){
		block[i].load(saver);
	}
	
	//n_docs acumulado por bloque
	docs_block_acum=new unsigned int[nBlocks+1];
	for(unsigned int i=0; i<nBlocks; ++i){
		docs_block_acum[i] = block[i].getnumsInBlock();
	}
	for(unsigned int i=1; i<nBlocks; ++i){
		docs_block_acum[i] += docs_block_acum[i-1];
	}
	
	reset();
}

bool Simple9::hasNext(){
	if(curDocId >= lastDocId){
		return false;
	}
	else{
		return true;
	}
}

unsigned int Simple9::next(){
	return next(curDocId + 1);
}



