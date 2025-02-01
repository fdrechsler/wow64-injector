# Contributing to WoW Lua Injector

Thank you for your interest in contributing to this project! This document provides guidelines and instructions for contributing.

## Code of Conduct

By participating in this project, you agree to maintain a professional and respectful environment. Any form of harassment or discriminatory behavior will not be tolerated.

## Getting Started

1. Fork the repository
2. Clone your fork:
```bash
git clone https://github.com/yourusername/wow-lua-injector.git
cd wow-lua-injector
```

3. Create a new branch:
```bash
git checkout -b feature/your-feature-name
```

## Development Environment Setup

1. Install required software:
   - Visual Studio 2019 or later
   - CMake 3.10 or later
   - Windows SDK 10.0 or later

2. Build the project:
```bash
build.bat
```

## Coding Standards

### General Guidelines
- Use consistent indentation (4 spaces)
- Keep lines under 100 characters
- Use meaningful variable and function names
- Document complex algorithms
- Add comments for non-obvious code
- Follow the existing code style

### C++ Specific
- Use modern C++ features (C++17)
- Prefer `nullptr` over `NULL`
- Use smart pointers for memory management
- Use `const` where appropriate
- Follow RAII principles
- Use `override` for virtual functions
- Use `noexcept` where appropriate

### File Organization
- Header files go in `include/`
- Source files go in `src/`
- Test files go in `tests/`
- One class per file
- Use meaningful file names

## Testing

1. Run the test suite:
```bash
test.bat
```

2. Add tests for new features:
   - Create test cases in appropriate test files
   - Cover both success and failure scenarios
   - Test edge cases
   - Verify anti-detection features

## Pull Request Process

1. Update documentation:
   - Add/update comments in code
   - Update README.md if needed
   - Document new features or changes

2. Run tests:
   - Ensure all tests pass
   - Add new tests if needed
   - Verify both 32-bit and 64-bit builds

3. Create pull request:
   - Use a clear title
   - Describe changes in detail
   - Reference any related issues
   - List any breaking changes

4. Code review:
   - Address reviewer comments
   - Make requested changes
   - Keep discussions professional

## Security Considerations

- Do not commit sensitive information
- Do not weaken anti-detection features
- Report security issues privately
- Follow secure coding practices
- Consider anti-analysis implications

## Performance Guidelines

- Profile code changes
- Minimize memory allocations
- Consider thread safety
- Optimize critical paths
- Benchmark significant changes

## Documentation

- Use clear and concise language
- Keep README.md up to date
- Document public APIs
- Include usage examples
- Explain complex features

## Version Control

- Make atomic commits
- Write meaningful commit messages
- Follow conventional commits format
- Keep history clean (rebase if needed)
- Tag releases appropriately

## Legal

- All contributions must be under MIT License
- Include copyright notices
- Respect intellectual property
- Follow license terms
- Sign-off commits

## Questions?

If you have questions about contributing, please:
1. Check existing documentation
2. Search closed issues
3. Open a new issue if needed

Thank you for contributing!