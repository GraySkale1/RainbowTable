#include<stdio.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>

#define BLOCK_SIZE 64

//Strange magic numbers woooo~
uint32_t CUBE_ROOTS[] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

uint32_t SQRT0 = 0x6a09e667;
uint32_t SQRT1 = 0xbb67ae85;
uint32_t SQRT2 = 0x3c6ef372;
uint32_t SQRT3 = 0xa54ff53a;
uint32_t SQRT4 = 0x510e527f;
uint32_t SQRT5 = 0x9b05688c;
uint32_t SQRT6 = 0x1f83d9ab;
uint32_t SQRT7 = 0x5be0cd19;

uint32_t right_rotate(uint32_t value, size_t shift) {
	return (value >> shift) | ( value << (32 - shift) );
}

uint32_t *_padding(unsigned char *string) {

	size_t str_length = strlen(string);
		
	//Padding will never be zero because as a single '1' must be inserted after string
	size_t padding = BLOCK_SIZE - (str_length % BLOCK_SIZE);
	
	unsigned char *padded_string = calloc(padding + str_length + 8*48, sizeof(unsigned char));
	assert(padded_string != NULL);

	printf("With padding, string is now %ld bits\n", (padding + str_length)*8);

	strncpy(padded_string, string, str_length);

	padded_string[padding + str_length - 1] = (str_length * 8);
	
	padded_string[str_length] = 0x80;

//	for (int i=0; i < (padding + str_length + 8*48); i++) {
//		if (!(i % 4) && (i != 0)) {printf("\n");}
//		printf("%02x", padded_string[i]);
//	}
//	printf("\n");
	
	uint32_t *padded_words = (uint32_t *) padded_string;

	for (int i = 0; i < BLOCK_SIZE; i++) {
		//places each element in big-endian
		padded_words[i] = htonl(padded_words[i]);
	}
	
	return padded_words;
}

void block_iterate(uint32_t *block) {
	uint32_t temp0;
	uint32_t temp1;
	uint32_t cur_int;
	for (int i = 16; i < 64; i++) {
		cur_int	= block[i-15];
		temp0 = right_rotate(cur_int, 7) ^ right_rotate(cur_int, 18) ^ (cur_int >> 3);

		cur_int	= block[i-2];
		temp1 = right_rotate(cur_int, 17) ^ right_rotate(cur_int, 19) ^ (cur_int >> 10);
		
		block[i] = block[i-16] + temp0 + block[i-7] + temp1;				
	}		
} 

uint32_t *block_compression(uint32_t *block) {
	uint32_t s1;
	uint32_t ch;
	uint32_t temp1;
	uint32_t s0;
	uint32_t maj;
	uint32_t temp2;


	uint32_t a = SQRT0;
	uint32_t b = SQRT1;
	uint32_t c = SQRT2;
	uint32_t d = SQRT3;
	uint32_t e = SQRT4;
	uint32_t f = SQRT5;
	uint32_t g = SQRT6;
	uint32_t h = SQRT7;

	for (int i=0; i < 64; i++) {
		s1 = right_rotate(e, 6) ^ right_rotate(e, 11) ^ right_rotate(e, 25);
		printf("s1: %x\n", s1);
			
		ch = (e & f) ^ ((~e) & g);
		printf("ch: %x\n", ch);

		temp1 = h + s1 + ch + CUBE_ROOTS[i] + block[i];
		printf("temp1: %x\n", temp1);

		s0 = right_rotate(a, 2) ^ right_rotate(a, 13) ^ right_rotate(a, 22);
		printf("s0: %x\n", s0);

		maj = (a & b) ^ (a & c) ^ (b & c);
		printf("maj: %x\n", maj);

		temp2 = s0 + maj;
		printf("temp2: %x\n", temp2);

		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
		printf("%x, %x, %x, %x, %x, %x, %x, %x\n",a,b,c,d,e,f,g,h);
	}	

	uint32_t *digest = malloc(8 * sizeof(uint32_t));
	assert(digest != NULL);

	digest[0] = SQRT0 + a;
	digest[1] = SQRT1 + b;
	digest[2] = SQRT2 + c;
	digest[3] = SQRT3 + d;
	digest[4] = SQRT4 + e;
	digest[5] = SQRT5 + f;
	digest[6] = SQRT6 + g;
	digest[7] = SQRT7 + h;

	return digest;
}

int main(int argc, char *argv[]) {
	char user_arg[64];

	if (argc > 1) {
		strncpy(user_arg, argv[1], 64);
		printf("%s\n", user_arg);
	}

	uint32_t *padded_block = _padding(user_arg); // [0-63]

	block_iterate(padded_block);	

	uint32_t *digest = block_compression(padded_block);
	free(padded_block);

	for (int i=0; i < 8; i++) {
		printf("%x", digest[i]);
	}
	printf("\n");
	free(digest);

	return 0;
}
