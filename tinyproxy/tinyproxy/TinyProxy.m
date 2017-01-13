//
//  TinyProxy.m
//  tinyproxy-iOS
//
//  Created by lidahe on 17/1/11.
//  Copyright © 2017年 lidahe. All rights reserved.
//

#import "TinyProxy.h"
#import "main.h"

char *TINYPROXY_SYSCONF;
char *TINYPROXY_BASE_DIR;
//TARGET_OS_IOS

int tinyproxy_main() {
    fprintf(stderr, "tinyproxy_main... \n");
    return tp_main(1, NULL);
}


@implementation TinyProxy

+ (void)start {
//    dispatch_queue_t queue_t = dispatch_queue_create([@"TinyProxy" UTF8String], DISPATCH_QUEUE_SERIAL);
//    dispatch_async(queue_t, ^{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self config];
        tp_main(1, "");
        NSLog(@"proxy finished.");
    });
}

+ (void)config {
    NSBundle *bundle = [NSBundle bundleForClass:[self class]];
    NSString *confFile = [bundle pathForResource:@"tinyproxy" ofType:@"conf"];
    NSLog(@"confFile:%@", confFile);
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *dir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSString *baseDir = [dir stringByAppendingPathComponent:@"/tinyproxy"];
    [fileManager createDirectoryAtPath:baseDir withIntermediateDirectories:YES attributes:nil error:nil];
    NSLog(@"baseDir:%@", baseDir);
    
    TINYPROXY_SYSCONF = [confFile UTF8String];    
    TINYPROXY_BASE_DIR = [baseDir UTF8String];
}

@end
