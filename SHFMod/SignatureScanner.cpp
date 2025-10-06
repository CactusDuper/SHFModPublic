#include "SignatureScanner.hpp"

#include <Windows.h>
#include <processthreadsapi.h>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <Psapi.h>

std::vector<int32_t> SignatureScanner::ParseSignatureStringToBytes(const std::string& signature) {
    std::vector<int32_t> signatureBytes;
    const char* currentSigChar = signature.data();
    const char* sigEnd = currentSigChar + signature.size();

    while (currentSigChar < sigEnd) {
        if (*currentSigChar == '?') {
            currentSigChar++;
            if (currentSigChar < sigEnd && *currentSigChar == '?') {
                currentSigChar++;
            }
            signatureBytes.push_back(-1);
        }
        else if (isxdigit(*currentSigChar)) {
            signatureBytes.push_back(strtoul(currentSigChar, const_cast<char**>(&currentSigChar), 16));
        }
        else {
            currentSigChar++;
        }
    }
    return signatureBytes;
}

std::vector<void*> SignatureScanner::ScanMemory(void* scanRegionStart, size_t scanRegionSize, const std::string& signature, uint32_t maxResults) {
    std::vector<void*> foundAddresses;
    std::vector<int32_t> signatureBytes = ParseSignatureStringToBytes(signature);
    const uint8_t* regionBytes = static_cast<uint8_t*>(scanRegionStart);
    const size_t signatureSize = signatureBytes.size();

    if (scanRegionSize < signatureSize) {
        return foundAddresses;
    }

    uint32_t resultsCount = 0;
    for (size_t i = 0; i <= scanRegionSize - signatureSize; ++i) {
        if (maxResults != 0 && resultsCount >= maxResults) {
            break;
        }

        bool matchFound = true;
        for (size_t j = 0; j < signatureSize; ++j) {
            if (signatureBytes[j] != -1 && regionBytes[i + j] != static_cast<uint8_t>(signatureBytes[j])) {
                matchFound = false;
                break; // Byte mismatch
            }
        }

        if (matchFound) {
            foundAddresses.push_back(const_cast<void*>(static_cast<const void*>(&regionBytes[i])));
            resultsCount++;
        }
    }
    return foundAddresses;
}

std::vector<void*> SignatureScanner::Scan(const std::string& signature, uint32_t maxResults) {
    static HMODULE moduleHandle = GetModuleHandle(nullptr);
    static MODULEINFO moduleInfo = { 0 };
    static bool moduleInfoInitialized = false;

    if (!moduleInfoInitialized) {
        if (moduleHandle != nullptr) {
            GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(moduleInfo));
            moduleInfoInitialized = true;
        }
        else {
            return {};
        }
    }
    return ScanMemory(moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage, signature, maxResults);
}

uint32_t SignatureScanner::FindOffsetByPattern(const std::string pattern, int offsetOffset, int ripRelativeOffset, uint64_t gameBase) {
    std::vector<void*> addresses = Scan(pattern, 1);
    if (addresses.empty() || addresses[0] == nullptr) {
        std::cerr << "Pattern scan failed for pattern: " << pattern << "\n";
        return 0;
    }

    uintptr_t patternAddress = reinterpret_cast<uintptr_t>(addresses[0]);
    uintptr_t rva = patternAddress - gameBase;
    uint32_t offsetBase = *reinterpret_cast<uint32_t*>(patternAddress + offsetOffset);
    return offsetBase + rva + ripRelativeOffset;
}

uintptr_t SignatureScanner::ResolveRipRelativeAddress(const std::string& signature, int32_t offsetToInstruction, int32_t instructionLength) {
    std::vector<void*> addresses = Scan(signature, 1);
    if (addresses.empty() || addresses[0] == nullptr) {
        std::cerr << "Pattern scan failed for signature: " << signature << std::endl;
        return 0;
    }

    uintptr_t patternAddress = reinterpret_cast<uintptr_t>(addresses[0]);
    uintptr_t instructionAddress = patternAddress + offsetToInstruction;
    int32_t relativeOffset = *reinterpret_cast<int32_t*>(instructionAddress + (instructionLength - 4));
    uintptr_t nextInstructionAddress = instructionAddress + instructionLength;
    return nextInstructionAddress + relativeOffset;
}
