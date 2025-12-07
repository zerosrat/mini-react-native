/*
 * Copyright (C) 2006 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JSBase_h
#define JSBase_h

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

/* JavaScript engine interface */

/*! @typedef JSContextGroupRef A group that associates JavaScript contexts with one another. */
typedef const struct OpaqueJSContextGroup* JSContextGroupRef;

/*! @typedef JSContextRef A JavaScript execution context. */
typedef const struct OpaqueJSContext* JSContextRef;

/*! @typedef JSGlobalContextRef A global JavaScript execution context. */
typedef struct OpaqueJSContext* JSGlobalContextRef;

/*! @typedef JSStringRef A UTF16 character buffer. */
typedef struct OpaqueJSString* JSStringRef;

/*! @typedef JSClassRef A JavaScript class. */
typedef struct OpaqueJSClass* JSClassRef;

/*! @typedef JSPropertyNameArrayRef An array of JavaScript property names. */
typedef struct OpaqueJSPropertyNameArray* JSPropertyNameArrayRef;

/*! @typedef JSPropertyNameAccumulatorRef A function used to accumulate the names of a JavaScript object's properties. */
typedef struct OpaqueJSPropertyNameAccumulator* JSPropertyNameAccumulatorRef;

/*! @typedef JSValueRef A JavaScript value. */
typedef const struct OpaqueJSValue* JSValueRef;

/*! @typedef JSObjectRef A JavaScript object. */
typedef struct OpaqueJSValue* JSObjectRef;

/* JavaScript data types */

/*! @typedef JSType A constant identifying the type of a JSValue. */
typedef enum {
    kJSTypeUndefined,
    kJSTypeNull,
    kJSTypeBoolean,
    kJSTypeNumber,
    kJSTypeString,
    kJSTypeObject
} JSType;

#ifdef __cplusplus
extern "C" {
#endif

/* Garbage collection */

/*!
@function
@abstract Performs a JavaScript garbage collection.
@discussion JavaScript values that are on the machine stack, in a register,
 protected by JSValueProtect, set as the global object of an execution context,
 or reachable from any such value will not be collected.

 During JavaScript execution, you are not required to call this function; the
 JavaScript engine will garbage collect as needed. JavaScript values created
 within a context group are automatically destroyed when the last reference
 to the context group is released.
@param ctx The execution context to use.
*/
void JSGarbageCollect(JSContextRef ctx);

#ifdef __cplusplus
}
#endif

#endif /* JSBase_h */