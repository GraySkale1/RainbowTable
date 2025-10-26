#include<stdio.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>

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

uint32_t right_rotate(uint32_t value, size_t shift) {
	return (value >> shift) | ( value << (32 - shift) );
}

void compress(uint32_t *block, uint32_t *digest) {
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
					
		ch = (e & f) ^ ((~e) & g);
		
		temp1 = h + s1 + ch + CUBE_ROOTS[i] + block[i];
		
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

	digest[0] += a;
	digest[1] += b;
	digest[2] += c;
	digest[3] += d;
	digest[4] += e;
	digest[5] += f;
	digest[6] += g;
	digest[7] += h;
}


void iterate(unsigned char *char_block) {
	uint32_t temp0;
    uint32_t temp1;
    uint32_t cur_int;

	uint32_t *block = (uint32_t *)char_block;
	printf("after: %p\n", char_block);

    for (int i = 16; i < 64; i++) {
        cur_int = block[i-15];
        temp0 = right_rotate(cur_int, 7) ^ right_rotate(cur_int, 18) ^ (cur_int >> 3);

        cur_int = block[i-2];
        temp1 = right_rotate(cur_int, 17) ^ right_rotate(cur_int, 19) ^ (cur_int >> 10);


        block[i] = block[i-16] + temp0 + block[i-7] + temp1;

        printf("block[%d]: %x\n", i, block[i]);
    }

    // Debug: Print the entire block at the end
    for (int i = 0; i < 64; i++) {
		if (i % 2 == 0) {printf("\n");}
        printf("%x", block[i]);
    }
	printf("\n%p\n", block);
} 

void sha_chunk(unsigned char *chunk, uint32_t *digest) {
	unsigned char *padded_chunk = calloc(64, 32);

	strncpy(padded_chunk, chunk, 64);
	iterate(padded_chunk);
	compress((uint32_t *)padded_chunk, digest);
	free(padded_chunk);

}

void final_sha(unsigned char *chunks, size_t total_length, uint32_t *digest) {
	size_t input_length = strlen(chunks);
	unsigned char *padded_chunk = calloc(64, 32);

	strncpy(padded_chunk, chunks, input_length);
	padded_chunk[input_length] = 0x80;

	if (input_length < 56) { // This is the final chunk
		padded_chunk[56] = (int64_t) (strlen(chunks) * 8);

		for (int i = 0; i < 64; i++) {
			//places each element in big-endian
			padded_chunk[i*4] = (unsigned char)htonl((uint32_t)padded_chunk[i]);
		}

		for (int i=0; i < 64; i++) {
			if (i % 8 == 0) {printf("\n");}
			printf("%02x", padded_chunk[i]);
		}
		printf("\n");

		printf("before: %p\n", padded_chunk);
		iterate(padded_chunk);

		for (int i=0; i < 64*4; i++) {
			if (i % 8 == 0) {printf("\n");}
			printf("%02x", padded_chunk[i]);
		}
		printf("\n");

		compress((uint32_t *)padded_chunk, digest);
		free(padded_chunk);
	}
	else { // There is an extra chunk that is mostly empty
		unsigned char *extra_chunk = calloc(64, 32);
		strncpy(extra_chunk, padded_chunk + 64, 64); //needs optimisation
		memset(padded_chunk + 64, '\0', 64);

		for (int i = 0; i < 64; i++) {
			//places each element in big-endian
			padded_chunk[i*4] = (unsigned char)htonl((uint32_t)padded_chunk[i]);
		}

		iterate(padded_chunk);
		compress((uint32_t *)padded_chunk, digest);
		free(padded_chunk);

		for (int i = 0; i < 64; i++) {
			//places each element in big-endian
			extra_chunk[i*4] = (unsigned char)htonl((uint32_t)extra_chunk[i]);
		}

		iterate(extra_chunk);
		compress((uint32_t *)extra_chunk, digest);
		free(extra_chunk);
	}
	
}

int main(int argc, char *argv[]) {
	unsigned char chunk[64];
	size_t cpy_size;
	uint32_t digest[] = {SQRT0, SQRT1, SQRT2, SQRT3, SQRT4, SQRT5, SQRT6, SQRT7};

	int j = 0;
	for (int i=64; i<strlen(argv[1])+9; i = i + 64){
		cpy_size = i + 64 < strlen(argv[1]) ? 64: strlen(argv[1]) - 64;
		strncpy(chunk, argv[1] + i, cpy_size);

		printf("%s\n", chunk);

		sha_chunk(chunk, digest);
		memset(chunk, '\0', 64);
		j = i;
	}
	final_sha(argv[1]+j, strlen(argv[1]), digest);

	for (int i=0; i < 8; i++) {
		printf("%x", digest[i]);
	}
	printf("\n");
	
	return 0;
}
