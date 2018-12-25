#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"

#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <vector>
#include <map>

#define TAG "assimp-jni"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型

#define NUM_BONES_PER_VEREX 4
using namespace std;

struct BoneMatrix
{
    aiMatrix4x4 offset_matrix;
    aiMatrix4x4 final_world_transform;

};


static const aiScene *scene;

map<string, unsigned int> m_bone_mapping; // maps a bone name and their index

unsigned int m_num_bones=0;

static vector<BoneMatrix> m_bone_matrices;

float ticks_per_second = 0.0f;

aiMatrix4x4 m_global_inverse_transform;


//functions
void processMesh(aiMesh* mesh, const aiScene* scene);
void processNode(aiNode* node, const aiScene* scene);

void readNodeHierarchy(aiScene* gscene,float p_animation_time, const aiNode* p_node, const aiMatrix4x4 parent_transform);
const aiNodeAnim * findNodeAnim(const aiAnimation * p_animation, const string p_node_name);
unsigned int findPosition(float p_animation_time, const aiNodeAnim* p_node_anim);
unsigned int findRotation(float p_animation_time, const aiNodeAnim* p_node_anim);
unsigned int findScaling(float p_animation_time, const aiNodeAnim* p_node_anim);
aiVector3D calcInterpolatedPosition(float p_animation_time, const aiNodeAnim* p_node_anim);
aiQuaternion calcInterpolatedRotation(float p_animation_time, const aiNodeAnim* p_node_anim);
aiVector3D calcInterpolatedScaling(float p_animation_time, const aiNodeAnim* p_node_anim);
glm::mat4 aiToGlm(aiMatrix4x4 ai_matr);
aiQuaternion nlerp(aiQuaternion a, aiQuaternion b, float blend);

//Error functions
int checkExc(JNIEnv *env) {
    if(env->ExceptionCheck()) {
        env->ExceptionDescribe(); // writes to logcat
        env->ExceptionClear();
        return 1;
    }
    return -1;
}

