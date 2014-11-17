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
      size_t outputLength;
      char* mEncoded = base64_encode(&M_[0], M_.size(), &outputLength);
      sprintf(result, "%d|%s", b_, mEncoded);
      free(mEncoded);
      return result;
    }

    static SerializedHyperLogLog* fromString(const char* encoded) {
      if (encoded == NULL) return NULL;
      if (strlen(encoded) < 3) return NULL;
      if (strchr(encoded, '|') == NULL) return NULL;

      int m;
      char base64[10000];
      sscanf(encoded, "%d|%s", &m, base64);
      
      SerializedHyperLogLog* result = new SerializedHyperLogLog(m);

      size_t outputLength;
      unsigned char* decoded = base64_decode(base64, strlen(base64), &outputLength);

      if (decoded == NULL) return NULL;

      for(int i = 0; i < outputLength; ++i) {
         result->M_[i] = decoded[i];
      }

      free(decoded);
      
      return result;
    }

};

#endif
