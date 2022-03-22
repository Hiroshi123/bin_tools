


// Context* con = _get_context();

// Accumulator(A)
// Index register(X,Y)
// Stack Pointer (S)
// Data Bank (DB)
// Direct Page Addressing (D)
// Processor Status (P)
// Program Bank (PC)
// 

int f1(int op) {

  // int op = 0;
  switch (op) {
  case 0x00:
    break;
  case 0x01:
    break;
  case 0x18:
    // CLC (clear carry flag)
    break;
  case 0x3a:
    // DEC
    break;
    
  case 0x8d:
    // Store Accumulator to Memory
    /// STA
    break;
  case 0xa9:
    // LDA (Load Accumulator from memory)
    // + 2 or 3
    break;
  }
}


