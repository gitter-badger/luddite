//
//  render_device_es2.cpp
//  luddite_ios
//
//  Created by Joel Davis on 7/29/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#include <luddite/render/render_device_es2.h>

// offsetof() gives a warning about non-POD types with xcode, so use these old
// school macros. This is OK because while VertType is non-POD, it doesn't have
// a vtable so these still work.
#define offset_d(i,f)    (long(&(i)->f) - long(i))
#define offset_s(t,f)    offset_d((t*)1000, f)

using namespace luddite;

void RenderDeviceES2::_drawGBatch( luddite::GBatch *gbatch )
{
    luddite::GBuff *gbuff = gbatch->m_gbuff;
    
    // Set transform and shader params from gbatch
    // TODO
    
    // Create gbo for this gbuff if not set up
    if (gbuff->m_vbo==0)
    {
        glGenBuffers( 1, &(gbuff->m_vbo) );
        
        glBindBuffer( GL_ARRAY_BUFFER, gbuff->m_vbo );
        glBufferData( GL_ARRAY_BUFFER, gbuff->m_vertData.size() * sizeof( DrawVert ), 
                     gbuff->m_vertData.data(), GL_STATIC_DRAW );
        
    }
    else
    {
        // Buffer is already created, just bind it
        glBindBuffer( GL_ARRAY_BUFFER, gbuff->m_vbo );
    }
    
    // Bind buffer data
    glEnableVertexAttribArray( VertexAttrib_TEXCOORD );
    glVertexAttribPointer( VertexAttrib_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_st ) );
    
    glEnableVertexAttribArray( VertexAttrib_POSITION );
    glVertexAttribPointer( VertexAttrib_POSITION, 3, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_pos) );


    glEnableVertexAttribArray( VertexAttrib_NORMAL );
    glVertexAttribPointer( VertexAttrib_NORMAL, 3, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_nrm) );
    
    glEnableVertexAttribArray( VertexAttrib_COLOR );
    glVertexAttribPointer( VertexAttrib_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_color) );
    
    // Draw it!
    glDrawArrays(GL_TRIANGLES, 0, gbuff->m_vertData.size() );

}
