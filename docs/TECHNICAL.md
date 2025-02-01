# Technical Documentation

## Architecture Overview

### Core Components
1. **Lua Injector**
   - Main injection logic
   - Process management
   - Memory operations
   - Thread manipulation

2. **Pattern Scanner**
   - Dynamic offset detection
   - Memory pattern matching
   - Region filtering
   - Signature verification

3. **Warden Detector**
   - Anti-cheat detection
   - Timing analysis
   - Module verification
   - Thread monitoring

4. **Memory Handler**
   - Memory allocation
   - Protection management
   - Process communication
   - Thread context control

5. **IAT Protection**
   - Import table hiding
   - Function redirection
   - Address obfuscation
   - Table restoration

## Memory Management

### Allocation Strategy
```cpp
enum class MemoryAllocationType {
    MEM_COMMIT_PATTERN,    // Pattern-based allocation
    MEM_COMMIT_STANDARD    // Standard allocation
};
```

### Protection Flags
- PAGE_EXECUTE_READ
- PAGE_EXECUTE_READWRITE
- PAGE_READWRITE
- PAGE_READONLY

### Memory Operations
1. Pattern-based allocation
2. XOR encryption
3. Permission management
4. Thread context manipulation

## Anti-Detection Mechanisms

### Warden Evasion
1. Signature scanning
2. Timing checks
3. Module verification
4. Thread monitoring

### Anti-Debug
1. IsDebuggerPresent checks
2. Remote debugger detection
3. Timing verification
4. Thread manipulation detection

### IAT Protection
1. Import table hiding
2. Function redirection
3. Address obfuscation
4. Dynamic resolution

## Thread Management

### Thread Operations
1. Main thread detection
2. Context manipulation
3. Stack analysis
4. Execution control

### Hijacking Process
1. Find main thread
2. Save context
3. Modify execution
4. Restore state

## Pattern Scanning

### Signature Format
```cpp
struct Pattern {
    std::string name;
    std::vector<std::pair<BYTE, bool>> bytes;
    int32_t offset;
};
```

### Scanning Process
1. Memory region enumeration
2. Pattern matching
3. Validation
4. Offset calculation

## Lua Integration

### Payload Structure
```lua
local function checkWarden()
    local t1 = GetTime()
    local t2 = GetTime()
    return (t2 - t1) < 0.001
end

local function main()
    if checkWarden() then return end
    -- Custom code here
end
```

### Injection Process
1. Payload preparation
2. Memory allocation
3. Code injection
4. Thread hijacking

## Security Measures

### Memory Protection
1. Secure allocation
2. Permission management
3. Content encryption
4. Access validation

### Anti-Analysis
1. Timing checks
2. Module verification
3. Thread monitoring
4. Pattern detection

## Performance Optimization

### Memory Operations
1. Pattern-based allocation
2. Efficient scanning
3. Minimal copying
4. Smart pointers

### Thread Management
1. Efficient context switching
2. Minimal suspension
3. Quick restoration
4. Resource cleanup

## Error Handling

### Exception Types
1. Memory allocation failures
2. Pattern matching errors
3. Thread manipulation issues
4. Protection violations

### Recovery Process
1. Resource cleanup
2. State restoration
3. Error logging
4. Graceful exit

## Testing Framework

### Test Categories
1. Pattern scanning
2. Memory operations
3. Thread manipulation
4. Anti-detection

### Validation Process
1. Function verification
2. Memory integrity
3. Thread state
4. Security checks

## Build System

### CMake Configuration
1. Platform detection
2. Feature flags
3. Dependency management
4. Output configuration

### Compilation Options
1. Optimization levels
2. Debug information
3. Warning levels
4. Security features

## Future Enhancements

### Planned Features
1. Enhanced pattern scanning
2. Additional anti-detection
3. Network protocol
4. GUI interface

### Optimization Areas
1. Memory usage
2. Scanning speed
3. Thread management
4. Error recovery

## API Reference

### Core Classes
1. LuaInjector
2. PatternScanner
3. WardenDetector
4. MemoryHandler

### Key Methods
1. Injection control
2. Memory management
3. Thread manipulation
4. Pattern scanning

## Implementation Details

### Memory Layout
1. Code section
2. Data section
3. Import table
4. Thread contexts

### Protection Mechanisms
1. Memory encryption
2. Address randomization
3. Thread monitoring
4. State validation