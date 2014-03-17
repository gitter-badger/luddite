//
//  emitter.cpp
//  luddite
//
//  Created by Joel Davis on 2/25/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#include "luddite/common/useful.h"
#include "luddite/render/emitter.h"
#include "luddite/render/particle.h"
#include "luddite/render/particle_group.h"

using namespace luddite;

EmitterNode::EmitterNode( SceneNode *parent ) :
        SceneNode( parent )
{
    
}

EmitterNode::EmitterNode( const std::string &name, SceneNode *parent ) :
            SceneNode( name, parent )
{
    
}


void EmitterNode::emitIntoGroup( luddite::ParticleGroup *group, float dt )
{
    // number of parts to emit
    size_t numParts = 0;
    float numPartsReal = m_rate * dt;
    
    // Treat fractional particles as a probability -- e.g. .3 particles
    // means 30% chance to emit
    float numPartsF = floor(numPartsReal);
    float partProb = numPartsReal - numPartsF;
    numParts = (size_t)numPartsF;
    
    if (randUniform() < partProb)
    {
        numParts++;
    }
    
    // Emit the particles
    Particle part;
    for (size_t i=0; i < numParts; i++)
    {
        // TODO: set up particle
        part.m_pos = GLKVector3Make( 0.0, 0.0, 0.0 ); // TODO: use our world loc
        part.m_vel = GLKVector3Make( ((randUniform()*2.0) - 1.0) * 0.3,
                                     randUniform() * 2.0,
                                    ((randUniform()*2.0) - 1.0)  * 0.3 ); // TODO: calculate this right
        part.m_vel = GLKVector3Normalize( part.m_vel );
        part.m_vel = GLKVector3MultiplyScalar( part.m_vel, randUniform( 3.0, 10.0 ) );
        
        group->addParticle( part );
        if (group->isFull()) break;
    }
    
    printf("EmitterNode: Now has %zu particles...\n", group->particleCount() );
}
