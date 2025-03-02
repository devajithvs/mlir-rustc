#ifndef MirOpInterfaces
#define MirOpInterfaces

include "mlir/IR/OpBase.td"

class Mir_OpInterface<string name> : OpInterface<name> {
  let cppNamespace = "::rust_compiler::Mir";
}

def BorrowOpInterface : OpInterface<"BorrowOpInterface"> {
  let description = [{
    This is an example interface definition.
  }];
}

#endif
