#ifndef PUREPATHTRACER_H
#define PUREPATHTRACER_H

#include "shader.h"

class PurePathTracer : public Shader
{
public:
    PurePathTracer();
    PurePathTracer(Vector3D bgColor_);

    Vector3D computeColor(
        const Ray &r,
        const std::vector<Shape*> &objList,
        const std::vector<LightSource*> &lsList
    ) const override;
};

#endif // PUREPATHTRACER_H
