//
//  TKViewController.h
//  mesh
//
//  Created by Joel Davis on 1/6/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

#import "TKGameViewController.h"

#import "MeshGame.h"

@interface TKMeshExampleViewController : TKGameViewController

@property (nonatomic, strong) MeshGame* meshGame;

@end
