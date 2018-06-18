#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "wasm.h"

#define own

// A function to be called from Wasm code.
own wasm_result_t fail_callback(wasm_val_vec_t args) {
  printf("Calling back...\n");
  return wasm_result_new_trap("callback abort");
}


int main(int argc, const char* argv[]) {
  // Initialize.
  printf("Initializing...\n");
  wasm_engine_t* engine = wasm_engine_new(argc, argv);
  wasm_store_t* store = wasm_store_new(engine);

  // Load binary.
  printf("Loading binary...\n");
  FILE* file = fopen("trap.wasm", "r");
  if (!file) {
    printf("> Error loading module!\n");
    return 1;
  }
  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0L, SEEK_SET);
  wasm_byte_vec_t binary = wasm_byte_vec_new_uninitialized(file_size);
  fread(binary.data, file_size, 1, file);
  fclose(file);

  // Compile.
  printf("Compiling module...\n");
  own wasm_module_t* module = wasm_module_new(store, binary);
  if (!module) {
    printf("> Error compiling module!\n");
    return 1;
  }

  wasm_byte_vec_delete(binary);

  // Create external print functions.
  printf("Creating callback...\n");
  own wasm_functype_t* fail_type = wasm_functype_new_0_1(wasm_valtype_new_i32());
  own wasm_func_t* fail_func = wasm_func_new(store, fail_type, fail_callback);

  wasm_functype_delete(fail_type);

  // Instantiate.
  printf("Instantiating module...\n");
  const wasm_extern_t* imports[] = { wasm_func_as_extern(fail_func) };
  own wasm_instance_t* instance = wasm_instance_new(store, module, wasm_extern_vec_const(2, imports));
  if (!instance) {
    printf("> Error instantiating module!\n");
    return 1;
  }

  // Extract export.
  printf("Extracting exports...\n");
  own wasm_extern_vec_t exports = wasm_instance_exports(instance);
  if (exports.size < 2) {
    printf("> Error accessing exports!\n");
    return 1;
  }

  wasm_module_delete(module);
  wasm_instance_delete(instance);

  // Call.
  for (int i = 0; i < 2; ++i) {
    const wasm_func_t* func = wasm_extern_as_func(exports.data[i]);
    if (func == NULL) {
      printf("> Error accessing export!\n");
      return 1;
    }

    printf("Calling export %d...\n", i);
    own wasm_result_t result = wasm_func_call(func, wasm_val_vec_empty());
    if (result.kind != WASM_TRAP) {
      printf("> Error calling function!\n");
      return 1;
    }

    printf("Printing message...\n");
    printf("> %s\n", result.of.trap.data);
    wasm_result_delete(result);
  }

  wasm_extern_vec_delete(exports);

  // Shut down.
  printf("Shutting down...\n");
  wasm_store_delete(store);
  wasm_engine_delete(engine);

  // All done.
  printf("Done.\n");
  return 0;
}