void JNU_ThrowByName(JNIEnv *env, const char *name, const char *msg)
{
    // 查找异常类
    jclass cls = env->FindClass(name);
    /* 如果这个异常类没有找到，VM会抛出一个NowClassDefFoundError异常 */
    if (cls != NULL) {
        env->ThrowNew(cls, msg);  // 抛出指定名字的异常
    }
    /* 释放局部引用 */
    env->DeleteLocalRef(cls);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_popo_assimptest_AssimpImporter_modelimporter(JNIEnv* env,jobject obj,jobject model,jobject assetManager,jstring filename)
{
    jfloat* vertexArray;
    jfloat* normalArray;
    jfloat* textureCoordArray;
    jshort* indexArray;
    jfloat* boneIds;
    jfloat* weights;

    Assimp::Importer importer;

    LOGD("ok1");

    const char* utf8 = env->GetStringUTFChars(filename,0);
    if(!utf8){return JNI_FALSE;}
    AAssetManager* mgr=AAssetManager_fromJava(env,assetManager);
    if(!mgr){return JNI_FALSE;}
    AAsset* asset=AAssetManager_open(mgr,utf8,AASSET_MODE_STREAMING);
    if(!asset){return JNI_FALSE;}

    //FILE* file=fopen("/sdcard/temp.txt","w+");
    std::vector<char> buffer;
    off64_t count = AAsset_getLength64(asset);
    off64_t remaining = AAsset_getRemainingLength64(asset);
    size_t Mb = 1024 *512; // 0.5Mb is maximum chunk size for compressed assets
    size_t currChunk;
    buffer.reserve(count);
    while (remaining != 0)
    {
        //set proper size for our next chunk
        if(remaining >= Mb)
        {
            currChunk = Mb;
        }
        else
        {
            currChunk = remaining;
        }
        char chunk[currChunk];

        //read data chunk
        if(AAsset_read(asset, chunk, currChunk) > 0) // returns less than 0 on error
        {
            //and append it to our vector
            buffer.insert(buffer.end(),chunk, chunk + currChunk);
            //fputs(chunk,file);
            //fflush(file);
            remaining = AAsset_getRemainingLength64(asset);
        }
    }
    //fflush(file);
    //fclose(file);
    AAsset_close(asset);

    char *buf=reinterpret_cast<char*>(buffer.data());

    importer.ReadFileFromMemory(buf,count,aiProcess_Triangulate | aiProcess_FlipUVs);
    scene=importer.GetOrphanedScene();
   // const aiScene* scene=importer.ReadFile("/sdcard/temp.txt", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if(!scene){return JNI_FALSE;}



    m_global_inverse_transform = scene->mRootNode->mTransformation;
    m_global_inverse_transform.Inverse();

    if (scene->mNumAnimations != 0&&scene->mAnimations[0]->mTicksPerSecond != 0.0)
    {
        ticks_per_second = (float)scene->mAnimations[0]->mTicksPerSecond;
    }
    else
    {
        ticks_per_second = 25.0f;
    }

    processNode(scene->mRootNode, scene);

    aiMesh* mesh=scene->mMeshes[0];

    int vertexArraySize=mesh->mNumVertices*3;
    int normalArraySize=mesh->mNumVertices*3;
    int textureCoordArraySize=mesh->mNumVertices*2;
    int indexArraySize=mesh->mNumFaces*3;
    int boneIdsAndWeightsSize=mesh->mNumVertices*NUM_BONES_PER_VEREX;


    vertexArray=new float[vertexArraySize];
    normalArray=new float[normalArraySize];
    textureCoordArray=new float[textureCoordArraySize];
    indexArray=new jshort[indexArraySize];
    boneIds=new jfloat[boneIdsAndWeightsSize];
    weights=new jfloat[boneIdsAndWeightsSize];

    // init all values in array = 0
    memset(vertexArray, 0, sizeof(jfloat)*vertexArraySize);
    memset(normalArray, 0, sizeof(jfloat)*normalArraySize);
    memset(textureCoordArray, 0, sizeof(jfloat)*textureCoordArraySize);
    memset(indexArray, 0, sizeof(jshort)*indexArraySize);
    memset(boneIds, 0, sizeof(jfloat)*boneIdsAndWeightsSize);
    memset(weights, 0, sizeof(jfloat)*boneIdsAndWeightsSize);




    for(unsigned int i=0;i<mesh->mNumVertices;++i)
    {
        aiVector3D pos=mesh->mVertices[i];
        vertexArray[3*i+0]=pos.x;
        vertexArray[3*i+1]=pos.y;
        vertexArray[3*i+2]=pos.z;

        aiVector3D normal=mesh->mNormals[i];
        normalArray[3*i+0]=normal.x;
        normalArray[3*i+1]=normal.y;
        normalArray[3*i+2]=normal.z;

        aiVector3D textureCoord=mesh->mTextureCoords[0][i];
        textureCoordArray[2*i+0]=textureCoord.x;
        textureCoordArray[2*i+1]=textureCoord.y;
    }

    for(unsigned int i=0;i<mesh->mNumFaces;++i)
    {
        const aiFace& face=mesh->mFaces[i];
        indexArray[3*i+0]=face.mIndices[0];
        indexArray[3*i+1]=face.mIndices[1];
        indexArray[3*i+2]=face.mIndices[2];
    }

    if(scene->mNumAnimations != 0)
    {
        //boneData
        for (unsigned int i = 0; i < mesh->mNumBones; i++) {
            unsigned int bone_index = 0;
            string bone_name(mesh->mBones[i]->mName.data);


            if (m_bone_mapping.find(bone_name) == m_bone_mapping.end()) {
                // Allocate an index for a new bone
                bone_index = m_num_bones;
                m_num_bones++;
                BoneMatrix bi;
                m_bone_matrices.push_back(bi);
                m_bone_matrices[bone_index].offset_matrix = mesh->mBones[i]->mOffsetMatrix;
                m_bone_mapping[bone_name] = bone_index;

            } else {
                bone_index = m_bone_mapping[bone_name];
            }

            for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; ++j) {
                unsigned int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
                float weight = mesh->mBones[i]->mWeights[j].mWeight;
                for(int g=0;g<NUM_BONES_PER_VEREX;++g)
                {
                    if(weights[vertex_id*NUM_BONES_PER_VEREX+g]==0.0)
                    {
                        boneIds[vertex_id*NUM_BONES_PER_VEREX+g]=bone_index;
                        weights[vertex_id*NUM_BONES_PER_VEREX+g]=weight;
                        break;
                    }
                }
            }
        }
    }

    LOGD("ok2");


    jclass cls=env->GetObjectClass(model);
    if(!cls){return JNI_FALSE;}
    if (checkExc(env)==1) {
        LOGD("jni exception happened at p0");
        JNU_ThrowByName(env, "java/lang/Exception", "exception from jni: jni exception happened at p0");
        return -1;
    }
    LOGD("ok3");

    jmethodID setVA=env->GetMethodID(cls,"setVertexArray","([F)V");
    jmethodID setNA=env->GetMethodID(cls,"setNormalArray","([F)V");
    jmethodID setTA=env->GetMethodID(cls,"setTextureCoordArray","([F)V");
    jmethodID setIA=env->GetMethodID(cls,"setIndexArray","([S)V");
    jmethodID setID=env->GetMethodID(cls,"setBoneIds","([F)V");
    jmethodID setWE=env->GetMethodID(cls,"setWeights","([F)V");
    jmethodID setHA=env->GetMethodID(cls,"setHasAni","(Z)V");

    if (checkExc(env)==1) {
        LOGD("jni exception happened at p1");
        JNU_ThrowByName(env, "java/lang/Exception", "exception from jni: jni exception happened at p1");
        return -1;
    }

    jfloatArray jvertexArray=env->NewFloatArray(vertexArraySize);
    env->SetFloatArrayRegion(jvertexArray,0,vertexArraySize,vertexArray);
    jfloatArray jnormalArray=env->NewFloatArray(normalArraySize);
    env->SetFloatArrayRegion(jnormalArray,0,normalArraySize,normalArray);
    jfloatArray jtexturecoordArray=env->NewFloatArray(textureCoordArraySize);
    env->SetFloatArrayRegion(jtexturecoordArray,0,textureCoordArraySize,textureCoordArray);
    jshortArray jindexArray=env->NewShortArray(indexArraySize);
    env->SetShortArrayRegion(jindexArray,0,indexArraySize,indexArray);
    if (checkExc(env)==1) {
        LOGD("jni exception happened at p3");
        JNU_ThrowByName(env, "java/lang/Exception", "exception from jni: jni exception happened at p3");
        return -1;
    }
    LOGD("ok5");

    env->CallVoidMethod(model,setVA,jvertexArray);
    env->CallVoidMethod(model,setNA,jnormalArray);
    env->CallVoidMethod(model,setTA,jtexturecoordArray);
    env->CallVoidMethod(model,setIA,jindexArray);
    env->CallVoidMethod(model,setHA,(scene->mNumAnimations != 0));
    if(scene->mNumAnimations != 0)
    {
        jfloatArray jBoneIds=env->NewFloatArray(boneIdsAndWeightsSize);
        env->SetFloatArrayRegion(jBoneIds,0,boneIdsAndWeightsSize,boneIds);
        jfloatArray jWeights=env->NewFloatArray(boneIdsAndWeightsSize);
        env->SetFloatArrayRegion(jWeights,0,boneIdsAndWeightsSize,weights);
        env->CallVoidMethod(model,setID,jBoneIds);
        env->CallVoidMethod(model,setWE,jWeights);
    }
    if (checkExc(env)==1) {
        LOGD("jni exception happened at p1");
        JNU_ThrowByName(env, "java/lang/Exception", "exception from jni: jni exception happened at p4");
        return -1;
    }
    LOGD("ok6");
    return (long)scene;
}

