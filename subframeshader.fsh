#extension GL_EXT_gpu_shader4 : enable
uniform int subFrame;
uniform int renderMode;
uniform vec4 color;
uniform bool sampleTexture;
uniform sampler1D sampler;

void main() {
  vec4 mycolor = color;

  if (sampleTexture) {
    mycolor = texture1D(sampler, gl_TexCoord[0].s);
  }

  if (renderMode == 24) {
        highp float intensity = (mycolor.r+mycolor.g+mycolor.b)/3.0;
        int bit = 0x1;
        if (intensity < 1.0/255.0) bit = 0x0;
        int word = bit << (23-subFrame);
        int r = (word>>16)&0xff;
        int g = (word>>8)&0xff;
        int b = word&0xff;
        gl_FragColor.r = float(r)/255.0;
        gl_FragColor.g = float(g)/255.0;
        gl_FragColor.b = float(b)/255.0;
  } else if (renderMode == 8) {
      int shift = 7-subFrame;
      highp float intensity = (mycolor.r+mycolor.g+mycolor.b)/3.0;
      int bits = int(intensity * 7.0);
      int r = ((bits&0x4)>>2)<<shift;
      int g = ((bits&0x2)>>1)<<shift;
      int b = (bits&0x1)<<shift;
      r = r & 0xff; g = g & 0xff; b = b & 0xff;
      gl_FragColor.r = float(r)/255.0;
      gl_FragColor.g = float(g)/255.0;
      gl_FragColor.b = float(b)/255.0;
  } else if (renderMode == 3) {
      if (subFrame == 0) gl_FragColor = vec4(mycolor.r, 0, 0, 1);
      else if (subFrame == 1) gl_FragColor = vec4(0,mycolor.g, 0, 1);
      else if (subFrame == 2) gl_FragColor = vec4(0,0,mycolor.b, 1);
  } else {
      gl_FragColor = mycolor;
  }
}
