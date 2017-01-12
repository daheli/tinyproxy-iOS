//
//  TinyProxy.m
//  tinyproxy-iOS
//
//  Created by lidahe on 17/1/11.
//  Copyright © 2017年 lidahe. All rights reserved.
//

#import "TinyProxy.h"
#import "main.h"

char *SYSCONF;
char *LOCALSTATE_LOG;
char *LOCALSTATE_RUN;
//TARGET_OS_IOS

int tinyproxy_main() {
    fprintf(stderr, "tinyproxy_main... \n");
    return tp_main(1, NULL);
}


@implementation TinyProxy

+ (void)start {
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
    NSString *logDir = [baseDir stringByAppendingPathComponent:@"/log"];
    [fileManager createDirectoryAtPath:logDir withIntermediateDirectories:YES attributes:nil error:nil];
    NSString *logFile = [baseDir stringByAppendingPathComponent:@"/tinyproxy.log"];
    NSString *runFile = [baseDir stringByAppendingPathComponent:@"/run.pid"];
    NSLog(@"baseDir:%@", baseDir);
    
    SYSCONF = [confFile UTF8String];
    LOCALSTATE_LOG = [logFile UTF8String];
    LOCALSTATE_RUN = [runFile UTF8String];
}

@end
