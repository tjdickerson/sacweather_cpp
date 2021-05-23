#include <jni.h>
#include <cstdlib>
#include "sacw_api.h"

extern "C" JNIEXPORT void JNICALL
Java_com_tjdickerson_sacweather_SacwLib_sacwInit(
        JNIEnv* env,
        jobject /* this */)
{
    sacw_Init();
}

extern "C" JNIEXPORT void JNICALL
Java_com_tjdickerson_sacweather_SacwLib_sacwMainLoop(
        JNIEnv* env,
        jobject /* this */)
{
    sacw_MainLoop();
}

extern "C" JNIEXPORT void JNICALL
Java_com_tjdickerson_sacweather_SacwLib_sacwUpdateViewport(
        JNIEnv* env,
        jobject /* this */,
        jfloat width,
        jfloat height)
{
    sacw_UpdateViewport(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_tjdickerson_sacweather_SacwLib_sacwPanMap(
        JNIEnv* env,
        jobject /* this */,
        jfloat x,
        jfloat y)
{
    sacw_PanMap(x, y);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_tjdickerson_sacweather_SacwLib_sacwRadarInit(
        JNIEnv* env,
        jobject /* this */,
        jstring filename,
        jshort productCode)
{
    const char* c_filepath = env->GetStringUTFChars(filename, 0);
    return sacw_RadarInit(c_filepath, productCode);
}

extern "C" JNIEXPORT void JNICALL
Java_com_tjdickerson_sacweather_SacwLib_sacwZoomMap(
        JNIEnv* env,
        jobject /* this */,
        jfloat zoom)
{
    sacw_ZoomMap(zoom);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_tjdickerson_sacweather_SacwLib_sacwScanTime(
        JNIEnv* env,
        jobject /* this */)
{
    return sacw_GetScanTime();
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_tjdickerson_sacweather_SacwLib_sacwGetPolarFromScreen(
        JNIEnv* env,
        jobject, /* this */
        jfloat x,
        jfloat y)
{
    jfloatArray temp;
    temp = env->NewFloatArray(2);

    float* data;
    data = (float*)malloc(sizeof(float) * 2);

    sacw_GetPolarFromScreen(x, y, data);
    env->SetFloatArrayRegion(temp, 0, 2, data);

    if (data != NULL) free(data);
    return temp;
}