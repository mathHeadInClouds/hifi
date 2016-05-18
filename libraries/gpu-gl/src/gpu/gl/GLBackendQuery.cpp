//
//  GLBackendQuery.cpp
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 7/7/2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GLBackend.h"
#include "GLBackendShared.h"

using namespace gpu;
using namespace gpu::gl;

GLBackend::GLQuery::GLQuery() {}

GLBackend::GLQuery::~GLQuery() {
    if (_qo != 0) {
        glDeleteQueries(1, &_qo);
    }
}

GLBackend::GLQuery* GLBackend::syncGPUObject(const Query& query) {
    GLQuery* object = Backend::getGPUObject<GLBackend::GLQuery>(query);

    // If GPU object already created and in sync
    if (object) {
        return object;
    }

    // need to have a gpu object?
    if (!object) {
        GLuint qo;
        glGenQueries(1, &qo);
        (void) CHECK_GL_ERROR();
        GLuint64 result = -1;

        // All is green, assign the gpuobject to the Query
        object = new GLQuery();
        object->_qo = qo;
        object->_result = result;
        Backend::setGPUObject(query, object);
    }

    return object;
}



GLuint GLBackend::getQueryID(const QueryPointer& query) {
    if (!query) {
        return 0;
    }
    GLQuery* object = GLBackend::syncGPUObject(*query);
    if (object) {
        return object->_qo;
    } else {
        return 0;
    }
}

void GLBackend::do_beginQuery(Batch& batch, size_t paramOffset) {
    auto query = batch._queries.get(batch._params[paramOffset]._uint);
    GLQuery* glquery = syncGPUObject(*query);
    if (glquery) {
        glBeginQuery(GL_TIME_ELAPSED, glquery->_qo);
        (void)CHECK_GL_ERROR();
    }
}

void GLBackend::do_endQuery(Batch& batch, size_t paramOffset) {
    auto query = batch._queries.get(batch._params[paramOffset]._uint);
    GLQuery* glquery = syncGPUObject(*query);
    if (glquery) {
        glEndQuery(GL_TIME_ELAPSED);
        (void)CHECK_GL_ERROR();
    }
}

void GLBackend::do_getQuery(Batch& batch, size_t paramOffset) {
    auto query = batch._queries.get(batch._params[paramOffset]._uint);
    GLQuery* glquery = syncGPUObject(*query);
    if (glquery) { 
        glGetQueryObjectui64v(glquery->_qo, GL_QUERY_RESULT_AVAILABLE, &glquery->_result);
        if (glquery->_result == GL_TRUE) {
            glGetQueryObjectui64v(glquery->_qo, GL_QUERY_RESULT, &glquery->_result);
            query->triggerReturnHandler(glquery->_result);
        }
        (void)CHECK_GL_ERROR();
    }
}

void GLBackend::resetQueryStage() {
}
