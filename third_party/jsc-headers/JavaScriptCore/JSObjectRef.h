/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
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

#ifndef JSObjectRef_h
#define JSObjectRef_h

#include "JSBase.h"
#include "JSValueRef.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stddef.h> /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif

/*!
@enum JSPropertyAttribute
@abstract A set of JSPropertyAttributes. Combine multiple attributes by logically ORing them together.
@constant kJSPropertyAttributeNone         Specifies that a property has no special attributes.
@constant kJSPropertyAttributeReadOnly     Specifies that a property is read-only.
@constant kJSPropertyAttributeDontEnum     Specifies that a property should not be enumerated by JSPropertyNameAccumulators.
@constant kJSPropertyAttributeDontDelete   Specifies that attempts to delete a property should fail.
*/
enum {
    kJSPropertyAttributeNone         = 0,
    kJSPropertyAttributeReadOnly     = 1 << 1,
    kJSPropertyAttributeDontEnum     = 1 << 2,
    kJSPropertyAttributeDontDelete   = 1 << 3
};

/*!
@typedef JSPropertyAttributes
@abstract A set of JSPropertyAttributes. Combine multiple attributes by logically ORing them together.
*/
typedef unsigned JSPropertyAttributes;

/*!
@typedef JSObjectInitializeCallback
@abstract The callback invoked when an object is first created.
@param ctx The execution context to use.
@param object The JSObject being created.
@discussion If you named your function Initialize, you would declare it like this:

void Initialize(JSContextRef ctx, JSObjectRef object);

Unlike the other object callbacks, the initialize callback is called on the least
derived class (the parent class) first, and the most derived class last.
*/
typedef void
(*JSObjectInitializeCallback) (JSContextRef ctx, JSObjectRef object);

/*!
@typedef JSObjectFinalizeCallback
@abstract The callback invoked when an object is finalized (prepared for garbage collection).
@param object The JSObject being finalized.
@discussion If you named your function Finalize, you would declare it like this:

void Finalize(JSObjectRef object);

The finalize callback is called on the most derived class first, and the least
derived class (the parent class) last.

You must not call any function that may cause a garbage collection or an allocation
of a garbage collected object from within a JSObjectFinalizeCallback. This includes
all functions that have a JSContextRef parameter.
*/
typedef void
(*JSObjectFinalizeCallback) (JSObjectRef object);

