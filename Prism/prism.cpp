#include "prism.h"

#include <algorithm>
#include <cassert>

/*	10000000;	01000000;	00100000;	00010000;	00001000;	00000100;	00000010;	00000001	*/
/*	0x80(128);	0x40(64);	0x20(32);	0x10(16);	0x08(8);	0x04(4);	0x02(2);	0x01(1)		*/

prime_t compute_prime_from_bits(bits_t bits) {
	prime_t value = 1;

	if (bits & 0x80) value *= 2;
	if (bits & 0x40) value *= 3;
	if (bits & 0x20) value *= 5;
	if (bits & 0x10) value *= 7;
	if (bits & 0x08) value *= 11;
	if (bits & 0x04) value *= 13;
	if (bits & 0x02) value *= 17;
	if (bits & 0x01) value *= 19;

	return value;
}

bits_t compute_bits_from_prime(prime_t prime) {
	bits_t value = 0;

	if ((prime % 19) == 0) {
		value |= 0x01;
		prime = prime / 19;
	}

	if ((prime % 17) == 0) {
		value |= 0x02;
		prime = prime / 17;
	}

	if ((prime % 13) == 0) {
		value |= 0x04;
		prime = prime / 13;
	}

	if ((prime % 11) == 0) {
		value |= 0x08;
		prime = prime / 11;
	}

	if ((prime % 7) == 0) {
		value |= 0x10;
		prime = prime / 7;
	}

	if ((prime % 5) == 0) {
		value |= 0x20;
		prime = prime / 5;
	}

	if ((prime % 3) == 0) {
		value |= 0x40;
		prime = prime / 3;
	}

	if ((prime % 2) == 0) {
		value |= 0x80;
		prime = prime / 2;
	}

	assert(prime == 1);
	
	return value;
}



bits_t compute_mask_from_bits(bits_t bits) {
	bits_t mask = 0x0; /* 0000 0000 */

	if (bits & 0x80) mask = 0x7F;		/* 0111 1111 (127)	*/
	else if (bits & 0x40) mask = 0x3F;	/* 0011 1111 (63)	*/
	else if (bits & 0x20) mask = 0x1F;	/* 0001 1111 (31)	*/
	else if (bits & 0x10) mask = 0x0F;	/* 0000 1111 (15)	*/
	else if (bits & 0x08) mask = 0x07;	/* 0000 0111 (7)	*/
	else if (bits & 0x04) mask = 0x3;	/* 0000 0011 (3)	*/
	else if (bits & 0x02) mask = 0x1;	/* 0000 0001 (1)	*/
	else if (bits & 0x01) mask = 0x0;	/* 0000 0000 (0)	*/

	return mask;
}

card_t compute_card_from_bits(bits_t bits) {
	card_t card = 0;

	if (bits & 0x80) card ++;
	if (bits & 0x40) card ++;
	if (bits & 0x20) card ++;
	if (bits & 0x10) card ++;
	if (bits & 0x08) card ++;
	if (bits & 0x04) card ++;
	if (bits & 0x02) card ++;
	if (bits & 0x01) card ++;

	return card;
}
