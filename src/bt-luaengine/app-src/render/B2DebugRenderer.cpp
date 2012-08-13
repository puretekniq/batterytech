
/*
 * B2DebugRenderer.cpp
 *
 *  Created on: Oct 30, 2010
 *      Author: rgreen
 */

#ifdef BATTERYTECH_INCLUDE_BOX2D

#include "B2DebugRenderer.h"
#include <batterytech/render/Renderer.h>
#include <batterytech/platform/platformgl.h>
#include <batterytech/math/Triangulator.h>
#include <bt-box2d/Dynamics/b2Body.h>
#include <bt-box2d/Dynamics/b2World.h>
#include <bt-box2d/Dynamics/b2Fixture.h>
#include <bt-box2d/Collision/Shapes/b2Shape.h>
#include <math.h>
#include "../GameConstants.h"
#include "../gameobject/GameObject.h"
#include "../World.h"
#include <batterytech/render/GLResourceManager.h>
#include <batterytech/render/ShaderProgram.h>
#include <batterytech/render/RenderContext.h>

#define CIRCLE_TESSELATION 12

static ShaderProgram *shader = NULL;

B2DebugRenderer::B2DebugRenderer(GameContext *context) {
	this->context = context;
}

B2DebugRenderer::~B2DebugRenderer() {
}

void B2DebugRenderer::init(BOOL32 newGameContext) {
	ShaderProgram *shader = context->glResourceManager->getShaderProgram("line");
	if (!shader) {
		shader = new ShaderProgram("line", "shaders/lineshader.vert", "shaders/lineshader.frag");
		context->glResourceManager->addShaderProgram("line", shader);
	}
}

