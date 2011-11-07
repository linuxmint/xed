#ifndef PLUMA_OSX_DELEGATE_H_
#define PLUMA_OSX_DELEGATE_H_

#import <Foundation/NSAppleEventManager.h>

@interface PlumaOSXDelegate : NSObject
{
}

-(id) init;
-(void) openFiles:(NSAppleEventDescriptor*)event
        withReply:(NSAppleEventDescriptor*)reply;

@end

#endif /* PLUMA_OSX_DELEGATE_H_ */
