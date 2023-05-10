#ifndef __SQUARE_H_
#define __SQUARE_H_

#include <string>

static const std::string squareVertexShaderSrc = R"(
#version 430 core

/** Inputs */
in vec2 aPosition;
in vec3 aColor;
in vec2 aTexcoord;

/** Outputs */
out vec3 vsColor;
out vec2 vsTexcoord;


/**We specify our uniforms. We do not need to specify locations manually, 
but it helps with knowing what is bound where.*/
uniform mat4 u_TransformationMat = mat4(1);
uniform mat4 u_ViewMat           = mat4(1);
uniform mat4 u_ProjectionMat     = mat4(1);


void main()
{
//Pass the color and texture data for the fragment shader
vsColor = aColor;
vsTexcoord = aTexcoord;
//We multiply our matrices with our position to change the positions of vertices to their final destinations.
gl_Position = u_ProjectionMat * u_ViewMat * u_TransformationMat * vec4(aPosition, 0.0f, 1.0f);
}
)";

static const std::string squareFragmentShaderSrc = R"(
#version 430 core

/** Inputs */
in vec3 vsColor;
in vec2 vsTexcoord;

/** Outputs */
out vec4 outColor;

/** Binding specifies what texture slot the texture should be at (in this case TEXTURE0) */
uniform sampler2D uTextureA;
uniform sampler2D uTextureB;

void main()
{
	vec4 textColorA = texture(uTextureA, vsTexcoord);
	vec4 textColorB = texture(uTextureB, vsTexcoord);
	vec4 textColormix = mix(textColorA,textColorB, 0.5);
	outColor = textColormix * vec4(vsColor, 1.0);
}
)";


#endif // __SQUARE_H_
