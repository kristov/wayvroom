#ifndef ESM_H
#define ESM_H

void esmDump(float* M, const char* name);
void esmMultiply(float* M, float* M2);
void esmScalef(float* M, float x, float y, float z);
void esmTranslatef(float* M, float x, float y, float z);
void esmRotatef(float* M, float angle, float x, float y, float z);
void esmFrustumf(float* M, float left, float right, float bottom, float top, float near, float far);
void esmPerspectivef(float* M, float fovy, float aspect, float near, float far);
void esmOrthof(float* M, float left, float right, float bottom, float top, float near, float far);
void esmLoadIdentity(float* M);
void esmQuaternionToMatrix(float* M, float x, float y, float z, float w);
void esmCopy(float* dst, float* src);

#endif
