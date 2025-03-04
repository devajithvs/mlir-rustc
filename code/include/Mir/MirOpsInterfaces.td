#ifndef MirInterfaces
#define MirInterfaces

include "mlir/IR/OpBase.td"

class Mir_OpInterface<string name> : OpInterface<name> {
  let cppNamespace = "::rust_compiler::Mir";
}

def BorrowOpInterface : Mir_OpInterface<"BorrowOpInterface"> {
  let description = [{
    This is an example interface definition.
  }];
}

#endif
