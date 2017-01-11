//
//  TinyProxy.m
//  tinyproxy-iOS
//
//  Created by lidahe on 17/1/11.
//  Copyright © 2017年 lidahe. All rights reserved.
//

#import "tinyproxy.h"
#import "main.h"


int tinyproxy_main() {
#ifndef NDEBUG
        fprintf(stderr, "tinyproxy_main... \n");
#endif
    return tp_main(1, NULL);
}

