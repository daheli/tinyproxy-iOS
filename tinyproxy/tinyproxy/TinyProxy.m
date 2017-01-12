//
//  TinyProxy.m
//  tinyproxy-iOS
//
//  Created by lidahe on 17/1/11.
//  Copyright © 2017年 lidahe. All rights reserved.
//

#import "TinyProxy.h"
#import "main.h"


int tinyproxy_main() {
    fprintf(stderr, "tinyproxy_main... \n");
    return tp_main(1, NULL);
}

