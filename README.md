# WoW Lua Injector

An advanced Lua code injection tool with sophisticated anti-detection features.

## Features

- **Pattern Scanning**: Dynamic offset detection for version independence
- **Warden Anti-Cheat Detection**: Advanced detection and evasion mechanisms
- **IAT Hiding**: Import Address Table protection and encryption
- **Thread Hijacking**: Sophisticated thread manipulation techniques
- **Memory Protection**: Secure memory allocation and management
- **Anti-Analysis**: Multiple layers of anti-debugging and anti-VM measures

## Requirements

- Windows 10 or later
- Visual Studio 2019 or later
- CMake 3.10 or later
- Administrator privileges

## Building the Project

### Quick Build

Run the build script:
```batch
build.bat
```
This will:
1. Build both 32-bit and 64-bit versions
2. Create binaries in the `dist` directory
3. Generate debug symbols

### Manual Build

1. Create build directories:
```batch
mkdir build32 build64
```

2. Configure and build 32-bit version:
```batch
cd build32
cmake .. -A Win32
cmake --build . --config Release
cd ..
```

3. Configure and build 64-bit version:
```batch
cd build64
cmake .. -A x64
cmake --build . --config Release
cd ..
```

### Cleaning

To clean all build artifacts:
```batch
clean.bat
```

## Project Structure

```
src_cpp/
├── include/                 # Header files
│   ├── warden_detector.h   # Warden detection system
│   ├── pattern_scanner.h   # Memory pattern scanning
│   ├── iat_hiding.h        # IAT protection
│   ├── lua_injector.h      # Main injection logic
│   ├── process_memory.h    # Memory management
│   ├── logger.h           # Logging system
│   └── version.h          # Version information
├── src/                    # Implementation files
│   ├── warden_detector.cpp
│   ├── pattern_scanner.cpp
│   ├── iat_hiding.cpp
│   ├── lua_injector.cpp
│   ├── process_memory.cpp
│   └── main.cpp
├── cmake/                  # CMake scripts
│   └── clean-all.cmake    # Cleanup script
├── CMakeLists.txt         # Build configuration
├── version.h.in           # Version template
├── build.bat              # Build script
└── clean.bat              # Cleanup script
```

## Usage

1. Build the project using either method above
2. Run the appropriate version for your target:
   - `dist/injector32.exe` for 32-bit processes
   - `dist/injector64.exe` for 64-bit processes

The injector must be run with administrator privileges.

## Security Features

### Anti-Detection
- Warden signature scanning
- Thread activity monitoring
- Memory permission analysis
- Module integrity checking
- Timing-based detection

### Anti-Analysis
- IAT hiding and encryption
- Anti-debugging checks
- Virtualization detection
- Process suspension during injection
- Memory protection management

### Pattern Scanning
- Dynamic function offset detection
- Support for signature patterns with wildcards
- Memory region filtering
- Efficient batch scanning

## Memory Management

- Secure allocation/deallocation
- Memory protection
- Process manipulation
- Thread context management

## Code Injection

- Encrypted Lua payload
- Remote thread creation
- Dynamic function resolution
- Pattern-based offset scanning

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is for educational purposes only. Use at your own risk.

## Disclaimer

This software is provided for educational purposes only. The authors are not responsible for any misuse or damages caused by this software. Using this software to modify game clients may violate terms of service and result in account penalties.