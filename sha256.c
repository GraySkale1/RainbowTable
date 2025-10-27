#include<stdio.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>

#define BLOCK_SIZE 64

#define SQRT0 0x6a09e667
#define SQRT1 0xbb67ae85
#define SQRT2 0x3c6ef372
#define SQRT3 0xa54ff53a
#define SQRT4 0x510e527f
#define SQRT5 0x9b05688c
#define SQRT6 0x1f83d9ab
#define SQRT7 0x5be0cd19



//Strange magic numbers woooo~
const uint32_t CUBE_ROOTS[] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

uint32_t digest[] = {SQRT0, SQRT1, SQRT2, SQRT3, SQRT4, SQRT5, SQRT6, SQRT7};

typedef union {
	unsigned char bytes[64*4];
	uint32_t words[64];
} chunk;

uint32_t right_rotate(uint32_t value, size_t shift) {
	return (value >> shift) | ( value << (32 - shift) );
}

int ceil_div(int x, int y) {
	return x/y + (x % y != 0);
}

void print_chunk(chunk *input, int big_endian) {
	if (big_endian) {
		uint32_t little_endian[64];
		for (int i=0; i<64; i++) {
			little_endian[i] = ntohl(input -> words[i]);
		}

		unsigned char *output = (unsigned char *)little_endian;
		for (int i=0; i < 64*4; i++) {
			if (i % 4 == 0) {printf(" ");}
			if (i % 8 == 0) {printf("\n");}
			printf("%02x", output[i]);
		}
		printf("\n");
	}
	else {
		for (int i=0; i < 64*4; i++) {
			if (i % 4 == 0) {printf(" ");}
			if (i % 8 == 0) {printf("\n");}
			printf("%02x", input -> bytes[i]);
		}
		printf("\n");
	}


}

void compress(chunk *block) {
	uint32_t s1; uint32_t ch;
	uint32_t temp1;
	uint32_t s0;
	uint32_t maj;
	uint32_t temp2;

	uint32_t a = digest[0];
	uint32_t b = digest[1];
	uint32_t c = digest[2];
	uint32_t d = digest[3];
	uint32_t e = digest[4];
	uint32_t f = digest[5];
	uint32_t g = digest[6];
	uint32_t h = digest[7];

	for (int i=0; i < 64; i++) {
		s1 = right_rotate(e, 6) ^ right_rotate(e, 11) ^ right_rotate(e, 25);
					
		ch = (e & f) ^ ((~e) & g);
		
		temp1 = h + s1 + ch + CUBE_ROOTS[i] + block -> words[i];
		
		s0 = right_rotate(a, 2) ^ right_rotate(a, 13) ^ right_rotate(a, 22);
		
		maj = (a & b) ^ (a & c) ^ (b & c);
		
		temp2 = s0 + maj;
		
		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}	

	//printf("before: %x\n", digest[0]);

	digest[0] += a;
	digest[1] += b;
	digest[2] += c;
	digest[3] += d;
	digest[4] += e;
	digest[5] += f;
	digest[6] += g;
	digest[7] += h;

	//printf("after: %x\n", digest[0]);
}

void iterate(chunk *padding) {
	uint32_t temp0;
	uint32_t temp1;
	uint32_t cur_int;
	for (int i=0; i<64; i++) {
		padding -> words[i] = htonl(padding -> words[i]);
	}
	//print_chunk(padding, 0);
	
	for (int i = 16; i < 64; i++) {
		cur_int	= padding -> words[i-15];
		temp0 = right_rotate(cur_int, 7) ^ right_rotate(cur_int, 18) ^ (cur_int >> 3);

		cur_int	= padding -> words[i-2];
		temp1 = right_rotate(cur_int, 17) ^ right_rotate(cur_int, 19) ^ (cur_int >> 10);
		
		padding -> words[i] = padding -> words[i-16] + temp0 + padding -> words[i-7] + temp1;				
	}		

	//print_chunk(padding, 0);
	compress(padding);

}

void start_cha(unsigned char input[], uint64_t total_bits) {

	chunk *padding = calloc(64, 32);
	assert(padding != NULL);

	strncpy(padding -> bytes, input, 64);


	if (strlen(input) == 64) { //not final chunk
		iterate(padding);
		free(padding);
	}
	else if (strlen(input) < 56) { //is final chunk and has room to pad
		padding -> bytes[strlen(input)] = 0x80;
		for (int i = 0; i < 8; i++) {
			padding -> bytes[63 - i] = (unsigned char)((total_bits) >> (i * 8) & 0xFF);
		}

		//printf("value in hex: %ld\n", total_bits);
		iterate(padding);
		free(padding);
	}
	else { //is less than 64 bytes but requires another chunk to fit ending stuff
		padding -> bytes[strlen(input)] = 0x80;
		for (int i = 0; i < 8; i++) {
			padding -> bytes[64 + (63 - i)] = (unsigned char)((total_bits) >> (i * 8) & 0xFF);
		}

		chunk *extra = calloc(64, 32);
		//print_chunk(padding, 0);
		memcpy(extra -> bytes, padding -> bytes + 64, 64);
		memset(padding -> bytes + 64*8, '\0', 64);

		//print_chunk(padding, 0);
		iterate(padding);
		free(padding);

		//print_chunk(extra, 0);
		iterate(extra);
		free(extra);
	}
}

int main(int argc, char *argv[]) {
	unsigned char plaintext[strlen(argv[1]) + 1];

	strncpy(plaintext, argv[1], strlen(argv[1]));
	plaintext[strlen(argv[1])] = '\0'; //marks the end of the string

	size_t chunks = ceil_div(strlen(plaintext) + 9, BLOCK_SIZE);
	//printf("chunks: %ld\n", chunks);

	unsigned char neutered_input64[64];

	for (int i=0; i < chunks; i++) {
		//printf("indexing %d in plaintext\n", i*64);
		size_t cpy_size = strlen(plaintext + i*64) < 64 ? strlen(plaintext + i*64) : 64;
		//printf("cpy_size = %ld\n", cpy_size);
		
		memcpy(neutered_input64, plaintext + i*64, cpy_size);

		//printf("%s\n", neutered_input64);

		start_cha( neutered_input64, strlen(plaintext)*8 );
		memset(neutered_input64, '\0', 64);
		if (cpy_size < 64) {
			break;
		}
	}
	for (int i=0; i < 8; i++) {
		printf("%x", digest[i]);
	}
	printf("\n");

	

	return 0;
}
