#if !defined(____PRISM_TABLES__H____)
#define ____PRISM_TABLES__H____

#include "prism.h"

#if defined(STORE_PRIME)

#include <algorithm>

extern const bits_t PRISM_BITS[256];
extern const bits_t PRISM_MASK[256];
extern const prime_t PRISM_PRIME[256];
extern const prime_t PRISM_ORDERED_PRIME[256];
extern const rank_t PRISM_RANK_OF_ORDERED_PRIME[256];
extern const card_t PRISM_CARD[256];
extern const prime_t PRISM_GDCS[256][256];

#define BITS_BY(rank) PRISM_BITS[(rank)]
#define MASK_BY(rank) PRISM_MASK[(rank)]
#define PRIME_BY(rank) PRISM_PRIME[(rank)]
#define CARD_BY(rank) PRISM_CARD[(rank)]
#define GDC_BY(rank1, rank2) PRISM_GDCS[(rank1)][(rank2)]

#if defined(_DEBUG)
rank_t compute_rank_from_prime(prime_t prime);
#define RANK_OF compute_rank_from_prime
#else /* of #if defined(_DEBUG) */
	#if defined(USE_LINEAR_LOOKUP)
		#define RANK_OF(prime) ((rank_t)(find(PRISM_PRIME, PRISM_PRIME + 256, (prime)) - PRISM_PRIME))
	#else /* of #if defined(USE_LINEAR_LOOKUP) */
		#define RANK_OF(prime) PRISM_RANK_OF_ORDERED_PRIME[lower_bound(PRISM_ORDERED_PRIME, PRISM_ORDERED_PRIME + 256, (prime)) - PRISM_ORDERED_PRIME]
	#endif /* of #if defined(USE_LINEAR_LOOKUP) */
#endif /* of #i defined(_DEBUG) */

#define BITS_OF(prime) BITS_BY(RANK_OF(prime))
#define MASK_OF(prime) PRIME_BY(MASK_BY(RANK_OF(prime)))
#define CARD_OF(prime) CARD_BY(RANK_OF(prime))
#define GDC_OF(prime1, prime2) GDC_BY(RANK_OF(prime1), RANK_OF(prime2))

#endif /* of #if defined(STORE_PRIME) */

#endif /* of #if !defined(____PRISM_TABLES__H____) */
