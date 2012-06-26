/*
 * BatteryTech
 * Copyright (c) 2010 Battery Powered Games, LLC.
 *
 * This code is a component of BatteryTech and is subject to the 'BatteryTech
 * End User License Agreement'.  Among other important provisions, this
 * license prohibits the distribution of source code to anyone other than
 * authorized parties.  If you have any questions or would like an additional
 * copy of the license, please contact: support@batterypoweredgames.com
 */

//============================================================================
// Name        : QuadRenderer.cpp
// Description : Renders Quads
//============================================================================

#include "QuadRenderer.h"
#include <stdio.h>
#include <string.h>
#include "../Logger.h"
#include "GraphicsConfiguration.h"
#include "RenderContext.h"
#include "GLResourceManager.h"
#include "../Context.h"

namespace BatteryTech {

QuadRenderer::QuadRenderer(Context *context) {
	this->context = context;
}

QuadRenderer::~QuadRenderer() {
}

void QuadRenderer::init(BOOL32 newContext) {
	if (context->gConfig->useShaders) {
		// this is a particularly important renderer so make sure it's ready to go
		ShaderProgram *shaderProgram = context->glResourceManager->getShaderProgram("quad");
		if (!shaderProgram) {
			shaderProgram = new ShaderProgram("quad", "shaders/quadshader.vert", "shaders/quadshader.frag");
			context->glResourceManager->addShaderProgram("quad", shaderProgram);
		}
		shaderProgram->load(FALSE);
	}
	checkGLError("QuadRenderer Init");
}

void QuadRenderer::render(Texture *texture, F32 top, F32 right, F32 bottom, F32 left) {
	Vector2f uvs[] = { Vector2f(0,0), Vector2f(1,0),
			Vector2f(1,1), Vector2f(0,1)
	};
	F32 desWidth = right-left;
	F32 desHeight = bottom-top;
	F32 width = desWidth;
	F32 height = desHeight;
	Vector2f offset(0,0);
	if (texture) {
		texture->bind();
		Matrix4f tMat = texture->getMatrix();
		// transform the UVs into texture space (to support atlased images)
		for (S32 i = 0; i < 4; i++) {
			uvs[i] = tMat * uvs[i];
		}
		// take the dimensions and convert them into percentages of the desired draw size(scale)
		Vector2f scale = Vector2f(desWidth / texture->getOriginalSize().x, desHeight / texture->getOriginalSize().y);
		Vector2f actualSize = texture->getTrimmedSize() * scale;
		offset = texture->getCornerOffset() * scale;
		width = actualSize.x;
		height = actualSize.y;
	}
	F32 actualLeft = left + offset.x;
	F32 actualRight = actualLeft + width;
	F32 actualTop = top + offset.y;
	F32 actualBottom = actualTop + height;
	F32 verts[] = {
			actualLeft, actualTop, 0, actualRight, actualTop, 0, actualRight, actualBottom, 0, actualLeft, actualBottom, 0
	};
	//glFrontFace(GL_CW);
	if (context->gConfig->useShaders) {
		ShaderProgram *shaderProgram = context->glResourceManager->getShaderProgram("quad");
		shaderProgram->bind();
		glVertexAttribPointer(shaderProgram->getVertexAttributeLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, verts);
		glVertexAttribPointer(shaderProgram->getVertexAttributeLoc("uvMap"), 2, GL_FLOAT, GL_FALSE, 0, uvs);
		glUniformMatrix4fv(shaderProgram->getUniformLoc("projection_matrix"), 1, GL_FALSE, (GLfloat*) context->renderContext->projMatrix.data);
		glUniformMatrix4fv(shaderProgram->getUniformLoc("modelview_matrix"), 1, GL_FALSE, (GLfloat*) context->renderContext->mvMatrix.data);
		glUniform1i(shaderProgram->getUniformLoc("tex"), 0);
		Vector4f colorFilter = context->renderContext->colorFilter;
		glUniform4f(shaderProgram->getUniformLoc("colorFilter"), colorFilter.x,colorFilter.y,colorFilter.z,colorFilter.a);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		shaderProgram->unbind();
	} else {
		glVertexPointer(3, GL_FLOAT, 0, &verts);
		glTexCoordPointer(2, GL_FLOAT, 0, &uvs);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
}

void QuadRenderer::render(Texture *texture, F32 x, F32 y, F32 width, F32 height, F32 angleRads) {
	render(texture, Vector3f(x,y,0), angleRads, Vector4f(0,0,1,1), Vector2f(width,height), Vector4f(1,1,1,1), TRUE, FALSE, Matrix4f());
}

void QuadRenderer::render(Texture *texture, Vector3f pos, F32 angleRads, Vector4f myUvs, Vector2f scale, Vector4f colorFilter, BOOL32 isOpaque, BOOL32 bb, Matrix4f bbMat) {
	//glFrontFace(GL_CW);
	F32 x = pos.x;
	F32 y = pos.y;
	F32 z = pos.z;
	Vector2f uvs[] = { Vector2f(myUvs.x, myUvs.y), Vector2f(myUvs.z, myUvs.y),
			Vector2f(myUvs.z, myUvs.w), Vector2f(myUvs.x, myUvs.w)
	};
	F32 desWidth = scale.x;
	F32 desHeight = scale.y;
	F32 width = desWidth;
	F32 height = desHeight;
	Vector2f offsetDelta(0,0);
	if (texture) {
		texture->bind();
		Matrix4f tMat = texture->getMatrix();
		// transform the UVs into texture space (to support atlased images)
		for (S32 i = 0; i < 4; i++) {
			uvs[i] = tMat * uvs[i];
		}
		// take the dimensions and convert them into percentages of the desired draw size(scale)
		Vector2f scale = Vector2f(desWidth / texture->getOriginalSize().x, desHeight / texture->getOriginalSize().y);
		Vector2f actualSize = texture->getTrimmedSize() * scale;
		// this is the corner offset when the trimmed image is perfectly centered (which is how we lay it out by default)
		// so really what we want to know is the difference between that and the actual trim position
		Vector2f centerOffset = (texture->getOriginalSize() - texture->getTrimmedSize()) / 2.0f;
		// this is the difference between our default corner and the real corner scaled to the req draw size
		offsetDelta = (texture->getCornerOffset() - centerOffset) * scale;
		width = actualSize.x;
		height = actualSize.y;
	}
	F32 top;
	F32 bottom;
	F32 right = width/2 + offsetDelta.x;
	if (bb) {
		top = height/2 + offsetDelta.y;
		bottom = -height/2 + offsetDelta.y;
	} else {
		top = -height/2 + offsetDelta.y;
		bottom = height/2 + offsetDelta.y;
	}
	F32 left = -width/2 + offsetDelta.x;
	F32 verts[] = {
			left, top, z, right, top, z, right, bottom, z, left, bottom, z
	};
	if (context->gConfig->useShaders) {
		Matrix4f myMvMatrix = context->renderContext->mvMatrix;
		myMvMatrix.translate(x, y, z);
		myMvMatrix.rotate(angleRads * (180 / PI), 0, 0, -1.0f);
		if (bb) {
			myMvMatrix = myMvMatrix * bbMat;
		}
		ShaderProgram *shaderProgram = context->glResourceManager->getShaderProgram("quad");
		shaderProgram->bind();
		glVertexAttribPointer(shaderProgram->getVertexAttributeLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, verts);
		glVertexAttribPointer(shaderProgram->getVertexAttributeLoc("uvMap"), 2, GL_FLOAT, GL_FALSE, 0, uvs);
		glUniformMatrix4fv(shaderProgram->getUniformLoc("projection_matrix"), 1, GL_FALSE, (GLfloat*) context->renderContext->projMatrix.data);
		glUniformMatrix4fv(shaderProgram->getUniformLoc("modelview_matrix"), 1, GL_FALSE, (GLfloat*) myMvMatrix.data);
		glUniform1i(shaderProgram->getUniformLoc("tex"), 0);
		glUniform4f(shaderProgram->getUniformLoc("colorFilter"), colorFilter.x,colorFilter.y,colorFilter.z,colorFilter.a);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		shaderProgram->unbind();
	} else {
		// GL1 rendering branch
		Vector4f colorFilter = context->renderContext->colorFilter;
		glColor4f(colorFilter.x,colorFilter.y,colorFilter.z,colorFilter.a);
		glPushMatrix();
		glTranslatef(x, y, 0);
		glRotatef(angleRads * (180 / PI), 0, 0, 1.0f);
		glVertexPointer(3, GL_FLOAT, 0, &verts);
		glTexCoordPointer(2, GL_FLOAT, 0, &uvs);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glPopMatrix();
	}
}

}

