#version 330 core
layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_normal;
layout(location=2) in vec3 a_color;
layout(location=3) in vec2 a_material;
layout(location=4) in vec2 a_info;
flat out vec3 u_color;
flat out vec2 u_material;
flat out float u_group,u_finish;
uniform vec3 u_camera, u_right, u_up, u_forward;
uniform float u_aspect;
out vec3 v_normal;out vec3 v_world;out vec3 v_view;
void main(){
 u_color=a_color;u_material=a_material;u_group=a_info.x;u_finish=a_info.y;
 v_world=a_position;v_normal=a_normal;
 vec3 rel=a_position-u_camera;
 float depth=max(.1,dot(rel,u_forward));
 float sx=dot(rel,u_right),sy=dot(rel,u_up);
 float nearPlane=.1,farPlane=35.;
 gl_Position=vec4((2.55/u_aspect)*sx,2.55*sy,depth*(farPlane+nearPlane)/(farPlane-nearPlane)-2.*farPlane*nearPlane/(farPlane-nearPlane),depth);
 v_view=normalize(u_camera-a_position);
}