#include "whittedintegrator.h"

#include "../core/utils.h"

#include <algorithm>
#include <cmath>


WhittedIntegrator::WhittedIntegrator() :
    Shader(Vector3D(0.0))
{
}


WhittedIntegrator::WhittedIntegrator(Vector3D bgColor_) :
    Shader(bgColor_)
{
}


Vector3D WhittedIntegrator::computeColor(
    const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    // MAXIMUM RAY DEPTH
    if (r.depth > 10)
    {
        return bgColor;
    }


    Intersection its;

    // RAY MISSES THE SCENE
    if (!Utils::getClosestIntersection(r, objList, its))
    {
        return bgColor;
    }


    const Material& material = its.shape->getMaterial();

    Vector3D wo = -r.d;


    // PERFECT MIRROR REFLECTION
    if (material.hasSpecular())
    {
        Vector3D wr =
            2.0 * dot(its.normal, wo) * its.normal - wo;

        Ray reflectedRay(
            its.itsPoint,
            wr.normalized(),
            r.depth + 1,
            Epsilon,
            INFINITY
        );

        return computeColor(
            reflectedRay,
            objList,
            lsList
        );
    }


    // PERFECT SPECULAR TRANSMISSION / REFRACTION
    if (material.hasTransmission())
    {
        Vector3D n = its.normal;

        double mu =
            material.getIndexOfRefraction();

        double cosTheta =
            dot(n, wo);


        // If the ray is leaving the object,
        // reverse the normal and invert eta1/eta2
        if (cosTheta < 0.0)
        {
            n = -n;
            cosTheta = -cosTheta;

            mu = 1.0 / mu;
        }


        double radicand = 1.0 - mu * mu * (1.0 - cosTheta * cosTheta);


        // REFRACTION
        if (radicand >= 0.0)
        {
            Vector3D wt =
                -mu * wo +
                n * (
                    mu * cosTheta -
                    std::sqrt(radicand)
                    );

            Ray refractedRay(
                its.itsPoint,
                wt.normalized(),
                r.depth + 1,
                Epsilon,
                INFINITY
            );

            return computeColor(
                refractedRay,
                objList,
                lsList
            );
        }


        // TOTAL INTERNAL REFLECTION
        else
        {
            Vector3D wr =
                2.0 * dot(n, wo) * n - wo;

            Ray reflectedRay(
                its.itsPoint,
                wr.normalized(),
                r.depth + 1,
                Epsilon,
                INFINITY
            );

            return computeColor(
                reflectedRay,
                objList,
                lsList
            );
        }
    }


    // AMBIENT TERM
    // Lo = at * rho_d + ...
    double ambient = 0.3;

    Vector3D color =
        material.getDiffuseReflectance() * ambient;


    // DIRECT ILLUMINATION
    for (size_t i = 0; i < lsList.size(); i++)
    {
        // Position of the point light
        Vector3D lightPos =
            lsList[i]->sampleLightPosition();


        // Direction from intersection point to light
        Vector3D toLight =
            lightPos - its.itsPoint;

        double lightDistance =
            toLight.length();

        Vector3D wi =
            toLight.normalized();


        // SHADOW RAY
        Ray shadowRay(
            its.itsPoint,
            wi,
            r.depth + 1,
            Epsilon,
            lightDistance
        );


        bool inShadow =
            Utils::hasIntersection(
                shadowRay,
                objList
            );


        // LIGHT CONTRIBUTION
        if (!inShadow)
        {
            // (wi · n)
            double cosTheta =
                std::max(
                    dot(wi, its.normal),
                    0.0
                );


            // Phong reflectance
            Vector3D fr =
                material.getReflectance(
                    its.normal,
                    wi,
                    wo
                );


            // Incident radiance from the point light
            Vector3D Li =
                lsList[i]->getIntensity();


            // Li * fr * (wi · n)
            color +=
                Li * fr * cosTheta;
        }
    }


    return color;
}
