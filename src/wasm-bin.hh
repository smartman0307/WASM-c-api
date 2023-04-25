#ifndef __WASM_BIN_HH
#define __WASM_BIN_HH

#include "wasm.hh"
#include <tuple>

namespace wasm {
namespace bin {

auto imports_exports(vec<byte_t>& binary) ->
  std::tuple<vec<importtype*>, vec<exporttype*>>;

}  // namespace bin
}  // namespace wasm

#endif  // #ifdef __WASM_BIN_HH
