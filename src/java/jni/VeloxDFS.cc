#include "com_dicl_velox_VeloxDFS.h"

#include <eclipsedfs/vdfs.hh>
#include <eclipsedfs/model/metadata.hh>
#include <eclipsedfs/model/block_metadata.hh>
#include <cstring>

#include <iostream>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

jobject convert_jmetadata(JNIEnv* env, jobject& obj, velox::model::metadata& md);

velox::vdfs* get_vdfs(JNIEnv* env, jobject& obj) {
  jclass vdfs_c = env->GetObjectClass(obj);
  jmethodID get_vdfs = env->GetMethodID(vdfs_c, "getVeloxDFS", "()J");
  if(get_vdfs == nullptr) return nullptr;
  
  jlong vdfs_ptr = env->CallLongMethod(obj, get_vdfs);
  return (vdfs_ptr == 0) ? nullptr : reinterpret_cast<velox::vdfs*>(vdfs_ptr);
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    constructVeloxDFS
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_dicl_velox_VeloxDFS_constructVeloxDFS
  (JNIEnv* env, jobject obj) {
  velox::vdfs* vdfs = get_vdfs(env, obj);
  return reinterpret_cast<jlong>(((vdfs == nullptr) ? new velox::vdfs() : vdfs));
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    destructVeloxDFS
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_dicl_velox_VeloxDFS_destructVeloxDFS
  (JNIEnv* env, jobject obj) {
  velox::vdfs* vdfs = get_vdfs(env, obj);
  if(vdfs != nullptr) delete vdfs;
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    open
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_dicl_velox_VeloxDFS_open
  (JNIEnv* env, jobject obj, jstring str) {
  velox::vdfs* vdfs = get_vdfs(env, obj);
  const char* file_name = env->GetStringUTFChars(str, 0);
  jlong fid = (jlong)vdfs->open_file(std::string(file_name));
  env->ReleaseStringUTFChars(str, file_name);

  return fid;
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    close
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_com_dicl_velox_VeloxDFS_close
  (JNIEnv* env, jobject obj, jlong fid) {
  velox::vdfs* vdfs = get_vdfs(env, obj);
  return (jboolean)vdfs->close(fid);
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    isOpen
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_com_dicl_velox_VeloxDFS_isOpen
  (JNIEnv* env, jobject obj, jlong fid) {
  velox::vdfs* vdfs = get_vdfs(env, obj);
  return (jboolean)vdfs->is_open(fid);
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    write
 * Signature: (JJ[BJJ)J
 */
JNIEXPORT jlong JNICALL Java_com_dicl_velox_VeloxDFS_write
  (JNIEnv* env, jobject obj, jlong fid, jlong pos, jbyteArray buf, jlong off, jlong len) {
  velox::vdfs* vdfs = get_vdfs(env, obj);

  char buffer[len];
  
  env->GetByteArrayRegion(buf, off, (jsize)len, reinterpret_cast<jbyte*>(buffer));

  // TODO: Copy buffer data from given buf, it might be slow.
  // If you don't want to copy data, set the second parameter to JNI_FALSE
  // jboolean* isCopy = JNI_FALSE;
  //jbyte* buffer = env->GetByteArrayElements(buf, NULL);

  jlong ret = vdfs->write((long)fid, buffer, (uint64_t)pos, (uint64_t)len);

  //env->ReleaseByteArrayElements(buf, buffer, JNI_ABORT);

  return ret;
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    read
 * Signature: (JJ[BJJ)J
 * fid : File descriptor
 * pos : position to read in a file
 * buf : buffer to store read data
 * off : offset to read in the buffer
 * len : length to read
 */
JNIEXPORT jlong JNICALL Java_com_dicl_velox_VeloxDFS_read
  (JNIEnv* env, jobject obj, jlong fid, jlong pos, jbyteArray buf, jlong off, jlong len) {
  velox::vdfs* vdfs = get_vdfs(env, obj);

  char c_buf[len+1] = {0};

  uint32_t ret = vdfs->read((long)fid, c_buf, (uint64_t)pos, (uint64_t)len);

  env->SetByteArrayRegion(buf, off, ret, (jbyte*)c_buf);

  return ret;
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    getMetadata
 * Signature: (J)Lcom/dicl/velox/model/Metadata;
 */
JNIEXPORT jobject JNICALL Java_com_dicl_velox_VeloxDFS_getMetadata
  (JNIEnv* env, jobject obj, jlong fid) {
  velox::vdfs* vdfs = get_vdfs(env, obj);

  velox::model::metadata md(vdfs->get_metadata((long)fid));
  return convert_jmetadata(env, obj, md);
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    remove
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_dicl_velox_VeloxDFS_remove
  (JNIEnv* env, jobject obj, jstring fname) {
  velox::vdfs* vdfs = get_vdfs(env, obj);
  const char* file_name = env->GetStringUTFChars(fname, 0);
  jboolean ret = vdfs->rm(file_name);
  env->ReleaseStringUTFChars(fname, file_name);
  return ret;
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    exists
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_dicl_velox_VeloxDFS_exists
  (JNIEnv* env, jobject obj, jstring fname) {
  velox::vdfs* vdfs = get_vdfs(env, obj);
  const char* file_name = env->GetStringUTFChars(fname, 0);
  jboolean ret = vdfs->exists(file_name);
  env->ReleaseStringUTFChars(fname, file_name);
  return ret;
}

/*
 * Class:     com_dicl_velox_VeloxDFS
 * Method:    list
 * Signature: (ZLjava/lang/String;)[Lcom/dicl/velox/model/Metadata;
 */
JNIEXPORT jobjectArray JNICALL Java_com_dicl_velox_VeloxDFS_list
  (JNIEnv* env, jobject obj, jboolean all, jstring name) {
  velox::vdfs* vdfs = get_vdfs(env, obj);

  const char* n = env->GetStringUTFChars(name, 0);
  std::vector<velox::model::metadata> metadatas = vdfs->list(all, n);
  env->ReleaseStringUTFChars(name, n);

  jclass MetadataClass = env->FindClass("Lcom/dicl/velox/model/Metadata;");
  jmethodID MetadataClassInit = env->GetMethodID(MetadataClass, "<init>", "(Ljava/lang/String;JJIII[Lcom/dicl/velox/model/BlockMetadata;)V");
  jobjectArray list = (jobjectArray)env->NewObjectArray(metadatas.size(), MetadataClass, NULL);

  for(int i=0; i<metadatas.size(); i++) 
    env->SetObjectArrayElement(list, (jsize)i, convert_jmetadata(env, obj, metadatas[i]));

  return list;
}

jobject convert_jmetadata(JNIEnv* env, jobject& obj, velox::model::metadata& md) {
  jstring file_name = env->NewStringUTF(md.name.c_str());
  jobjectArray block_data = nullptr;

  if(md.has_block_data) {
    jclass velox_model_BlockMetadata = env->FindClass("Lcom/dicl/velox/model/BlockMetadata;");
    block_data = (jobjectArray)env->NewObjectArray(md.num_block, velox_model_BlockMetadata, NULL);

    jmethodID velox_model_BlockMetadata_init = env->GetMethodID(velox_model_BlockMetadata, "<init>", "()V");

    // file_name
    jfieldID fn_field_id = env->GetFieldID(velox_model_BlockMetadata, "fileName", "Ljava/lang/String;");

    for(int i=0; i<md.num_block; i++) {
      velox::model::block_metadata& bdata = md.block_data[i];

      jobject data = env->NewObject(velox_model_BlockMetadata, velox_model_BlockMetadata_init);

      // name
      jfieldID name_field_id = env->GetFieldID(velox_model_BlockMetadata, "name", "Ljava/lang/String;");
      jstring name = env->NewStringUTF(bdata.name.c_str());
      env->SetObjectField(data, name_field_id, name);
      env->DeleteLocalRef(name);

      // host
      jfieldID host_field_id = env->GetFieldID(velox_model_BlockMetadata, "host", "Ljava/lang/String;");
      jstring host = env->NewStringUTF(bdata.host.c_str());
      env->SetObjectField(data, host_field_id, host);
      env->DeleteLocalRef(host);

      // file_name
      env->SetObjectField(data, fn_field_id, file_name);

      // index
      jfieldID index_field_id = env->GetFieldID(velox_model_BlockMetadata, "index", "I");
      env->SetIntField(data, index_field_id, (jint)i);

      // size
      jfieldID size_field_id = env->GetFieldID(velox_model_BlockMetadata, "size", "J");
      env->SetLongField(data, size_field_id, (jlong)md.block_size[i]);

      env->SetObjectArrayElement(block_data, (jsize)i, data);
      env->DeleteLocalRef(data);
    }
  }

  jclass MetadataClass = env->FindClass("Lcom/dicl/velox/model/Metadata;");
  jmethodID init = env->GetMethodID(MetadataClass, "<init>", "(Ljava/lang/String;JJIII[Lcom/dicl/velox/model/BlockMetadata;)V");
  jobject ret = env->NewObject(MetadataClass, init,
    file_name, md.hash_key, md.size, md.num_block, md.type, md.replica, block_data
  );
  
  env->DeleteLocalRef(file_name);
  if(block_data != nullptr)
    env->DeleteLocalRef(block_data);

  return std::move(ret);
}

#ifdef __cplusplus
}
#endif
