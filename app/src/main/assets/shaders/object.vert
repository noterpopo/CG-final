/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

uniform mat4 u_ModelView;
uniform mat4 u_ModelViewProjection;

attribute vec4 a_Position;
attribute vec3 a_Normal;
attribute vec2 a_TexCoord;
attribute vec4 a_BoneIDs;
attribute vec4 a_Weights;

const int MAX_BONES = 100;

uniform mat4 u_bones[MAX_BONES];
uniform int u_hasAni;

varying vec3 v_ViewPosition;
varying vec3 v_ViewNormal;
varying vec2 v_TexCoord;

void main() {

    v_ViewPosition = (u_ModelView * a_Position).xyz;
    v_ViewNormal = normalize((u_ModelView * vec4(a_Normal, 0.0)).xyz);
    v_TexCoord = a_TexCoord;

    if(u_hasAni==1)
    {
        mat4 BoneTransform = u_bones[int(a_BoneIDs[0])] * a_Weights[0];
        BoneTransform += u_bones[int(a_BoneIDs[1])] * a_Weights[1];
        BoneTransform += u_bones[int(a_BoneIDs[2])] * a_Weights[2];
        BoneTransform += u_bones[int(a_BoneIDs[3])] * a_Weights[3];

        gl_Position = u_ModelViewProjection * BoneTransform * a_Position;
    }
    else
    {
        gl_Position = u_ModelViewProjection * a_Position;
    }
}
