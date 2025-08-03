#ifndef SEED_N_KEY_H
#define SEED_N_KEY_H

extern "C" {

typedef enum
{
	KGREO_Ok = 0,
	KGREO_BufferToSmall = 1,
	KGREO_SecurityLevelInvalid = 2,
	KGREO_VariantInvalid = 3,
	KGREO_UnspecifiedError = 4
} VKeyGenResultExOpt;

typedef VKeyGenResultExOpt (*GenerateKeyExFunc)(
    const unsigned char* ipSeedArray,            // Array for the seed [in]
    unsigned int         iSeedArraySize,         // Length of the array for the seed [in]
    const unsigned int   iSecurityLevel,         // Security level [in]
    const char*          ipVariant,              // Name of the active variant [in]
    unsigned char*       iopKeyArray,            // Array for the key [in, out]
    unsigned int         iMaxKeyArraySize,       // Maximum length of the array for the key [in]
    unsigned int*        oActualKeyArraySize     // Length of the key [out]
);

VKeyGenResultExOpt GenerateKeyEx(
    const unsigned char* ipSeedArray,            // Array for the seed [in]
    unsigned int         iSeedArraySize,         // Length of the array for the seed [in]
    const unsigned int   iSecurityLevel,         // Security level [in]
    const char*          ipVariant,              // Name of the active variant [in]
    unsigned char*       iopKeyArray,            // Array for the key [in, out]
    unsigned int         iMaxKeyArraySize,       // Maximum length of the array for the key [in]
    unsigned int*        oActualKeyArraySize     // Length of the key [out]
);

typedef VKeyGenResultExOpt (*GenerateKeyExOptFunc)(
	const unsigned char* ipSeedArray,
	unsigned int iSeedArraySize,
	const unsigned int iSecurityLevel,
	const char* ipVariant,
	const char* ipOptions,
	unsigned char* iopKeyArray,
	unsigned int iMaxKeyArraySize,
	unsigned int* oActualKeyArraySize
);

// The client has to provide a keyArray buffer and has to transfer this buffer - 
// including its size - to the GenerateKey method. The method checks, if the size is
// sufficient. The client can discover the required size by examining the service used
// transfer the key to the ECU.
// Returns false if the key could not be generated:
//  -> keyArraySize to small
//  -> generation for specified security level not possible
//  -> variant unknown
VKeyGenResultExOpt GenerateKeyExOpt(
	const unsigned char* ipSeedArray,
	unsigned int iSeedArraySize,
	const unsigned int iSecurityLevel,
	const char* ipVariant,
	const char* ipOptions,
	unsigned char* iopKeyArray,
	unsigned int iMaxKeyArraySize,
	unsigned int* oActualKeyArraySize
);

} // extern "C"

#endif // SEED_N_KEY_H
