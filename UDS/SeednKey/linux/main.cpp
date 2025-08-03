#include <iostream>
#include <dlfcn.h>
#include "seednkey.h"

int main(void)
{
	void* handle = dlopen("./libseednkey.so", RTLD_LAZY);
	if (!handle) {
		std::cerr << "Failed to load library: " << dlerror() << std::endl;
		return 1;
	}

	// Clear any existing errors
	dlerror();

	GenerateKeyExOptFunc GenerateKeyExOpt = (GenerateKeyExOptFunc)dlsym(handle, "GenerateKeyExOpt"); // Try original name

	if (GenerateKeyExOpt == nullptr) {
		std::cerr << "Failed to find function: " << dlerror() << std::endl;
		dlclose(handle);
		return 1;
	}

	unsigned char seedArray[] = { 0x01, 0x02, 0x03, 0x04 };
	unsigned char keyArray[64];
	unsigned int actualKeySize = 0;
	VKeyGenResultExOpt result = GenerateKeyExOpt(
		seedArray,
		sizeof(seedArray),
		3, // Security level
		"variant",
		"options",
		keyArray,
		sizeof(keyArray),
		&actualKeySize
	);

	if (result == KGREO_Ok) {
		std::cout << "Key generated successfully. Actual key size: " << actualKeySize << std::endl;
		for (unsigned int i = 0; i < actualKeySize; ++i) {
			std::cout << std::hex << static_cast<int>(keyArray[i]) << " ";
		}
		std::cout << std::dec << std::endl; // Reset to decimal output
	}

	if (result != KGREO_Ok) {
		std::cerr << "Key generation failed with error code: " << result << std::endl;
		dlclose(handle);
		return 1;
	}

	dlclose(handle);
	return 0;
}