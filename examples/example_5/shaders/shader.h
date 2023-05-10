#ifndef __SQUARE_H_
#define __SQUARE_H_

#include <string>

static const std::string VertexShaderSrc = R"(
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_normals;
//layout(location = 2) in vec2 a_texture; Incase we want to add textures to our model later.

//We specify our uniforms. We do not need to specify locations manually, but it helps with knowing what is bound where.
layout(location=0) uniform mat4 u_TransformationMat = mat4(1);
layout(location=1) uniform mat4 u_ViewMat           = mat4(1);
layout(location=2) uniform mat4 u_ProjectionMat     = mat4(1);

out vec4 vertexPositions;
out vec3 normals;

void main()
{

//We need these in a different shader later down the pipeline, so we need to send them along. Can't just call in a_Position unfortunately.
vertexPositions = vec4(a_Position, 1.0);

//Find the correct values for our normals given that we move our object around in the world and the normals change quite a bit.
mat3 normalmatrix = transpose(inverse(mat3(u_ViewMat * u_TransformationMat)));

//Then normalize those new values so we do not accidentally go above length = 1. Also normalize the normals themselves beforehand, just to be sure calculations are accurate.
normals = normalize(normalmatrix * normalize(a_normals));

//We multiply our matrices with our position to change the positions of vertices to their final destinations.
gl_Position = u_ProjectionMat * u_ViewMat * u_TransformationMat * vertexPositions;
}
)";

static const std::string directionalLightFragmentShaderSrc = R"(
#version 430 core

in vec4 vertexPositions;
in vec3 normals;

out vec4 color;

uniform mat4 u_TransformationMat = mat4(1);
uniform mat4 u_ViewMat = mat4(1);

uniform vec4 u_Color;
uniform vec3 u_LightColor;
uniform vec3 u_LightDirection;
uniform vec3 u_LightPosition;    //Not used in this shader, but included to avoid errors when we query for it's location
uniform float u_Specularity;


//This is a function definition in GLSL. Not necessary, but may help readability in long shaders.
vec3 DirectionalLight(
    in vec3 color,
    in vec3 direction
)
{

    //Ambient lighting. Ambient light is light that is present even when normally no light should shine upon that part of the object. 
    //This is the poor mans way of simulating light reflecting from other surfaces in the room. For those that don't want to get into more advanced lighting models.
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * color;
    
    //Diffuse lighting. Light that scatters in all directions after hitting the object.
    vec3 dir_to_light = normalize(-direction);                                          //First we get the direction from the object to the lightsource ( which is of course the opposite of the direction of the light)
    vec3 diffuse = color * max(0.0, dot(normals, dir_to_light));                         //Then we find how strongly the light scatters in different directions, with a minimum of 0.0, via the normals and the direction we just found.

    //Specular Lighting. Light that is reflected away from the object it hits with the same angle it hit it. Think of it as light hitting a mirror and bouncing off, if you will.
    vec3 viewDirection = normalize(vec3(inverse(u_ViewMat) * vec4(0,0,0,1) - u_TransformationMat * vertexPositions)); //We find the direction from the surface the light hits to our camera.
    
    vec3 reflectionDirection = reflect(dir_to_light,normals);                                                         //And then we find the angle between the direction of the light and the direction from surface to camera
    
    float specular_power = pow(max(0.0,dot(viewDirection,reflectionDirection)),32);                                   //The closer together those two vectors are, the more powerful the specular light.
    vec3 specular = u_Specularity * specular_power * color;                                                           //Finally, multiply with how reflective the surface is and add the color.


    return ambient + diffuse + specular;
}

void main()
{
//Calling a function is the same as in C++
vec3 light = DirectionalLight(u_LightColor,u_LightDirection);

//Finally, multiply with the color. Make sure the vector still has the same dimensions. Alpha channel is set to 1 here, because our object is not transparent. Might be different if you use a texture.
color = u_Color * vec4(light, 1.0);
}


)";

static const std::string pointLightFragmentShaderSrc = R"(
#version 430 core

in vec4 vertexPositions;
in vec3 normals;

out vec4 color;

uniform mat4 u_TransformationMat = mat4(1);
uniform mat4 u_ViewMat = mat4(1);

uniform vec4 u_Color;
uniform vec3 u_LightColor;
uniform vec3 u_LightDirection; //Not used in this shader, but included to avoid errors when we query for it's location
uniform vec3 u_LightPosition;
uniform float u_Specularity;

//Pointlights are the little brother of directional lights. A lot less powerful, but they are also a little more difficult to understand. Like a toddler.
vec3 PointLight(
    in vec3 position,
    in vec3 color,
    in float constant,
    in float linear,
    in float quadratic
)
{
    //Ambient lighting. This is still the same as in the other shader.
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * color;

    //Diffuse light. Also still the same, except now we need to calculate the direction between the position of the lightsource and the vertext ourselves.
    vec3 lightDirection = normalize(position - vec3(u_TransformationMat * vertexPositions));
    float diffusion = max(0.0, dot(normals, lightDirection));
    vec3 diffuse = diffusion * color;

    //Speculatity. Also still mostly the same. We can use the lightDirection from above to save ourselves some time here.
    vec3 viewDirection = normalize(vec3(inverse(u_ViewMat) * vec4(0,0,0,1) - u_TransformationMat * vertexPositions));
    vec3 reflectionDirection = reflect(-lightDirection,normals);
    float specularPower = pow(max(0.0, dot(viewDirection,reflectionDirection)),32);
    vec3 specular = u_Specularity * specularPower * color;

    //Attenuation. This is the only thing different. Since pointlights don't light up things with infinite range (usually) we have to figure out how strong the light is after traveling for a certain distance from it's source.
    float distance = length(position - vec3(u_TransformationMat * vertexPositions));

    //This is a... interesting formula with a lot of variables to try and make semi realistic falloff for the light strength. Feel free to mess with the values and see what results you get.
    float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance + 0.0000000000001);

    //Finally, remember that ambient light is above such petty attempts at curbing it's strength as "attenuation" and always remains the same no matter how far from the lightsource it is. 
    //Alternatively, if you do not want truly complete "ambient light" feel free to add it to the inner paratheses to make it fade out the further you get away from your pointlight. This means things get pitch black after a certain distance.
    return ambient + ((diffuse + specular) * attenuation);
}

void main()
{
vec3 pointLight = PointLight(u_LightPosition,u_LightColor,0.5,0.25,0.125);
color = u_Color * vec4(pointLight, 1.0);
}
)";

#endif // __SQUARE_H_
