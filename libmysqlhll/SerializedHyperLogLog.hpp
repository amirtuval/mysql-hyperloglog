#if !defined(SERIALIZED_HYPERLOGLOG_HPP)
#define SERIALIZED_HYPERLOGLOG_HPP

#include <stdio.h>
#include <string.h>

#include "hyperloglog.hpp"
#include "base64.h"

class SerializedHyperLogLog : public hll::HyperLogLog {

public:
    SerializedHyperLogLog(uint8_t b, bool legacyMode=true) : HyperLogLog(b, legacyMode) {
    }

    char* toString(char* result) {
      sprintf(result, "%d|%d|bin_", (legacyMode_ ? 1 : 0), b_);
      memcpy(&result[strlen(result)], &M_[0], M_.size());
      return result;
    }

    int stringLength() {
      return 10 + M_.size();
    }

    static SerializedHyperLogLog* fromString(const char* encoded) {
      if (encoded == NULL) return NULL;
      const char* firstSep = (const char*)memchr(encoded, '|', 4);
      if (firstSep == NULL) return NULL;

      int m;
      const char* data;
      bool legacyMode;

      const char* secondSep = (const char*) memchr(&firstSep[1], '|', 4);
      if (secondSep == NULL) { // check if string has 2 '|'
        sscanf(encoded, "%d|", &m);
        data = &firstSep[1];
        legacyMode = true;
      } else {
        int legacyModeInt;
        sscanf(encoded, "%d|%d|", &legacyModeInt, &m);
        legacyMode = legacyModeInt == 0 ? false : true;
        data = &secondSep[1];
      }
      
      SerializedHyperLogLog* result = new SerializedHyperLogLog(m, legacyMode);

      const unsigned char* decoded;
      bool freeDecoded;
      if (strncmp(data, "bin_", 4) == 0) {
        decoded = (const unsigned char*)&data[4];
        freeDecoded = false;
      } else {
        size_t outputLength;
        decoded = base64_decode(data, strlen(data), &outputLength);
        freeDecoded = true;
      }

      if (decoded == NULL) return NULL;

      memcpy(&result->M_[0], decoded, result->M_.size());

      if (freeDecoded)
        free((void*)decoded);
      
      return result;
    }

};

#endif
