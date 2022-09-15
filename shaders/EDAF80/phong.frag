#version 410

uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec3 diffuse_colour;
uniform vec3 ambient_colour;
uniform vec3 specular_colour;
uniform float shininess_value;
uniform sampler2D diffuse_texture;
uniform sampler2D normal_texture;
uniform sampler2D roughness_texture;
uniform int has_diffuse_texture;
uniform int use_normal_mapping;
// uniform texture2D normal;

in VS_OUT{
	vec2 texcoord;
	vec3 vertex;
	vec3 normal;
	mat3 TBN;
}fs_in;

out vec4 frag_color;

void main()
{
	
	vec3 L=normalize(light_position-fs_in.vertex);
	vec3 norm=normalize(fs_in.normal);
	if(use_normal_mapping!=0){
		norm=texture(normal_texture,fs_in.texcoord).xyz;
		norm=norm*2.-1.;
		norm=normalize(fs_in.TBN*norm);
	}
	
	vec3 viewDir=normalize(camera_position-fs_in.vertex);
	vec3 reflectDir=normalize(reflect(-L,norm));
	
	float spec=pow(max(dot(viewDir,reflectDir),0.),shininess_value);
	
	vec3 specular=texture(roughness_texture,fs_in.texcoord).xyz*spec*specular_colour;
	
	vec3 diffuse=diffuse_colour;
	// vec3 rgb_normal=normal*.5+.5;
	if(has_diffuse_texture!=0){
		diffuse=texture(diffuse_texture,fs_in.texcoord).xyz;
	}
	
	float diff=max(dot(norm,L),0.);
	vec3 result=(ambient_colour+(diff*diffuse)+specular);
	
	frag_color=vec4(result,1.);
}
