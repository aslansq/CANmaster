#include "seednkey.h"

extern "C" {

VKeyGenResultExOpt GenerateKeyEx(
    const unsigned char* ipSeedArray,
    unsigned int         iSeedArraySize,
    const unsigned int   iSecurityLevel,
    const char*          ipVariant,
    unsigned char*       iopKeyArray,
    unsigned int         iMaxKeyArraySize,
    unsigned int*        oActualKeyArraySize
)
{
    const char c = 'a';
    return GenerateKeyExOpt(
        ipSeedArray,
        iSeedArraySize,
        iSecurityLevel,
        ipVariant,
        &c,
        iopKeyArray,
        iMaxKeyArraySize,
        oActualKeyArraySize
    );
}

VKeyGenResultExOpt GenerateKeyExOpt(
	const unsigned char* ipSeedArray,
	unsigned int iSeedArraySize,
	const unsigned int iSecurityLevel,
	const char* ipVariant,
	const char* ipOptions,
	unsigned char* iopKeyArray,
	unsigned int iMaxKeyArraySize,
	unsigned int* oActualKeyArraySize
)
{
	VKeyGenResultExOpt result = KGREO_Ok;

	if(iMaxKeyArraySize < iSeedArraySize) {
		result = KGREO_BufferToSmall;
	} else if(iSecurityLevel < 0 || iSecurityLevel > 5) {
		result = KGREO_SecurityLevelInvalid;
	} else {
		// Simulate key generation logic
		for(unsigned int i = 0; i < iSeedArraySize && i < iMaxKeyArraySize; ++i) {
			iopKeyArray[i] = iSecurityLevel; // Example transformation
		}
		*oActualKeyArraySize = iSeedArraySize; // Set the actual size of the key
	}

	return result;
}

} // extern "C"