void processMesh(aiMesh* mesh, const aiScene* scene) {
}

void processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* ai_mesh = scene->mMeshes[i];
        processMesh(ai_mesh, scene);
    }

}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_popo_assimptest_AssimpImporter_getBoneTransform(JNIEnv* env,jobject obj,jlong ptr,jdouble time_in_sec,jobject data)
{
    aiScene * gscene=(aiScene*)ptr;
    vector<aiMatrix4x4> transforms;

    aiMatrix4x4 identity_matrix; // = mat4(1.0f);

    double time_in_ticks = time_in_sec * ticks_per_second;
//    //非循环播放
//    if(time_in_ticks>gscene->mAnimations[0]->mDuration){
//        return false;
//    }
    //循环播放
    float animation_time = (float)fmod(time_in_ticks, gscene->mAnimations[0]->mDuration);
    readNodeHierarchy(gscene,animation_time, gscene->mRootNode, identity_matrix);

    transforms.resize(m_num_bones);

    for (unsigned int i = 0; i < m_num_bones; i++)
    {
        transforms[i] = m_bone_matrices[i].final_world_transform;
    }

    jclass cls=env->GetObjectClass(data);
    if(!cls){return false;}
    if (checkExc(env)==1) {
        LOGD("jni exception happened at p0");
        JNU_ThrowByName(env, "java/lang/Exception", "exception from jni: jni exception happened at p0");
    }
    LOGD("ok0");

    jmethodID addData=env->GetMethodID(cls,"addMatrix","([F)V");
    if (checkExc(env)==1) {
        LOGD("jni exception happened at p1");
        JNU_ThrowByName(env, "java/lang/Exception", "exception from jni: jni exception happened at p1");
    }

    jfloat* dataArray;
    jfloatArray jdataArray;

    int size=transforms.size();
    for(int i=0;i<size;++i)
    {
        dataArray=new float[16];
        aiMatrix4x4 temp=transforms[i];

        dataArray[0]=(temp.a1);
        dataArray[1]=(temp.a2);
        dataArray[2]=(temp.a3);
        dataArray[3]=(temp.a4);
        dataArray[4]=(temp.b1);
        dataArray[5]=(temp.b2);
        dataArray[6]=(temp.b3);
        dataArray[7]=(temp.b4);
        dataArray[8]=(temp.c1);
        dataArray[9]=(temp.c2);
        dataArray[10]=(temp.c3);
        dataArray[11]=(temp.c4);
        dataArray[12]=(temp.d1);
        dataArray[13]=(temp.d2);
        dataArray[14]=(temp.d3);
        dataArray[15]=(temp.d4);

        jdataArray=env->NewFloatArray(16);
        env->SetFloatArrayRegion(jdataArray,0,16,dataArray);

        env->CallVoidMethod(data,addData,jdataArray);

        delete []dataArray;
    }
    if (checkExc(env)==1) {
        LOGD("jni exception happened at p100");
        JNU_ThrowByName(env, "java/lang/Exception", "exception from jni: jni exception happened at p100");
    }
    LOGD("ok100");
    return true;

}

