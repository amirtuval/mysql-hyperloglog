#ifdef WIN32
  #include <winsock.h>
  typedef signed char int8_t;
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#include <mysql.h>

#include "constants.hpp"
#include "SerializedHyperLogLog.hpp"

#define HLL_LEGACY_BIT_WIDTH 12

extern "C" {

#ifndef NDEBUG
 #define LOG(...) fprintf(stderr, __VA_ARGS__);
#else
 #define LOG(...)
#endif

my_bool EXPORT hll_create_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void EXPORT hll_create_deinit(UDF_INIT *initid);
char EXPORT *hll_create(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error);
void EXPORT hll_create_clear(UDF_INIT* initid, char* is_null, char* message);
void EXPORT hll_create_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message);

my_bool EXPORT hll_create_legacy_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void EXPORT hll_create_legacy_deinit(UDF_INIT *initid);
char EXPORT *hll_create_legacy(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error);
void EXPORT hll_create_legacy_clear(UDF_INIT* initid, char* is_null, char* message);
void EXPORT hll_create_legacy_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message);

my_bool EXPORT hll_compute_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void EXPORT hll_compute_deinit(UDF_INIT *initid);
long long EXPORT hll_compute(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error);
void EXPORT hll_compute_clear(UDF_INIT* initid, char* is_null, char* message);
void EXPORT hll_compute_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message);

my_bool EXPORT hll_merge_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void EXPORT hll_merge_deinit(UDF_INIT *initid);
char EXPORT *hll_merge(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error);
void EXPORT hll_merge_clear(UDF_INIT* initid, char* is_null, char* message);
void EXPORT hll_merge_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message);

my_bool EXPORT hll_merge_compute_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void EXPORT hll_merge_compute_deinit(UDF_INIT *initid);
long long EXPORT hll_merge_compute(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error);
void EXPORT hll_merge_compute_clear(UDF_INIT* initid, char* is_null, char* message);
void EXPORT hll_merge_compute_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message);

class Data {
  public: 
    SerializedHyperLogLog* shll;
    char* result;

    Data(bool need_result, SerializedHyperLogLog* hll) {
      init(need_result, hll);
    }

    Data(bool need_result, int bitWidth, bool legacyMode) {
      init(need_result, new SerializedHyperLogLog(bitWidth, legacyMode));
    }

    ~Data() {
      if (shll != NULL)
        delete shll;
      if (result != NULL) {
        free(result);
      }
    }

  private:
    void init(bool need_result, SerializedHyperLogLog* hll) {
      shll = hll;

      if (need_result) {
        result = (char*)malloc(std::pow(2,HLL_BIT_WIDTH));
      } else {
        result = NULL;
      }      
    }
};

my_bool init(UDF_INIT *initid, UDF_ARGS *args, char *message, bool need_result, int bitWidth, bool legacyMode, const char* function_name) {
  if (args->arg_count == 0) {
    sprintf(message, "Wrong arguments to %s();  Must have at least 1 argument", function_name);
    return 1;
  }

  for(int i = 0; i < args->arg_count; ++i) {
    args->arg_type[i] = STRING_RESULT;
  }

  initid->ptr = (char*)new Data(need_result, bitWidth, legacyMode);
  return 0;
}

my_bool EXPORT hll_create_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  return init(initid, args, message, true, HLL_BIT_WIDTH, false, "HLL_CREATE");
}

my_bool EXPORT hll_create_legacy_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  return init(initid, args, message, true, HLL_LEGACY_BIT_WIDTH, true, "HLL_CREATE_LEGACY");
}

Data* data(UDF_INIT *initid) {
  return (Data*)initid->ptr;
}

SerializedHyperLogLog* shll(UDF_INIT *initid) {
  return data(initid)->shll;
}

void EXPORT hll_create_deinit(UDF_INIT *initid) {
  delete data(initid);
}

void EXPORT hll_create_legacy_deinit(UDF_INIT *initid) {
  hll_create_deinit(initid);
}

char EXPORT *hll_create(UDF_INIT *initid, UDF_ARGS *args, char *result, 
          unsigned long *length, char *is_null, char *error) {

  char* hll_result = data(initid)->result;
  if (shll(initid) == NULL) {
    hll_result[0] = '\0';
    *length = 0;
  } else {
    shll(initid)->toString(hll_result);
    *length = shll(initid)->stringLength();
  }

  return hll_result;
}

