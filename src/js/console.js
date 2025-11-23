/**
 * Console.js - JavaScript-side console implementation
 *
 * Implements console object on the JavaScript side using nativeLoggingHook
 * for actual logging. This replaces the C++ setupConsole implementation.
 */

'use strict'

// Create console object with standard methods
const console = {
  log(...args) {
    if (typeof global.nativeLoggingHook === 'function') {
      // Join all arguments with spaces, similar to standard console.log behavior
      const message = args
        .map((arg) => {
          if (typeof arg === 'string') return arg
          if (typeof arg === 'object') {
            try {
              return JSON.stringify(arg)
            } catch (e) {
              return '[Object]'
            }
          }
          return String(arg)
        })
        .join(' ')

      global.nativeLoggingHook('JS LOG', message)
    }
  },

  error(...args) {
    if (typeof global.nativeLoggingHook === 'function') {
      const message = args.map((arg) => String(arg)).join(' ')
      global.nativeLoggingHook('JS ERROR', message)
    }
  },

  warn(...args) {
    if (typeof global.nativeLoggingHook === 'function') {
      const message = args.map((arg) => String(arg)).join(' ')
      global.nativeLoggingHook('JS WARN', message)
    }
  },

  info(...args) {
    if (typeof global.nativeLoggingHook === 'function') {
      const message = args.map((arg) => String(arg)).join(' ')
      global.nativeLoggingHook('JS INFO', message)
    }
  },

  debug(...args) {
    if (typeof global.nativeLoggingHook === 'function') {
      const message = args.map((arg) => String(arg)).join(' ')
      global.nativeLoggingHook('JS DEBUG', message)
    }
  },
}

// Register console on global object
if (typeof global !== 'undefined') {
  global.console = console
}

// Export for CommonJS compatibility
module.exports = console