void B2DebugRenderer::render(World *world) {
    if (!world->boxWorld) {
        return;
    }
	if (!context->gConfig->useShaders) {
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
        shader = context->glResourceManager->getShaderProgram("line");
        if (shader) {
            shader->bind();
        } else {
            // no shader
            return;
        }
    }
	b2Body *nextBody = world->boxWorld->GetBodyList();
	while (nextBody) {
		if (nextBody->GetUserData()) {
			GameObject *obj = (GameObject*) nextBody->GetUserData();
			glColor4f(1, 1, 1, 1);
		} else {
			glColor4f(1,1,1,1);
		}
		b2Fixture *nextFixture = nextBody->GetFixtureList();
		while (nextFixture) {
			b2Shape *shape = nextFixture->GetShape();
			if (shape->GetType() == b2Shape::e_circle) {
				// draw circle
				renderCircleShape(nextBody, (b2CircleShape*) shape);
			} else if (shape->GetType() == b2Shape::e_polygon) {
				// draw polygon
				renderPolyShape(nextBody, (b2PolygonShape*) shape);
			}
			renderOrientationLine(nextBody);
			nextFixture = nextFixture->GetNext();
		}
		nextBody = nextBody->GetNext();
	}
	if (!context->gConfig->useShaders) {
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void B2DebugRenderer::renderPolyShape(b2Body *body, b2PolygonShape *polyShape) {
	F32 angle = body->GetAngle();
	//F32 aVelocity = body->GetAngularVelocity();
	b2Vec2 pos = body->GetPosition();
	S32 vertCount = polyShape->GetVertexCount();
	// 12 is max verts for a polyshape
    Vector3f verts[12];
    Vector4f colors[12];
    S32 indices[24];
    for (S32 i = 0; i < vertCount; i++) {
        indices[i*2] = i;
        if (i == vertCount - 1) {
            indices[i*2+1] = 0;
        } else {
            indices[i*2+1] = i + 1;
        }
        colors[i] = Vector4f(1,1,1,1);
    }
	for (S32 i = 0; i < vertCount; i++) {
		b2Vec2 b2v = polyShape->GetVertex(i);
        verts[i] = Vector3f(b2v.x, b2v.y, 0);
	}
	glFrontFace(GL_CCW);
	if (context->gConfig->useShaders) {
        Matrix4f myMv = context->renderContext->mvMatrix;
        myMv.translate(pos.x, pos.y, 0);
        myMv.rotate(angle * (180 / PI), 0, 0, 1.0f);
		// mv + proj, position and color
        glUniformMatrix4fv(shader->getUniformLoc("modelview_matrix"), 1, FALSE, myMv.data);
        glUniformMatrix4fv(shader->getUniformLoc("projection_matrix"), 1, FALSE, context->renderContext->projMatrix.data);
		glVertexAttribPointer(shader->getVertexAttributeLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, verts);
		glVertexAttribPointer(shader->getVertexAttributeLoc("vColor"), 4, GL_FLOAT, GL_FALSE, 0, colors);
        glDrawElements(GL_LINES, vertCount*2, GL_UNSIGNED_SHORT, indices);
	} else {
		glVertexPointer(3, GL_FLOAT, 0, verts);
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(angle * (180 / PI), 0, 0, 1.0f);
        glDrawElements(GL_LINES, vertCount*2, GL_UNSIGNED_SHORT, indices);
		glPopMatrix();
	}
}

void B2DebugRenderer::renderCircleShape(b2Body *body, b2CircleShape *shape) {
	F32 angle = body->GetAngle();
	b2Vec2 pos = body->GetPosition();
	S32 vertCount = CIRCLE_TESSELATION;
	F32 radsPerSegment = TAU / CIRCLE_TESSELATION;
    Vector3f verts[CIRCLE_TESSELATION];
    Vector4f colors[CIRCLE_TESSELATION];
    S32 indices[CIRCLE_TESSELATION*2];
    for (S32 i = 0; i < CIRCLE_TESSELATION; i++) {
        indices[i*2] = i;
        if (i == vertCount - 1) {
            indices[i*2+1] = 0;
        } else {
            indices[i*2+1] = i + 1;
        }
        colors[i] = Vector4f(1,1,1,1);
    }
	for (S32 i = 0; i < vertCount; i++) {
		F32 thisX = cos(radsPerSegment * i) * shape->m_radius;
		F32 thisY = sin(radsPerSegment * i) * shape->m_radius;
        verts[i] = Vector3f(thisX, thisY, 0);
	}
    glFrontFace(GL_CCW);
	if (context->gConfig->useShaders) {
        Matrix4f myMv = context->renderContext->mvMatrix;
        myMv.translate(pos.x, pos.y, 0);
        myMv.rotate(angle * (180 / PI), 0, 0, 1.0f);
		// mv + proj, position and color
        glUniformMatrix4fv(shader->getUniformLoc("modelview_matrix"), 1, FALSE, myMv.data);
        glUniformMatrix4fv(shader->getUniformLoc("projection_matrix"), 1, FALSE, context->renderContext->projMatrix.data);
		glVertexAttribPointer(shader->getVertexAttributeLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, verts);
		glVertexAttribPointer(shader->getVertexAttributeLoc("vColor"), 4, GL_FLOAT, GL_FALSE, 0, colors);
        glDrawElements(GL_LINES, vertCount*2, GL_UNSIGNED_SHORT, indices);
	} else {
		glVertexPointer(3, GL_FLOAT, 0, verts);
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(angle * (180 / PI), 0, 0, 1.0f);
        glDrawElements(GL_LINES, vertCount*2, GL_UNSIGNED_SHORT, indices);
		glPopMatrix();
	}
}

void B2DebugRenderer::renderOrientationLine(b2Body *body) {
	glColor4f(0, 1, 0.25f, 1);
	F32 angle = body->GetAngle();
	b2Vec2 pos = body->GetPosition();
	F32 glVerts[6];
	glVerts[0] = 0;
	glVerts[1] = 0;
	glVerts[2] = 0;
	glVerts[3] = 0;
	glVerts[4] = 0.5f;
	glVerts[5] = 0;
	if (context->gConfig->useShaders) {
		// mv + proj
		// v position
		// v color
	} else {
		glVertexPointer(3, GL_FLOAT, 0, &glVerts);
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(angle * (180 / PI), 0, 0, 1.0f);
		glDrawArrays(GL_LINES, 0, 2);
		glPopMatrix();
	}
}

void B2DebugRenderer::renderLine(F32 x, F32 y, F32 angle, F32 length) {
	F32 glVerts[6];
	glVerts[0] = 0;
	glVerts[1] = 0;
	glVerts[2] = 0;
	glVerts[3] = 0;
	glVerts[4] = length;
	glVerts[5] = 0;
	if (context->gConfig->useShaders) {
		// mv + proj
		// v position
		// v color
	} else {
		glVertexPointer(3, GL_FLOAT, 0, &glVerts);
		glPushMatrix();
		glTranslatef(x, y, 0);
		glRotatef(angle * (180 / PI), 0, 0, 1.0f);
		glDrawArrays(GL_LINES, 0, 2);
		glPopMatrix();
	}
}

void B2DebugRenderer::renderDrawLines(ManagedArray<Vector2f> *points) {
	glColor4f(0, 1, 0, 1);
	S32 vertCount = points->getSize() - 1;
	// 200 is max
	F32 glVerts[200 * 3 * 2];
	S32 i;
	Vector2f vert2;
	for (i = 0; i < vertCount; i++) {
		Vector2f vert = *(points->array[i]);
		vert2 = *(points->array[i + 1]);
		glVerts[i * 6] = vert.x;
		glVerts[i * 6 + 1] = vert.y;
		glVerts[i * 6 + 2] = 0;
		glVerts[i * 6 + 3] = vert2.x;
		glVerts[i * 6 + 4] = vert2.y;
		glVerts[i * 6 + 5] = 0;
	}
	if (context->gConfig->useShaders) {
		// mv + proj
		// v position
		// v color
	} else {
		glFrontFace(GL_CCW);
		glVertexPointer(3, GL_FLOAT, 0, &glVerts);
		glPushMatrix();
		glDrawArrays(GL_LINES, 0, vertCount * 2);
		glPopMatrix();
		glColor4f(1, 1, 1, 1);
	}
}

#endif
