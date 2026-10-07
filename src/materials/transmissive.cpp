#include "transmissive.h"

Transmissive::Transmissive(double refractionRatio_) :
    refractionRatio(refractionRatio_)
{ }

Vector3D Transmissive::getReflectance(
    const Vector3D &n,
    const Vector3D &wo,
    const Vector3D &wi) const
{
    return Vector3D(0.0);
}

double Transmissive::getIndexOfRefraction() const
{
    return refractionRatio;
}
