//
// Created by joeld on 4/7/13.
//
// To change the template use AppCode | Preferences | File Templates.
//

#include <EASTL/string.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <luddite/common/useful.h>
#include <luddite/render/scene.h>
#include <luddite/render/scene_objfile.h>
#include <luddite/platform/platform.h>
#include <luddite/render/render_device_gl.h>
#include <luddite/render/material_db.h>
#include <luddite/render/gbuff_prim.h>
#include <luddite/render/texture_info.h>
#include <luddite/render/color_util.h>


#include "scene_collect.h"


eastl::string TestApp::SceneCollect::sceneName()
{
    return "collect";
}


void TestApp::SceneCollect::init()
{
    // Set up luddite stuff
    luddite::RenderDeviceGL *renderDeviceGL = new luddite::RenderDeviceGL();
    m_renderDevice = renderDeviceGL;
    
    // Setup camera (TODO: do this differently)
    glhPerspectivef2( renderDeviceGL->matProjection, 20.0, 800.0/600.0, 1.0, 500.0 );
    matrix4x4f cameraXlate, cameraRot;
    cameraXlate.Translate(0.0, -4, -15.0);
    cameraRot.RotateX( 15.0 * (M_PI/180.0) );
    renderDeviceGL->matBaseModelView = cameraXlate * cameraRot;
    
    // Initialize shader DB
    m_mtlDB = new luddite::MaterialDB( );
    m_mtlDB->initShaderDB();
    
    // Add material def files
    m_mtlDB->addMaterialDefs("CollectGame.material.xml" );
    
    luddite::Material *mtl  = m_mtlDB->getNamedMaterial( m_renderDevice, "mtl.environment" );
    
    // Build scene graph
    m_worldRoot = new luddite::SceneNode( "worldRoot" );

    // Load the ground plane obj file in the center.
    m_groundPlane = scene_objfile_named( "grid10x10.obj", m_renderDevice, m_mtlDB );
    m_worldRoot->addChild( m_groundPlane );

    // Load the player mesh
    m_player = scene_objfile_named( "player.obj", m_renderDevice, m_mtlDB );
    m_worldRoot->addChild( m_player );

    // Load the trees
    luddite::SceneNode *tree = scene_objfile_named("tree_fir.obj", m_renderDevice, m_mtlDB );
    tree->m_pos = vec3f( 2.0, 0.0, 1.5 );
    m_worldRoot->addChild( tree );
    
    m_trees.push_back(tree);
    
    // Add tree instances
    for (int i=0; i <20; i++)
    {
        luddite::SceneNode *treeInst = tree->makeInstance();
        
        // Generate a random position for the tree, outside the center of the
        // map (TODO: also don't overlap other trees)
        do {
            vec3f treePos = vec3f( randUniform(-10.0, 10.0), 0.0, randUniform(-10.0, 10.0) );
            treeInst->m_pos = treePos;
        } while (prmath::LengthSquared(treeInst->m_pos) < 1.0 );

        m_worldRoot->addChild( treeInst );
        m_trees.push_back( treeInst );
    }
    
    // FIXME.. this is temporary, shouldn't need to do this (and we shouldn't
    // upload all the shaders, only used ones)
    m_mtlDB->useAllShaders( m_renderDevice );
    
    // Finally, create scene for our node graph
    m_scene = new luddite::Scene( m_worldRoot );


}

void TestApp::SceneCollect::render()
{
    glClearColor( 0.64f, 0.79f, 0.67f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Eval and render the scene
    m_scene->eval( m_renderDevice );
    m_renderDevice->renderFrame();
}