/*!
@typedef JSObjectCallAsFunctionCallback
@abstract The callback invoked when an object is called as a function.
@param ctx The execution context to use.
@param function The JSObject being called as a function.
@param thisObject The JSObject to use as "this," or NULL to use the global object as "this."
@param argumentCount An integer count of the number of arguments in arguments.
@param arguments A JSValue array of the  arguments passed to the function.
@param exception A pointer to a JSValueRef in which to return an exception, if any.
@result A JSValue that is the function's return value.
@discussion If you named your function CallAsFunction, you would declare it like this:

JSValueRef CallAsFunction(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

If your callback were invoked by the JavaScript expression 'myObject.myFunction()', function would be set to myFunction, and thisObject would be set to myObject.

If your callback were invoked by the JavaScript expression 'myFunction()', function would be set to myFunction, and thisObject would be set to the global object.

If your callback were invoked by the JavaScript expression 'new myConstructor()', function would be set to myConstructor, and thisObject would be set to a newly created object with a prototype set to myConstructor.prototype.

If your callback were invoked by the JavaScript expression 'myFunction.call(myObject)', function would be set to myFunction, and thisObject would be set to myObject.

If your callback were invoked by the JavaScript expression 'myFunction.apply(myObject)', function would be set to myFunction, and thisObject would be set to myObject.

If a callback is not specified, calling your object as a function will throw an exception.
*/
typedef JSValueRef
(*JSObjectCallAsFunctionCallback) (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

/*!
@typedef JSClassAttributes
@abstract A set of JSClassAttributes. Combine multiple attributes by logically ORing them together.
@constant kJSClassAttributeNone Specifies that a class has no special attributes.
@constant kJSClassAttributeNoAutomaticPrototype Specifies that a class should not automatically generate a shared prototype for its instance objects. Use kJSClassAttributeNoAutomaticPrototype in combination with JSObjectSetPrototype to manage prototypes manually.
*/
enum {
    kJSClassAttributeNone = 0,
    kJSClassAttributeNoAutomaticPrototype = 1 << 1
};

typedef unsigned JSClassAttributes;

/*!
@struct JSClassDefinition
@abstract This structure contains properties and callbacks that define a type of object.
@field version The version number of this structure. The current version is 0.
@field attributes A logically ORed set of JSClassAttributes to give to the class.
@field className A null-terminated UTF8 string containing the class's name.
@field parentClass A JSClass to set as the class's parent class. Pass NULL to use the default object class.
@field staticValues A JSStaticValue array containing the class's statically declared value properties.
@field staticFunctions A JSStaticFunction array containing the class's statically declared function properties.
@field initialize The callback invoked when an object is first created. Use this callback to initialize the object.
@field finalize The callback invoked when an object is finalized (prepared for garbage collection). Use this callback to release resources allocated for the object, and perform other cleanup.
@field hasProperty The callback invoked when determining whether an object has a property. If this field is NULL, getProperty is called instead. The hasProperty callback enables optimization in cases where only a property's existence needs to be known, not its value, and computing its value is expensive.
@field getProperty The callback invoked when getting a property's value.
@field setProperty The callback invoked when setting a property's value.
@field deleteProperty The callback invoked when deleting a property.
@field getPropertyNames The callback invoked when collecting the names of an object's properties.
@field callAsFunction The callback invoked when an object is called as a function.
@field callAsConstructor The callback invoked when an object is used as the target of an 'instanceof' expression.
@field hasInstance The callback invoked when an object is used as the target of an 'instanceof' expression.
@field convertToType The callback invoked when converting an object to a particular JavaScript type.
@discussion The staticValues and staticFunctions arrays are the simplest and most efficient means for vending custom properties. Statically declared properties autmatically service requests like getProperty, setProperty, and getPropertyNames. Property access callbacks are required only to implement unusual properties, like array indexes, whose names are not known at compile time.

If you named your getter function "GetX" and your setter function "SetX", you would declare a JSStaticValue array containing "X" like this:

JSStaticValue StaticValueArray[] = {
    { "X", GetX, SetX, kJSPropertyAttributeNone },
    { 0, 0, 0, 0 }
};

Standard JavaScript practice calls for storing function objects in prototypes, so they can be shared. The default JSClass created by JSClassCreate follows this idiom, instantiating objects with a shared, automatically generating prototype containing the class's function objects. The kJSClassAttributeNoAutomaticPrototype attribute specifies that a JSClass should not automatically generate such a prototype. The resulting JSClass instantiates objects with the default object prototype, and gives each instance object its own copy of the class's function objects.

A NULL callback specifies that the default object callback should substitute, except in the case of hasProperty, where it specifies that getProperty should substitute.
*/
typedef struct {
    int                                 version; /* current (and only) version is 0 */
    JSClassAttributes                   attributes;

    const char*                         className;
    JSClassRef                          parentClass;

    const struct JSStaticValue*                   staticValues;
    const struct JSStaticFunction*                staticFunctions;

    JSObjectInitializeCallback          initialize;
    JSObjectFinalizeCallback            finalize;
    void*                               hasProperty;
    void*                               getProperty;
    void*                               setProperty;
    void*                               deleteProperty;
    void*                               getPropertyNames;
    JSObjectCallAsFunctionCallback      callAsFunction;
    void*                               callAsConstructor;
    void*                               hasInstance;
    void*                               convertToType;
} JSClassDefinition;

/*!
@const kJSClassDefinitionEmpty
@abstract A JSClassDefinition structure of the current version, filled with NULL pointers and having no attributes.
@discussion Use this constant as a convenience when creating class definitions. For example, to create a class definition with only a finalize method:

JSClassDefinition definition = kJSClassDefinitionEmpty;
definition.finalize = Finalize;
*/
extern const JSClassDefinition kJSClassDefinitionEmpty;

/*!
@function
@abstract Creates a JavaScript class suitable for use with JSObjectMake.
@param definition A JSClassDefinition that defines the class.
@result A JSClass with the given definition. Ownership follows the Create Rule.
*/
JSClassRef JSClassCreate(const JSClassDefinition* definition);

/*!
@function
@abstract Retains a JavaScript class.
@param jsClass The JSClass to retain.
@result A JSClass that is the same as jsClass.
*/
JSClassRef JSClassRetain(JSClassRef jsClass);

/*!
@function
@abstract Releases a JavaScript class.
@param jsClass The JSClass to release.
*/
void JSClassRelease(JSClassRef jsClass);

/*!
@function
@abstract Creates a JavaScript object.
@param ctx The execution context to use.
@param jsClass The JSClass to assign to the object. Pass NULL to use the default object class.
@param data A void* to set as the object's private data. Pass NULL to specify no private data.
@result A JSObject with the given class and private data.
@discussion The default object class does not allocate storage for private data, so you must provide a non-NULL jsClass to JSObjectMake if you want your object to be able to store private data.

data is set on the created object before the intialize methods in its class chain are called. This enables the initialize methods to retrieve and manipulate data through JSObjectGetPrivate.
*/
JSObjectRef JSObjectMake(JSContextRef ctx, JSClassRef jsClass, void* data);

/*!
@function
@abstract Convenience method for creating a JavaScript function with a given callback as its implementation.
@param ctx The execution context to use.
@param name A JSString containing the function's name. This will be used when converting the function to string. Pass NULL to create an anonymous function.
@param callAsFunction The JSObjectCallAsFunctionCallback to invoke when the function is called.
@result A JSObject that is a function. The object's prototype will be the default function prototype.
*/
JSObjectRef JSObjectMakeFunctionWithCallback(JSContextRef ctx, JSStringRef name, JSObjectCallAsFunctionCallback callAsFunction);

/*!
@function
@abstract Creates a JavaScript Array object.
@param ctx The execution context to use.
@param argumentCount An integer count of the number of arguments in arguments.
@param arguments A JSValue array of data to populate the Array with. Pass NULL if argumentCount is 0.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care about exceptions.
@result A JSObject that is an Array.
@discussion The behavior of this function does not exactly match the behavior of the built-in Array constructor. Specifically, if one argument
is supplied, this function returns an array with one element.
*/
JSObjectRef JSObjectMakeArray(JSContextRef ctx, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

/*!
@function
@abstract Tests whether an object can be called as a function.
@param ctx The execution context to use.
@param object The JSObject to test.
@result true if the object can be called as a function, otherwise false.
*/
bool JSObjectIsFunction(JSContextRef ctx, JSObjectRef object);

/*!
@function
@abstract Calls an object as a function.
@param ctx The execution context to use.
@param object The JSObject to call as a function.
@param thisObject The object to use as "this," or NULL to use the global object as "this."
@param argumentCount An integer count of the number of arguments in arguments.
@param arguments A JSValue array of arguments to pass to the function. Pass NULL if argumentCount is 0.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care about exceptions.
@result The JSValue that results from calling object as a function, or NULL if an exception is thrown or object is not a function.
*/
JSValueRef JSObjectCallAsFunction(JSContextRef ctx, JSObjectRef object, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

/*!
@function
@abstract Gets a property from an object.
@param ctx The execution context to use.
@param object The JSObject whose property you want to get.
@param propertyName A JSString containing the property's name.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care about exceptions.
@result The property's value if object has the property, otherwise the undefined value.
*/
JSValueRef JSObjectGetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

/*!
@function
@abstract Sets a property on an object.
@param ctx The execution context to use.
@param object The JSObject whose property you want to set.
@param propertyName A JSString containing the property's name.
@param value A JSValue to use as the property's value.
@param attributes A logically ORed set of JSPropertyAttributes to give to the property.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care about exceptions.
*/
void JSObjectSetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSPropertyAttributes attributes, JSValueRef* exception);

/*!
@function
@abstract Tests whether an object has a given property.
@param ctx The execution context to use.
@param object The JSObject to test.
@param propertyName A JSString containing the property's name.
@result true if the object has a property whose name matches propertyName, otherwise false.
*/
bool JSObjectHasProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName);

/*!
@function
@abstract Deletes a property from an object.
@param ctx The execution context to use.
@param object The JSObject whose property you want to delete.
@param propertyName A JSString containing the property's name.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care about exceptions.
@result true if the delete operation succeeds, otherwise false (for example, if the property has the kJSPropertyAttributeDontDelete attribute set).
*/
bool JSObjectDeleteProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

/*!
@function
@abstract Gets an object's private data.
@param object A JSObject whose private data you want to get.
@result A void* that is the object's private data, if the object has private data, otherwise NULL.
*/
void* JSObjectGetPrivate(JSObjectRef object);

/*!
@function
@abstract Sets a pointer to private data on an object.
@param object The JSObject whose private data you want to set.
@param data A void* to set as the object's private data.
@result true if object can store private data, otherwise false.
@discussion The default object class does not allocate storage for private data. Only objects created with a non-NULL JSClass can store private data.
*/
bool JSObjectSetPrivate(JSObjectRef object, void* data);

#ifdef __cplusplus
}
#endif

#endif /* JSObjectRef_h */