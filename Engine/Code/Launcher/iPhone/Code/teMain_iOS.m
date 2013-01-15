//
//  teMainiPhone.m
//  TatEngine
//
//  Created by Dmitrii Ivanov on 11/23/09.
//  Copyright 2009 Tatem Games. All rights reserved.
//

#include "TatEngineCoreConfig.h"

#import <UIKit/UIKit.h>

int main(int argc, char * argv[])
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, nil);
    [pool release];
    return retVal;
}
