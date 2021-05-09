#include <jni.h>
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

extern "C" JNIEXPORT void JNICALL
Java_com_tjdickerson_sacweather_SacwLib_sacwRadarInit(
        JNIEnv* env,
        jobject /* this */,
        jstring filename)
{

    const char* c_filepath = env->GetStringUTFChars(filename, 0);
    sacw_RadarInit(c_filepath);
}