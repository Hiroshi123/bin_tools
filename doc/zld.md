# zld - Custom Linker

zld is a custom linker implementation that supports both ELF and COFF/PE object file formats. It provides fine-grained control over the linking process and can generate executables, shared objects, and DLLs.

## Usage

```bash
zld [options] object_files...
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `-v` | Verbose output - displays detailed linking information |
| `-o <file>` | Specify output filename (default: a.out) |
| `-l <library>` | Link against dynamic library |
| `-p` | Pack binary - optimize section layout |
| `-nodynamic` | Create static binary without dynamic linking |
| `-use-dt-hash` | Use DT_HASH instead of GNU_HASH for symbol tables |
| `-no-pie` | Disable Position Independent Executable |
| `-ef <function>` | Specify entry point function name |
| `-ib <address>` | Specify image base address |

### Output File Types

The output format is determined by the file extension:
- `.exe` - Windows executable (COFF/PE format)
- `.dll` - Windows dynamic library
- `.so` - Unix shared object (ELF format)
- `.o` - Object file
- Default: ELF executable

## Architecture

### Core Data Structures

**SectionContainer**
- Manages groups of related sections that will be merged
- Contains linked list of SectionChain entries
- Tracks virtual addresses and sizes
- Handles section alignment and layout

**ObjectChain** 
- Represents each input object file
- Contains symbol table and string table pointers
- Links to associated section chains
- Manages relocation information

**SymbolChain**
- Individual symbol entries with hash table support
- Links symbols to their containing sections
- Supports both local and global symbol resolution

### Linking Process

1. **Initialization**
   - Parse command line arguments
   - Initialize memory management and logging
   - Set up hash tables for symbol resolution

2. **Object File Processing**
   - Read each input object file using custom memory allocator
   - Parse ELF section headers and symbol tables
   - Build internal data structures (ObjectChain, SectionContainer)

3. **Section Merging**
   - Group compatible sections together
   - Calculate virtual addresses and offsets
   - Handle special sections (.plt.got, .dynamic, .bss)

4. **Symbol Resolution**
   - Build export symbol table
   - Resolve external references
   - Handle dynamic library dependencies

5. **Relocation Processing**
   - Process relocation entries for each section
   - Apply address fixups
   - Generate PLT/GOT entries for dynamic symbols

6. **Output Generation**
   - Write ELF/PE headers
   - Emit program headers and section data
   - Generate final executable or shared object

### Memory Layout

**ELF Executables**
- Base address: 0x400000 (configurable)
- Program headers follow ELF header
- Virtual address alignment: configurable (default: none)
- Sections laid out sequentially in memory

**ELF Shared Objects**
- Base address: 0x00000 (relocatable)
- Position-independent code required
- Dynamic section contains runtime linking info
- GNU hash table for symbol lookup optimization

## Hash Table Support

zld supports both traditional DT_HASH and GNU_HASH symbol tables:

**DT_HASH**
- Compatible with older systems
- Simple bucket/chain hash table
- Use `-use-dt-hash` option

**GNU_HASH** (default)
- More efficient symbol lookup
- Bloom filter optimization
- Configurable bucket count and bloom size

## Dynamic Linking

When creating dynamic executables or shared objects:

1. **Dynamic Section** - Contains runtime linking metadata
2. **PLT/GOT** - Procedure Linkage Table and Global Offset Table
3. **Symbol Tables** - DYNSYM and DYNSTR sections
4. **Relocation Entries** - Runtime relocations for dynamic symbols

## Configuration

Default configuration can be customized:
- Entry point: virtual_address_offset (after headers)
- Program headers: 2 (LOAD and DYNAMIC)
- Hash table parameters: 1 bucket, 2 bloom filters
- Dynamic entries: 13 for dynamic, 1 for static

## Supported Platforms

- **Linux x86-64**: Full ELF support
- **Windows x86-64**: COFF/PE support (partial)
- **Object formats**: ELF, COFF, PE, Mach-O (read-only)

## Examples

```bash
# Link object files into executable
zld main.o utils.o -o myprogram

# Create shared library
zld -o libmath.so math.o operations.o

# Static linking (no dynamic dependencies)
zld -nodynamic main.o -o static_program

# Verbose linking with custom entry point
zld -v -ef my_start main.o -o program

# Link with external libraries
zld main.o -l lib/so/mylib.so -o program
```

## Debugging

Use `-v` flag for verbose output showing:
- Section merging process
- Virtual address assignments  
- Symbol resolution details
- Relocation processing
- Final memory layout

Log files are generated in `misc.log` for detailed debugging information.

## Limitations

- Concurrent object processing not yet implemented
- Limited support for some ELF extensions
- COFF/PE support is experimental
- No linker script support currently