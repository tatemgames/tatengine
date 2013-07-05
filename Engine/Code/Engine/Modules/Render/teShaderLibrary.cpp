/*
 *  teShaderLibrary.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 01/31/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#include "teShaderLibrary.h"

#ifdef TE_RENDER_GL_SHADERS

//uniform float teTime;\n\
//tePosition.xz = tePosition.xz + vec2(sin(teTime / 5000.0), cos(teTime / 5000.0)) * tePosition.y / 10.0;\n\

const char * shaderDefault = "\
\
#if defined TE_VS \n\
attribute TE_TEXP vec3 tePosition;\n\
attribute TE_TEXP vec2 teUV0;\n\
uniform TE_TEXP mat4 teMVPmat;\n\
varying TE_TEXP vec2 TexCoord0;\n\
void main()\n\
{\n\
gl_Position = teMVPmat * vec4(tePosition, 1);\n\
TexCoord0 = teUV0;\n\
}\n\
#endif\n\
#if defined TE_FS \n\
uniform sampler2D teTexture0;\n\
uniform vec4 teDiffuseColor;\n\
varying TE_TEXP vec2 TexCoord0;\n\
void main()\n\
{\n\
vec4 Diff0 = texture2D(teTexture0, TexCoord0);\n\
gl_FragColor = Diff0 * teDiffuseColor;\n\
}\n\
#endif";

const char * shaderSprite = "\
\
#if defined TE_VS \n\
attribute vec3 tePosition;\n\
attribute TE_TEXP vec2 teUV0;\n\
attribute vec4 teColor;\n\
uniform mat4 teMVPmat;\n\
varying TE_TEXP vec2 TexCoord0;\n\
varying vec4 Color;\n\
void main()\n\
{\n\
	gl_Position = teMVPmat * vec4(tePosition, 1);\n\
	TexCoord0 = teUV0;\n\
	Color = teColor;\n\
}\n\
#endif\n\
#if defined TE_FS \n\
uniform sampler2D teTexture0;\n\
uniform vec4 teDiffuseColor;\n\
varying TE_TEXP vec2 TexCoord0;\n\
varying vec4 Color;\n\
void main()\n\
{\n\
	vec4 Diff0 = texture2D(teTexture0, TexCoord0);\n\
	gl_FragColor = Diff0 * Color * teDiffuseColor;\n\
}\n\
#endif";

const char * shaderFailsafe = "\
\
#if defined TE_VS \n\
attribute vec3 tePosition;\n\
uniform mat4 teMVPmat;\n\
void main()\n\
{\n\
	gl_Position = teMVPmat * vec4(tePosition, 1);\n\
}\n\
#endif\n\
#if defined TE_FS \n\
uniform vec4 teDiffuseColor;\n\
void main()\n\
{\n\
	gl_FragColor = teDiffuseColor;\n\
}\n\
#endif";

const char * shaderLightmap = "\
\
#if defined TE_VS\n\
attribute vec3 tePosition;\n\
attribute TE_TEXP vec2 teUV0;\n\
attribute TE_TEXP vec2 teUV1;\n\
attribute vec4 teColor;\n\
uniform mat4 teMVPmat;\n\
varying TE_TEXP vec2 TexCoord0;\n\
varying TE_TEXP vec2 TexCoord1;\n\
varying vec4 Color;\n\
void main()\n\
{\n\
	gl_Position = teMVPmat * vec4(tePosition, 1);\n\
	TexCoord0 = teUV0;\n\
	TexCoord1 = teUV1;\n\
	Color = teColor;\n\
}\n\
#endif\n\
#if defined TE_FS\n\
\n\
uniform sampler2D teTexture0;\n\
uniform sampler2D teTexture1;\n\
uniform vec4 teDiffuseColor;\n\
varying TE_TEXP vec2 TexCoord0;\n\
varying TE_TEXP vec2 TexCoord1;\n\
varying vec4 Color;\n\
void main()\n\
{\n\
	vec4 Diff0 = texture2D(teTexture0, TexCoord0);\n\
	vec4 Diff1 = texture2D(teTexture1, TexCoord1);\n\
	gl_FragColor = Diff0 * Diff1 * Color * teDiffuseColor;\n\
}\n\
#endif";

const char * shaderSSAO = "\
\
#if defined TE_VS\n\
attribute vec3 tePosition;\n\
attribute TE_TEXP vec2 teUV0;\n\
uniform mat4 teMVPmat;\n\
varying TE_TEXP vec2 TexCoord0;\n\
void main()\n\
{\n\
	gl_Position = teMVPmat * vec4(tePosition, 1);\n\
	TexCoord0 = vec2(teUV0.x, 1.0 - teUV0.y);\n\
}\n\
#endif\n\
#if defined TE_FS\n\
\n\
uniform sampler2D teTexture0;\n\
uniform sampler2D teTexture1;\n\
uniform sampler2D teTexture2;\n\
varying TE_TEXP vec2 TexCoord0;\n\
float getZ(vec2 pos)\n\
{\n\
	const float zFar     = 50.0;\n\
	const float zNear    = 2.0;\n\
	\n\
	float zb = texture2D(teTexture1, pos).x;\n\
	float z = zFar * zNear / (zb * (zFar - zNear) - zFar);\n\
	return z;\n\
	\n\
}\n\
vec3 rndTable[16] = vec3[16](vec3(0.53812504, 0.18565957, -0.43192),vec3(0.13790712, 0.24864247, 0.44301823),vec3(0.33715037, 0.56794053, -0.005789503),vec3(-0.6999805, -0.04511441, -0.0019965635),vec3(0.06896307, -0.15983082, -0.85477847),vec3(0.056099437, 0.006954967, -0.1843352),vec3(-0.014653638, 0.14027752, 0.0762037),vec3(0.010019933, -0.1924225, -0.034443386),vec3(-0.35775623, -0.5301969, -0.43581226),vec3(-0.3169221, 0.106360726, 0.015860917),vec3(0.010350345, -0.58698344, 0.0046293875),vec3(-0.08972908, -0.49408212, 0.3287904),vec3(0.7119986, -0.0154690035, -0.09183723),vec3(-0.053382345, 0.059675813, -0.5411899),vec3(0.035267662, -0.063188605, 0.54602677),vec3(-0.47761092, 0.2847911, -0.0271716));\n\
//vec4 rndTable[8] = vec4[8](vec4( -0.5, -0.5, -0.5, 0.0 ),vec4(  0.5, -0.5, -0.5, 0.0 ),vec4( -0.5,  0.5, -0.5, 0.0 ),vec4(  0.5,  0.5, -0.5, 0.0 ),vec4( -0.5, -0.5,  0.5, 0.0 ),vec4(  0.5, -0.5,  0.5, 0.0 ),vec4( -0.5,  0.5,  0.5, 0.0 ),vec4(  0.5,  0.5,  0.5, 0.0 ));\n\
void main()\n\
{\n\
	vec4 diff = texture2D(teTexture0, TexCoord0);\n\
	\n\
	const float radius   = 0.4;\n\
	const float attBias  = 0.2;\n\
	const float attScale = 2.0;\n\
	const float distScale = 1.0;\n\
	\n\
	\n\
	float z = getZ(TexCoord0);\n\
	float att = 0.0;\n\
	vec3 plane = 2.0 * texture2D(teTexture2, TexCoord0 * 2048.0 / 1.0).xyz - vec3(1.0);\n\
	\n\
	for(int i = 0; i < 8; i++)\n\
	{\n\
		vec3 sample = reflect(rndTable[i].xyz, plane);\n\
		float zSample = getZ(TexCoord0 + radius*sample.xy / z);\n\
		\n\
		float dist = max(zSample - z, 0.0) / distScale;\n\
		float occl = 30.0 * max(dist * (2.0 - dist), 0.0);\n\
		\n\
		att += 1.0 / (1.0 + occl * occl);\n\
	}\n\
	\n\
	att = pow(att / 8.0 + 0.2, 4.0);\n\
	att = clamp((att + attBias) * attScale, 0.0, 1.0);\n\
	gl_FragColor = diff * vec4(vec3(att), 1);\n\
}\n\
#endif";

const char * shaderSkin = "\
\
#if defined TE_VS\n\
attribute vec3 tePosition;\n\
attribute TE_TEXP vec2 teUV0;\n\
attribute vec4 teBonesIds;\n\
attribute vec4 teBonesW;\n\
uniform mat4 teMVPmat;\n\
const int BoneCount = 64;\n\
uniform vec4 teBoneMatrixes[BoneCount];\n\
varying TE_TEXP vec2 TexCoord0;\n\
void main()\n\
{\n\
	vec4 ip = vec4(tePosition, 1);\n\
	ivec4 ids = ivec4(teBonesIds);\n\
	vec4 a = \n\
		  teBoneMatrixes[ids.x * 2 + 0] * teBonesW.x\n\
		+ teBoneMatrixes[ids.y * 2 + 0] * teBonesW.y\n\
		+ teBoneMatrixes[ids.z * 2 + 0] * teBonesW.z\n\
		+ teBoneMatrixes[ids.w * 2 + 0] * teBonesW.w;\n\
	vec4 b = \n\
		  teBoneMatrixes[ids.x * 2 + 1] * teBonesW.x\n\
		+ teBoneMatrixes[ids.y * 2 + 1] * teBonesW.y\n\
		+ teBoneMatrixes[ids.z * 2 + 1] * teBonesW.z\n\
		+ teBoneMatrixes[ids.w * 2 + 1] * teBonesW.w;\n\
	float aLength = length(a);\n\
	a /= aLength;\n\
	b /= aLength;\n\
	mat4 bone = mat4(\n\
		1.0-2.0*(a.y*a.y+a.z*a.z),\n\
		2.0*(a.x*a.y+a.z*a.w),\n\
		2.0*(a.x*a.z-a.y*a.w),\n\
		0.0,\n\
		\n\
		2.0*(a.x*a.y-a.z*a.w),\n\
		1.0-2.0*(a.x*a.x+a.z*a.z),\n\
		2.0*(a.z*a.y+a.x*a.w),\n\
		0.0,\n\
		\n\
		2.0*(a.x*a.z+a.y*a.w),\n\
		2.0*(a.z*a.y-a.x*a.w),\n\
		1.0-2.0*(a.x*a.x+a.y*a.y),\n\
		0.0,\n\
		\n\
		2.0*(-b.w*a.x + b.x*a.w - b.y*a.z + b.z*a.y),\n\
		2.0*(-b.w*a.y + b.x*a.z + b.y*a.w - b.z*a.x),\n\
		2.0*(-b.w*a.z - b.x*a.y + b.y*a.x + b.z*a.w),\n\
		1.0\n\
		);\n\
	gl_Position = teMVPmat * (bone * ip);\n\
	TexCoord0 = teUV0;\n\
}\n\
#endif\n\
#if defined TE_FS\n\
\n\
uniform sampler2D teTexture0;\n\
uniform vec4 teDiffuseColor;\n\
varying vec2 TexCoord0;\n\
void main()\n\
{\n\
	vec4 Diff0 = texture2D(teTexture0, TexCoord0);\n\
	gl_FragColor = Diff0 * teDiffuseColor;\n\
}\n\
#endif";

const te::c8 * shaderFontDistanceField = "\
\
#if defined TE_VS \n\
attribute vec3 tePosition;\n\
attribute TE_TEXP vec2 teUV0;\n\
attribute vec4 teColor;\n\
uniform mat4 teMVPmat;\n\
varying TE_TEXP vec2 TexCoord0;\n\
varying vec4 Color;\n\
void main()\n\
{\n\
	gl_Position = teMVPmat * vec4(tePosition, 1);\n\
	TexCoord0 = teUV0;\n\
	Color = teColor;\n\
}\n\
#endif\n\
#if defined TE_FS \n\
uniform sampler2D teTexture0;\n\
uniform vec4 teDiffuseColor;\n\
uniform float teUser0;\n\
varying TE_TEXP vec2 TexCoord0;\n\
varying vec4 Color;\n\
void main()\n\
{\n\
	float a = texture2D(teTexture0, TexCoord0).a;\n\
	vec4 Diff0 = vec4(1.0, 1.0, 1.0, 0.0);\n\
	if(a > (0.5 - teUser0 / 2.5))\n\
		Diff0.a = 1.0;\n\
	Diff0.a = Diff0.a * smoothstep(0.5 - teUser0, 0.5 + teUser0, a);\n\
	gl_FragColor = Diff0 * Color * teDiffuseColor;\n\
}\n\
#endif";

#endif

namespace te
{
	namespace video
	{
		void teShaderLibrary::BuildShaders()
		{
			#ifdef TE_RENDER_GL_SHADERS
			shader[ST_DEFAULT].SetSource(shaderDefault);
			shader[ST_SPRITE].SetSource(shaderSprite);
			shader[ST_FAILSAFE].SetSource(shaderFailsafe);
			shader[ST_LIGHT_MAP].SetSource(shaderLightmap);
			shader[ST_SSAO].SetSource(shaderSSAO);
			shader[ST_SKIN].SetSource(shaderSkin);
			shader[ST_FONT_DF].SetSource(shaderFontDistanceField);
			#endif
		}
	}
}
