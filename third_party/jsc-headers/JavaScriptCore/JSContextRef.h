/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
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

#ifndef JSContextRef_h
#define JSContextRef_h

#include "JSBase.h"
#include "JSValueRef.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
@function
@abstract Creates a global JavaScript execution context.
@discussion JSGlobalContextCreate allocates a global object and populates it with all the
 built-in JavaScript objects, such as Object, Function, String, and Array.

 In WebKit version 4.0 and later, the context is created in a unique context group.
 Therefore, scripts may execute in it concurrently with scripts executing in other contexts.
 However, you may not use values created in the context in other contexts.
@param globalObjectClass The class to use when creating the global object. Pass
 NULL to use the default object class.
@result A JSGlobalContext with a global object of class globalObjectClass.
*/
JSGlobalContextRef JSGlobalContextCreate(JSClassRef globalObjectClass);

/*!
@function
@abstract Retains a global JavaScript execution context.
@param ctx The JSGlobalContext to retain.
@result A JSGlobalContext that is the same as ctx.
*/
JSGlobalContextRef JSGlobalContextRetain(JSGlobalContextRef ctx);

/*!
@function
@abstract Releases a global JavaScript execution context.
@param ctx The JSGlobalContext to release.
*/
void JSGlobalContextRelease(JSGlobalContextRef ctx);

/*!
@function
@abstract Gets the global object of a JavaScript execution context.
@param ctx The JSContext whose global object you want to get.
@result ctx's global object.
*/
JSObjectRef JSContextGetGlobalObject(JSContextRef ctx);

/*!
@function
@abstract Gets the context group to which a JavaScript execution context belongs.
@param ctx The JSContext whose group you want to get.
@result ctx's group.
*/
JSContextGroupRef JSContextGetGroup(JSContextRef ctx);

/*!
@function
@abstract Gets the global context of a JavaScript execution context.
@param ctx The JSContext whose global context you want to get.
@result ctx's global context.
*/
JSGlobalContextRef JSContextGetGlobalContext(JSContextRef ctx);

/*!
@function
@abstract Gets a copy of the name of a context.
@param ctx The JSGlobalContext whose name you want to get.
@result The name for ctx.
@discussion A JSGlobalContext's name is exposed for remote debugging to make it
easier to identify the context you would like to attach to.
*/
JSStringRef JSGlobalContextCopyName(JSGlobalContextRef ctx);

/*!
@function
@abstract Sets the remote debugging name for a context.
@param ctx The JSGlobalContext that you want to name.
@param name The remote debugging name to set on ctx.
*/
void JSGlobalContextSetName(JSGlobalContextRef ctx, JSStringRef name);

/*!
@function
@abstract Evaluates a string of JavaScript.
@param ctx The execution context to use.
@param script A JSString containing the script to evaluate.
@param thisObject The object to use as "this," or NULL to use the global object as "this."
@param sourceURL A JSString containing a URL for the script's source file. This is used by debuggers and when reporting exceptions. Pass NULL if you do not care to include source file information.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is used by debuggers and when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care about exceptions.
@result The JSValue that results from evaluating script, or NULL if an exception is thrown.
*/
JSValueRef JSEvaluateScript(JSContextRef ctx, JSStringRef script, JSObjectRef thisObject, JSStringRef sourceURL, int startingLineNumber, JSValueRef* exception);

#ifdef __cplusplus
}
#endif

#endif /* JSContextRef_h */