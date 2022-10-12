#version 410

uniform vec3 light_position;
uniform samplerCube cube_map;
uniform mat4 normal_model_to_world;
uniform sampler2D normal_map;
uniform vec3 camera_position;

in VS_OUT{
	vec3 texcoord;
	vec3 vertex;
	vec3 normal;
	vec3 view;
	vec2 normalCoord0;
	vec2 normalCoord1;
	vec2 normalCoord2;
	mat3 TBN;
}fs_in;

out vec4 frag_color;

void main()
{
	vec4 deep=vec4(0.,0.,0.,1.);
	vec4 shallow=vec4(0.,0.,0.,1.);
	
	vec3 n=normalize(texture(normal_map,fs_in.normalCoord0).xyz*2.-1.
	+texture(normal_map,fs_in.normalCoord1).xyz*2.-1.
	+texture(normal_map,fs_in.normalCoord2).xyz*2.-1.);
	// n=vec3(0.,0.,1.);
	
	n=mix(vec3(0.,0.,1.),n,.3);
	vec3 N=normalize(fs_in.TBN*n);
	N=mix(vec3(0.,1.,0.),N,.05);
	// N=normalize(fs_in.TBN[2]);
	// N=vec3(0.,1.,0.);
	// N=normalize(fs_in)
	float Rn=1/1.33;
	
	vec3 V=normalize(camera_position-fs_in.vertex);
	vec3 R=normalize(reflect(-V,N));
	vec3 Refract=normalize(refract(-V,N,Rn));
	
	float R0=.5037;
	// R0=Rn;
	float fresnel=R0+(1.-R0)*pow((1.-dot(V,N)),5.);
	
	float facing=1.-max(dot(V,N),0.);
	
	frag_color=vec4(0.,0.,0.,1.);
	frag_color+=texture(cube_map,R)*fresnel;
	frag_color+=texture(cube_map,Refract)*(1.-fresnel)*vec4(0.,0.,.0118,1.);
	frag_color+=mix(deep,shallow,facing);
	
	// frag_color=texture(cube_map,R)*fresnel;
	// frag_color=texture(cube_map,R)*fresnel+texture(cube_map,Refract)*(1.-fresnel);
	// frag_color=vec4(fresnel,fresnel,fresnel,1.);
	// + texture(cube_map, R) * fresnel + texture(cube_map, Refract) * (1.0 - fresnel);
}
