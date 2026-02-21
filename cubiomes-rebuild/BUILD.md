# Build Instructions

## Requirements

- GCC (MinGW or similar)
- Windows command prompt

## Building

```batch
cd cubiomes-rebuild
build.bat           # Build library only
build.bat test      # Build library + tests
test.bat            # Run all tests
clean.bat           # Clean build artifacts
```

## Output

- `lib/libcubiomes.a` - Static library
- `bin/*.exe` - Test executables (if built with `build.bat test`)

## Using in Your Project

### Compile Flags
```bash
-I path/to/cubiomes-rebuild
-L path/to/cubiomes-rebuild/lib
-lcubiomes -lm
```

### Example
```cpp
#include "generator.h"

int main() {
    Generator g;
    setupGenerator(&g, MC_1_18, 0);
    applySeed(&g, DIM_OVERWORLD, 12345);
    
    int biome = getBiomeAt(&g, 1, 0, 63 >> 2, 0);
    printf("Biome: %d\n", biome);
    
    return 0;
}
```

Compile:
```bash
gcc your_code.c -I cubiomes-rebuild -L cubiomes-rebuild/lib -lcubiomes -lm
```

## Troubleshooting

**"gcc is not recognized"**  
Add GCC to your PATH or use full path to gcc.exe

**"ar is not recognized"**  
Make sure your GCC installation includes binutils (ar tool).

**Link errors**  
Make sure to link with `-lm` (math library).
