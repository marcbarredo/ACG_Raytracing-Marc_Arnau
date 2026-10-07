#ifndef TRANSMISSIVE_H
#define TRANSMISSIVE_H

#include "material.h"

class Transmissive : public Material
{
public:
    Transmissive(double refractionRatio_);

    Vector3D getReflectance(const Vector3D &n,
                            const Vector3D &wo,
                            const Vector3D &wi) const;

    double getIndexOfRefraction() const;

    bool hasSpecular() const { return false; }
    bool hasTransmission() const { return true; }
    bool hasDiffuseOrGlossy() const { return false; }
    bool isEmissive() const { return false; }

private:
    double refractionRatio;
};

#endif