char EXPORT *hll_create_legacy(UDF_INIT *initid, UDF_ARGS *args, char *result, 
          unsigned long *length, char *is_null, char *error) {
  return hll_create(initid, args, result, length, is_null, error);
}

void EXPORT hll_create_clear(UDF_INIT* initid, char* is_null, char* message) {
  if (shll(initid) != NULL)
    shll(initid)->clear();
}

void EXPORT hll_create_legacy_clear(UDF_INIT* initid, char* is_null, char* message) {
  hll_create_clear(initid, is_null, message);
}

void get_value_and_length(UDF_ARGS* args, int i, const char** value, uint32_t* length) {
    *value = (args->args[i] == NULL ? "" : args->args[i]);
    *length = (args->args[i] == NULL ? 0 : args->lengths[i]);
}

void EXPORT hll_create_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message) {
  for(int i = 0; i < args->arg_count; ++i) {
    const char* value;
    uint32_t length;
    get_value_and_length(args, i, &value, &length);
    shll(initid)->add(value, length);
  }
}

void EXPORT hll_create_legacy_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message) {
  hll_create_add(initid, args, is_null, message);
}

my_bool EXPORT hll_compute_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  return init(initid, args, message, false, HLL_BIT_WIDTH, false, "HLL_COMPUTE");
}

void EXPORT hll_compute_deinit(UDF_INIT *initid) {
  return hll_create_deinit(initid);
}

long long EXPORT hll_compute(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error) {
  return shll(initid)->estimate();
}

void EXPORT hll_compute_clear(UDF_INIT* initid, char* is_null, char* message) {
  hll_create_clear(initid, is_null, message);
}

void EXPORT hll_compute_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message) {
  hll_create_add(initid, args, is_null, message);
}

my_bool merge_init(UDF_INIT *initid, UDF_ARGS *args, char *message, bool need_result, const char* function_name) {
  if (args->arg_count == 0) {
    sprintf(message,"Wrong arguments to %s();  Must have at least 1 argument", function_name);
    return 1;
  }

  for(int i = 0; i < args->arg_count; ++i) {
    if (args->arg_type[i] != STRING_RESULT) {
      sprintf(message,"Wrong arguments to %s();  All arguments must be of type string", function_name);
      return 1;    
    }
  }

  initid->ptr = (char*)new Data(need_result, NULL);
  return 0;
}

my_bool EXPORT hll_merge_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  return merge_init(initid, args, message, true, "HLL_MERGE");
}

void EXPORT hll_merge_deinit(UDF_INIT *initid) {
  delete data(initid);
}

char EXPORT *hll_merge(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error) {
  return hll_create(initid, args, result, length, is_null, error);
}

void EXPORT hll_merge_clear(UDF_INIT* initid, char* is_null, char* message) {
  if (shll(initid) != NULL)
    shll(initid)->clear();
}

void EXPORT hll_merge_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message) {
  for(int i = 0; i < args->arg_count; ++i) {
    uint32_t length;
    const char* arg;
    get_value_and_length(args, i, &arg, &length);
    if (length == 0) continue; // NULL handling
    
    char* hll_str = (char*)malloc(length + 1);
    
    memcpy(hll_str, arg, length);
    hll_str[length] = '\0';

    SerializedHyperLogLog* current_shll = SerializedHyperLogLog::fromString(hll_str);
    free(hll_str);

    if (current_shll != NULL) {
      if (shll(initid) != NULL) {
        shll(initid)->merge(*current_shll);
        delete current_shll;
      } else {
        data(initid)-> shll = current_shll;
      }
    }
  }
}

my_bool EXPORT hll_merge_compute_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  return merge_init(initid, args, message, false, "HLL_MERGE_COMPUTE");
}

void EXPORT hll_merge_compute_deinit(UDF_INIT *initid) {
  hll_merge_deinit(initid);
}

long long EXPORT hll_merge_compute(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error) {
  if (shll(initid) == NULL) return 0;
  return shll(initid)->estimate();
}

void EXPORT hll_merge_compute_clear(UDF_INIT* initid, char* is_null, char* message) {
  hll_merge_clear(initid, is_null, message);
}

void EXPORT hll_merge_compute_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message) {
  hll_merge_add(initid, args, is_null, message);
}

}
