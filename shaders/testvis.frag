#version 330 core
in vec3 ourColor;
in vec2 TexCoord;
uniform sampler2D ourTexture;

void main()
{
  //vec4 FragColor1 = vec4(ourColor,1.0f);
  vec4 FragColor = texture(ourTexture, TexCoord);
  gl_FragColor = FragColor;
}