void readNodeHierarchy(aiScene* gscene,float p_animation_time, const aiNode* p_node, const aiMatrix4x4 parent_transform)
{
    string node_name(p_node->mName.data);

    const aiAnimation* animation = gscene->mAnimations[0];
    aiMatrix4x4 node_transform = p_node->mTransformation;

    const aiNodeAnim* node_anim = findNodeAnim(animation, node_name);

    if (node_anim)
    {

        //scaling
        //aiVector3D scaling_vector = node_anim->mScalingKeys[2].mValue;
        aiVector3D scaling_vector = calcInterpolatedScaling(p_animation_time, node_anim);
        aiMatrix4x4 scaling_matr;
        aiMatrix4x4::Scaling(scaling_vector, scaling_matr);

        //rotation
        //aiQuaternion rotate_quat = node_anim->mRotationKeys[2].mValue;
        aiQuaternion rotate_quat = calcInterpolatedRotation(p_animation_time, node_anim);
        aiMatrix4x4 rotate_matr = aiMatrix4x4(rotate_quat.GetMatrix());

        //translation
        //aiVector3D translate_vector = node_anim->mPositionKeys[2].mValue;
        aiVector3D translate_vector = calcInterpolatedPosition(p_animation_time, node_anim);
        aiMatrix4x4 translate_matr;
        aiMatrix4x4::Translation(translate_vector, translate_matr);


        node_transform = translate_matr * rotate_matr * scaling_matr;


    }

    aiMatrix4x4 global_transform = parent_transform * node_transform;

    if (m_bone_mapping.find(node_name) != m_bone_mapping.end()) // true if node_name exist in bone_mapping
    {
        unsigned int bone_index = m_bone_mapping[node_name];
        m_bone_matrices[bone_index].final_world_transform = m_global_inverse_transform * global_transform * m_bone_matrices[bone_index].offset_matrix;
    }

    for (unsigned int i = 0; i < p_node->mNumChildren; i++)
    {
        readNodeHierarchy(gscene,p_animation_time, p_node->mChildren[i], global_transform);
    }
}
unsigned int findPosition(float p_animation_time, const aiNodeAnim* p_node_anim)
{
    for (unsigned int i = 0; i < p_node_anim->mNumPositionKeys - 1; i++)
    {
        if (p_animation_time < (float)p_node_anim->mPositionKeys[i + 1].mTime)
        {
            return i;
        }
    }
    assert(0);
    return 0;
}
unsigned int findRotation(float p_animation_time, const aiNodeAnim* p_node_anim)
{
    for (unsigned int i = 0; i < p_node_anim->mNumRotationKeys - 1; i++)
    {
        if (p_animation_time < (float)p_node_anim->mRotationKeys[i + 1].mTime)
        {
            return i;
        }
    }

    assert(0);
    return 0;
}
unsigned int findScaling(float p_animation_time, const aiNodeAnim* p_node_anim)
{
    for (unsigned int i = 0; i < p_node_anim->mNumScalingKeys - 1; i++)
    {
        if (p_animation_time < (float)p_node_anim->mScalingKeys[i + 1].mTime)
        {
            return i;
        }
    }

    assert(0);
    return 0;
}
aiVector3D calcInterpolatedPosition(float p_animation_time, const aiNodeAnim* p_node_anim)
{
    if (p_node_anim->mNumPositionKeys == 1)
    {
        return p_node_anim->mPositionKeys[0].mValue;
    }

    unsigned int position_index = findPosition(p_animation_time, p_node_anim);
    unsigned int next_position_index = position_index + 1;
    assert(next_position_index < p_node_anim->mNumPositionKeys);
    float delta_time = (float)(p_node_anim->mPositionKeys[next_position_index].mTime - p_node_anim->mPositionKeys[position_index].mTime);
    float factor = (p_animation_time - (float)p_node_anim->mPositionKeys[position_index].mTime) / delta_time;
    //assert(factor >= 0.0f && factor <= 1.0f);
    aiVector3D start = p_node_anim->mPositionKeys[position_index].mValue;
    aiVector3D end = p_node_anim->mPositionKeys[next_position_index].mValue;
    aiVector3D delta = end - start;

    return start + factor * delta;
}
aiQuaternion calcInterpolatedRotation(float p_animation_time, const aiNodeAnim* p_node_anim)
{
    if (p_node_anim->mNumRotationKeys == 1)
    {
        return p_node_anim->mRotationKeys[0].mValue;
    }

    unsigned int rotation_index = findRotation(p_animation_time, p_node_anim);
    unsigned int next_rotation_index = rotation_index + 1;
    assert(next_rotation_index < p_node_anim->mNumRotationKeys);
    float delta_time = (float)(p_node_anim->mRotationKeys[next_rotation_index].mTime - p_node_anim->mRotationKeys[rotation_index].mTime);
    float factor = (p_animation_time - (float)p_node_anim->mRotationKeys[rotation_index].mTime) / delta_time;
    //assert(factor >= 0.0f && factor <= 1.0f);
    aiQuaternion start_quat = p_node_anim->mRotationKeys[rotation_index].mValue;
    aiQuaternion end_quat = p_node_anim->mRotationKeys[next_rotation_index].mValue;

    return nlerp(start_quat, end_quat, factor);
}
aiVector3D calcInterpolatedScaling(float p_animation_time, const aiNodeAnim* p_node_anim)
{
    if (p_node_anim->mNumScalingKeys == 1)
    {
        return p_node_anim->mScalingKeys[0].mValue;
    }

    unsigned int scaling_index = findScaling(p_animation_time, p_node_anim);
    unsigned int next_scaling_index = scaling_index + 1;
    assert(next_scaling_index < p_node_anim->mNumScalingKeys);

    float delta_time = (float)(p_node_anim->mScalingKeys[next_scaling_index].mTime - p_node_anim->mScalingKeys[scaling_index].mTime);

    float  factor = (p_animation_time - (float)p_node_anim->mScalingKeys[scaling_index].mTime) / delta_time;
    //assert(factor >= 0.0f && factor <= 1.0f);
    aiVector3D start = p_node_anim->mScalingKeys[scaling_index].mValue;
    aiVector3D end = p_node_anim->mScalingKeys[next_scaling_index].mValue;
    aiVector3D delta = end - start;

    return start + factor * delta;
}
const aiNodeAnim * findNodeAnim(const aiAnimation * p_animation, const string p_node_name)
{
    // channel in animation contains aiNodeAnim (aiNodeAnim its transformation for bones)
    // numChannels == numBones
    for (unsigned int i = 0; i < p_animation->mNumChannels; i++)
    {
        const aiNodeAnim* node_anim = p_animation->mChannels[i];
        if (string(node_anim->mNodeName.data) == p_node_name)
        {
            return node_anim;
        }
    }

    return nullptr;
}
glm::mat4 aiToGlm(aiMatrix4x4 ai_matr)
{
    glm::mat4 result;
    result[0].x = ai_matr.a1; result[0].y = ai_matr.b1; result[0].z = ai_matr.c1; result[0].w = ai_matr.d1;
    result[1].x = ai_matr.a2; result[1].y = ai_matr.b2; result[1].z = ai_matr.c2; result[1].w = ai_matr.d2;
    result[2].x = ai_matr.a3; result[2].y = ai_matr.b3; result[2].z = ai_matr.c3; result[2].w = ai_matr.d3;
    result[3].x = ai_matr.a4; result[3].y = ai_matr.b4; result[3].z = ai_matr.c4; result[3].w = ai_matr.d4;
    return result;
}
aiQuaternion nlerp(aiQuaternion a, aiQuaternion b, float blend)
{
    //cout << a.w + a.x + a.y + a.z << endl;
    a.Normalize();
    b.Normalize();

    aiQuaternion result;
    float dot_product = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    float one_minus_blend = 1.0f - blend;

    if (dot_product < 0.0f)
    {
        result.x = a.x * one_minus_blend + blend * -b.x;
        result.y = a.y * one_minus_blend + blend * -b.y;
        result.z = a.z * one_minus_blend + blend * -b.z;
        result.w = a.w * one_minus_blend + blend * -b.w;
    }
    else
    {
        result.x = a.x * one_minus_blend + blend * b.x;
        result.y = a.y * one_minus_blend + blend * b.y;
        result.z = a.z * one_minus_blend + blend * b.z;
        result.w = a.w * one_minus_blend + blend * b.w;
    }

    return result.Normalize();
}
