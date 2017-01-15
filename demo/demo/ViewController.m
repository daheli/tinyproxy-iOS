//
//  ViewController.m
//  demo
//
//  Created by lidahe on 17/1/11.
//  Copyright © 2017年 lidahe. All rights reserved.
//

#import "ViewController.h"
#import <tinyproxy/TinyProxy.h>

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

#define Log(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#define Exit(msg) \
Log("%s", msg);\
exit(-1);


@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
//    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
////        [self makeListen];
//    });
//    
//    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
//        [self requestBD];
//    });
    
    [TinyProxy start];
    
}

- (void)requestBD {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
//        NSURL *url = [NSURL URLWithString:@"https://www.baidu.com"];
        NSURL *url = [NSURL URLWithString:@"http://127.0.0.1:28888"];
        NSData *data = [NSData dataWithContentsOfURL:url];
        NSLog(@"url data:%@", data);
    });
}

- (int)makeListen {
    printf("Hello, World!\n");
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(fd < 0) {
        Log("create fd failed, err:%d", errno);
        return -1;
    }
    
    int ret = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK );    
    if(ret <0) {
        Log("set sock no-block failed, err:%d", errno);
        return -1;
    }
    int on = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if(ret <0) {
        Log("set sock addr reuse failed, err:%d", errno);
        return -1;
    }
    
    struct sockaddr_in si;
    si.sin_family = AF_INET;
    si.sin_port = htons(17777);
    si.sin_addr.s_addr = inet_addr("0.0.0.0");
    socklen_t sl = sizeof(si);
    
    ret = bind(fd, (struct sockaddr*)&si, sl);
    if(ret <0) {
        Log("set sock bind failed, err:%d", errno);
        return -1;
    }
    
    listen(fd, 1024);
    
    while(1) {
        struct sockaddr_in peer;
        socklen_t sp = sizeof(peer);
        ret = accept(fd, (struct sockaddr*)&peer, &sp);
        if (ret > 0) {
            Log("new link coming, peer %s:%d", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
        }
        sleep(1);
    }

}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
