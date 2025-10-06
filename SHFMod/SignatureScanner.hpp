#pragma once

#include <cstdarg>
#include <cstdint>
#include <string>
#include <vector>

class SignatureScanner {
public:
    static std::vector<int32_t> ParseSignatureStringToBytes(const std::string& signature);

    std::vector<void*> ScanMemory(void* scanRegionStart, size_t scanRegionSize, const std::string& signature, uint32_t maxResults = 1);

    std::vector<void*> Scan(const std::string& signature, uint32_t maxResults = 1);

    uint32_t FindOffsetByPattern(const std::string pattern, int offsetOffset, int ripRelativeOffset, uint64_t gameBase);

    uintptr_t ResolveRipRelativeAddress(const std::string& signature, int32_t offsetToInstruction, int32_t instructionLength);
};