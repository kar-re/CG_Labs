#version 410
uniform samplerCube uTexture;
in VS_OUT{
  vec3 normal;
}fs_in;

out vec4 frag_color;

void main()
{
  frag_color=texture(uTexture,fs_in.normal);
  // frag_color=vec4(fs_in.normal,1.);
}
