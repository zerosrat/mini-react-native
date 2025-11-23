#import <Foundation/Foundation.h>

// C函数接口，供C++代码调用
const char* getBundlePath() {
    @autoreleasepool {
        NSBundle* mainBundle = [NSBundle mainBundle];
        if (mainBundle) {
            NSString* bundlePath = [mainBundle bundlePath];
            if (bundlePath) {
                // 返回静态字符串，避免内存管理问题
                static char pathBuffer[1024];
                strncpy(pathBuffer, [bundlePath UTF8String], sizeof(pathBuffer) - 1);
                pathBuffer[sizeof(pathBuffer) - 1] = '\0';
                return pathBuffer;
            }
        }
        return NULL;
    }
}

const char* getResourcePath(const char* resourceName) {
    @autoreleasepool {
        if (!resourceName) {
            return NULL;
        }

        NSBundle* mainBundle = [NSBundle mainBundle];
        if (mainBundle) {
            NSString* fileName = [NSString stringWithUTF8String:resourceName];
            NSString* resourcePath = [mainBundle pathForResource:fileName ofType:nil];
            if (resourcePath) {
                // 返回静态字符串，避免内存管理问题
                static char pathBuffer[1024];
                strncpy(pathBuffer, [resourcePath UTF8String], sizeof(pathBuffer) - 1);
                pathBuffer[sizeof(pathBuffer) - 1] = '\0';
                return pathBuffer;
            }
        }
        return NULL;
    }
}