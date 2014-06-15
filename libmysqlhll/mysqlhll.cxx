#include <mysql.h>

#include "SerializedHyperLogLog.hpp"

extern "C" {

#define LOG(...) fprintf(stderr, __VA_ARGS__);

my_bool hll_create_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void hll_create_deinit(UDF_INIT *initid);
char *hll_create(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error);
void hll_create_clear(UDF_INIT* initid, char* is_null, char* message);
void hll_create_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message);

my_bool hll_compute_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void hll_compute_deinit(UDF_INIT *initid);
long long hll_compute(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error);
void hll_compute_clear(UDF_INIT* initid, char* is_null, char* message);
void hll_compute_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message);

class Data {
  public: 
    SerializedHyperLogLog* shll;
    char* result;

    Data(bool need_result) {
      shll = new SerializedHyperLogLog(12);

      if (need_result) {
        result = (char*)malloc(10000);
      } else {
        result = NULL;
      }
    }

    ~Data() {
      delete shll;
      if (result != NULL) {
        free(result);
      }
    }
};

my_bool init(UDF_INIT *initid, UDF_ARGS *args, char *message, bool need_result) {
  if (args->arg_count == 0) {
    strcpy(message,"Wrong arguments to HLL();  Must have at least 1 argument");
    return 1;
  }

  for(int i = 0; i < args->arg_count; ++i) {
    args->arg_type[i] = STRING_RESULT;
  }

  initid->ptr = (char*)new Data(need_result);
  return 0;
}

my_bool hll_create_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  return init(initid, args, message, true);
}

Data* data(UDF_INIT *initid) {
  return (Data*)initid->ptr;
}

SerializedHyperLogLog* shll(UDF_INIT *initid) {
  return data(initid)->shll;
}

void hll_create_deinit(UDF_INIT *initid) {
  delete data(initid);
}

char *hll_create(UDF_INIT *initid, UDF_ARGS *args, char *result, 
          unsigned long *length, char *is_null, char *error) {

  char* hll_result = data(initid)->result;
  shll(initid)->toString(hll_result);
  *length = strlen(hll_result);

  LOG("hll %s\n", hll_result);

  return hll_result;
}

void hll_create_clear(UDF_INIT* initid, char* is_null, char* message) {
  LOG("hll clear\n");
  shll(initid)->clear();
}

void hll_create_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message) {
  for(int i = 0; i < args->arg_count; ++i) {
    LOG("hll_add %.*s %d\n", (int)args->lengths[i], args->args[i], (int)args->lengths[i]);

    shll(initid)->add(args->args[i], args->lengths[i]);
  }
}

my_bool hll_compute_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  return init(initid, args, message, false);
}

void hll_compute_deinit(UDF_INIT *initid) {
  return hll_create_deinit(initid);
}

long long hll_compute(UDF_INIT *initid, UDF_ARGS *args, char *result,
          unsigned long *length, char *is_null, char *error) {
  LOG("hll_compute\n");
  return shll(initid)->estimate();
}

void hll_compute_clear(UDF_INIT* initid, char* is_null, char* message) {
  LOG("hll_compute_clear\n");
  hll_create_clear(initid, is_null, message);
}

void hll_compute_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message) {
  LOG("hll_compute_add\n");
  hll_create_add(initid, args, is_null, message);
}

}
