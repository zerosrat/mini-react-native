# Mini React Native

> **[中文文档](README_CN.md) | English**

[![License](https://img.shields.io/badge/license-Not%20Yet%20Specified-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20iOS%20%7C%20Android-lightgrey.svg)]()
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

**The first complete educational project that implements React Native from scratch.**

A minimal React Native implementation that deeply explores core mechanisms from traditional Bridge architecture to modern JSI architecture. Helping developers who want to truly understand cross-platform technology internals.

---

## Why This Project

While React Native is widely used, there's a significant gap in resources that deeply explain its internals:

- **Official docs** focus on usage, not implementation details
- **Existing tutorials** rarely go beyond surface-level explanations
- **React Native source code** is complex and hard to navigate (especially for JS developers unfamiliar with C++, Objective-C, and Java)

**This project fills that gap** by providing a complete, from-scratch implementation with educational documentation at every step.

## Key Features

✅ **Progressive Learning** - 4 phases from Bridge to JSI, each phase is independently runnable  
✅ **Real Implementation** - Genuine JavaScriptCore integration, RN-compatible design, not mocked code  
✅ **Complete Documentation** - Detailed architecture analysis and implementation notes explaining every design decision

---

## Quick Start

### Prerequisites

**System Requirements**:

- macOS 10.15+
- Xcode Command Line Tools (includes make and clang)
- CMake 3.15+ (brew install cmake)
- C++17 compatible compiler

### Build and Run

```bash
# Clone the repository
git clone https://github.com/yourusername/mini-react-native.git
cd mini-react-native

# Build and run tests
make test-integration

# Expected output:
# ✓ Module registered successfully
# ✓ getSystemVersion: macOS 14.0
# ✓ getModel: MacBookPro18,1
```

### Project Structure

```
mini-react-native/
├── src/common/          # Cross-platform core code
│   ├── bridge/          # JSCExecutor - JS engine integration
│   ├── modules/         # Module registration and management
│   └── utils/           # JSON serialization and utilities
├── src/js/              # JavaScript implementation
│   ├── MessageQueue.js  # RN-compatible message queue
│   └── NativeModule.js  # Module proxy
├── src/macos/           # Platform-specific implementation
└── examples/            # Test cases
```

### Troubleshooting

**Issue**: `JavaScriptCore framework not found`
```bash
# Solution: Ensure Xcode Command Line Tools are installed
xcode-select --install
```

**Issue**: `CMake version too old`
```bash
# Solution: Install latest CMake via Homebrew
brew install cmake
```

**Issue**: Build errors with C++ standard
```bash
# Solution: Ensure your compiler supports C++17
clang++ --version  # Should be 10.0+
```

---

## Roadmap

### Phase 1: Bridge Communication ✅

**Goal**: Complete JS ↔ Native bidirectional messaging

- [x] JSCExecutor with real JavaScriptCore
- [x] RN-compatible MessageQueue
- [x] Native function injection
- [x] Type conversion system
- [x] Complete module system

**Learning Value**: Understand the foundation of all RN communication

### Phase 2: JavaScript Engine Deep Dive (3-4 weeks)

**Goal**: Optimize JS engine integration + cross-platform support

- [ ] iOS support
- [ ] Memory management optimization
- [ ] Chrome DevTools integration
- [ ] Android platform support
- [ ] Hot reload basics

**Learning Value**: Master JS engine internals and multi-platform architecture

### Phase 3: View Rendering System (4-5 weeks)

**Goal**: Implement React component → native view rendering

- [ ] Shadow Tree (Virtual DOM) implementation
- [ ] Yoga layout engine integration
- [ ] Basic components (View, Text, Image)
- [ ] Event system (touch, gestures)
- [ ] Diff algorithm and incremental updates

**Learning Value**: Understand how React renders to native UI

### Phase 4: New Architecture Migration (3-4 weeks)

**Goal**: Implement JSI + TurboModules + Fabric

- [ ] JSI synchronous calling
- [ ] TurboModules with lazy loading
- [ ] C++ object direct exposure to JS
- [ ] Performance comparison: Bridge vs JSI
- [ ] Migration guide

**Learning Value**: Experience the future of React Native

---

**⭐ Star this repo if you find it helpful!**

Your support keeps me motivated to continue!
