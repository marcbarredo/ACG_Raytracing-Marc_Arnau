#include "phong.h"

#include <algorithm>
#include <cmath>

#include <iostream>

Phong::Phong()
{ }

Phong::Phong(Vector3D Kd_, Vector3D Ks_, float alpha_):
rho_d(Kd_), Ks(Ks_), alpha(alpha_){}

//n -> normal
// wo -> viewing direction
//wi -> incident light direction
// wr -> reflection direction

//rho_d = pd -> diffuse
// Ks -> specular
//alpha -> shininess

Vector3D Phong::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {

    Vector3D wr = 2.0 * dot(n, wi) * n - wi;

    double cosAlpha = std::max(dot(wo, wr), 0.0);

    Vector3D diffuse = rho_d / 3.1416;
    Vector3D specular = ((alpha + 2.0)/(2.0 * 3.1416))* Ks * std::pow(cosAlpha, alpha);

    return diffuse + specular;
};

double Phong::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Phong::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Phong::getDiffuseReflectance() const
{
    return rho_d;
}

