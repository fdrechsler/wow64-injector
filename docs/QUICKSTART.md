# Quick Start Guide

## Installation

### Prerequisites
- Windows 10 or later
- Visual Studio 2019 or later
- CMake 3.10 or later
- Administrator privileges

### Download
1. Clone the repository:
```bash
git clone https://github.com/yourusername/wow-lua-injector.git
cd wow-lua-injector
```

2. Or download the latest release:
   - Visit the [Releases](https://github.com/yourusername/wow-lua-injector/releases) page
   - Download the appropriate version (32-bit or 64-bit)

## Building from Source

### Quick Build
Run the build script:
```bash
build.bat
```
This will create both 32-bit and 64-bit versions in the `dist` directory.

### Manual Build
1. Create build directory:
```bash
mkdir build32
cd build32
```

2. Configure and build:
```bash
cmake .. -A Win32
cmake --build . --config Release
```

## Basic Usage

### Running the Injector
1. Start World of Warcraft
2. Run the appropriate injector version as administrator:
   - `dist/injector32.exe` for 32-bit WoW
   - `dist/injector64.exe` for 64-bit WoW

### Command Line Options
```bash
injector32.exe [options]

Options:
  -h, --help              Show help message
  -v, --verbose           Enable verbose logging
  --script <file>         Use custom Lua script
  --test-all             Run all tests
```

### Using Custom Scripts
1. Create a Lua script file (e.g., `script.lua`)
2. Run with script option:
```bash
injector32.exe --script script.lua
```

## Example Scripts

### Basic Test Script
```lua
local function test()
    print("Injection successful!")
end
test()
```

### Function Hook Example
```lua
local function onSpellCast(self, ...)
    print("Spell cast detected!")
    -- Your code here
end

hooksecurefunc("CastSpell", onSpellCast)
```

## Troubleshooting

### Common Issues
1. **Administrator Rights**
   - Run as administrator
   - Check UAC settings

2. **Process Not Found**
   - Verify WoW is running
   - Check version match (32/64-bit)

3. **Injection Fails**
   - Check antivirus settings
   - Verify process permissions

### Getting Help
1. Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
2. Search GitHub issues
3. Create new issue with details

## Security

### Best Practices
1. Run in test environment first
2. Keep software updated
3. Use minimum privileges needed
4. Monitor for detection

### Safety Measures
1. Backup important data
2. Test scripts thoroughly
3. Follow security guidelines
4. Report issues responsibly

## Additional Resources

### Documentation
- [Technical Guide](TECHNICAL.md)
- [Security Policy](../SECURITY.md)
- [Contributing Guide](../CONTRIBUTING.md)
- [Change Log](../CHANGELOG.md)

### Community
- GitHub Discussions
- Issue Tracker
- Security Reports
- Feature Requests

## Support

### Getting Help
1. Read documentation
2. Check existing issues
3. Create detailed reports
4. Follow guidelines

### Reporting Bugs
1. Check known issues
2. Provide reproduction steps
3. Include system details
4. Follow templates

## Updates

### Staying Current
1. Watch repository
2. Check releases
3. Update regularly
4. Follow changelog

### Version Check
```bash
injector32.exe --version
```

Remember: This software is for educational purposes only. Use responsibly and follow all applicable laws and terms of service.